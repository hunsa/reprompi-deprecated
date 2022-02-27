/*  ReproMPI Benchmark
 *
 *  Copyright 2015 Alexandra Carpen-Amarie, Sascha Hunold
    Research Group for Parallel Computing
    Faculty of Informatics
    Vienna University of Technology, Austria
 *
 * Copyright (c) 2021 Stefan Christians
 *
<license>
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
</license>
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include "mpi.h"
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_fit.h>
#include <gsl/gsl_sort.h>

#include "reprompi_bench/misc.h"
#include "reprompi_bench/sync/time_measurement.h"
#include "reprompi_bench/sync/sync_info.h"
#include "hca_parse_options.h"
#include "hca_sync.h"

#include "contrib/intercommunication/intercommunication.h"

const int HCA_WARMUP_ROUNDS = 5;


static double start_sync = 0;       /* current window start timestamp (global time) */
static int* invalid;
static int repetition_counter = 0;  /* current repetition index */


double initial_timestamp = 0;


// options specified from the command line
static reprompi_hca_params_t parameters;

//linear model
typedef struct {
    double intercept;
    double slope;
} lm_t;

lm_t lm;



enum {
    Number_ping_pongs1 = 100,
    Minimum_ping_pongs1 = 8
};


inline double hca_get_adjusted_time(void) {
    return get_time() - initial_timestamp;
}


inline static lm_t merge_linear_models(lm_t lm1, lm_t lm2) {
    lm_t new_model;

    new_model.intercept = 0;
    new_model.slope = lm1.slope + lm2.slope - lm1.slope * lm2.slope;
#ifdef ENABLE_LOGP_SYNC         // need to merge previously computed intercepts
    new_model.intercept = lm1.intercept + lm2.intercept - lm2.intercept * lm1.slope;
#endif

    return new_model;
}

static double ping_pong_skampi(int p1, int p2)
{
    int i, other_global_id;
    double s_now, s_last, t_last, t_now;
    double td_min, td_max;
    double invalid_time = -1.0;
    MPI_Status status;
    static double *ping_pong_min_time;
    int pp_tag = 43;
    double offset = 0;

    int my_rank = icmb_global_rank();
    int np = icmb_global_size();

    ping_pong_min_time = (double*)malloc(np * sizeof(double));
    for( i = 0; i < np; i++) ping_pong_min_time[i] = -1.0;


    /* I had to unroll the main loop because I didn't find a portable way
     to define the initial td_min and td_max with INFINITY and NINFINITY */
    if( my_rank == p1 ) {
        other_global_id = p2;

        s_last = hca_get_adjusted_time();
        MPI_Send(&s_last, 1, MPI_DOUBLE, p2, pp_tag, icmb_global_communicator());
        MPI_Recv(&t_last, 1, MPI_DOUBLE, p2, pp_tag, icmb_global_communicator(), &status);
        s_now = hca_get_adjusted_time();
        MPI_Send(&s_now, 1, MPI_DOUBLE, p2, pp_tag, icmb_global_communicator());


        td_min = t_last - s_now;
        td_max = t_last - s_last;


    } else {
        other_global_id = p1;

        MPI_Recv(&s_last, 1, MPI_DOUBLE, p1, pp_tag, icmb_global_communicator(), &status);
        t_last = hca_get_adjusted_time();
        MPI_Send(&t_last, 1, MPI_DOUBLE, p1, pp_tag, icmb_global_communicator());
        MPI_Recv(&s_now, 1, MPI_DOUBLE, p1, pp_tag, icmb_global_communicator(), &status);
        t_now = hca_get_adjusted_time();


        td_min = s_last - t_last;
        td_min = repro_max(td_min, s_now - t_now);

        td_max = s_now - t_last;
    }


    if( my_rank == p1 ) {
        i = 1;
        while( 1 ) {

            MPI_Recv(&t_last, 1, MPI_DOUBLE, p2, pp_tag, icmb_global_communicator(), &status);
            if( t_last < 0.0 ) break;

            s_last = s_now;
            s_now = hca_get_adjusted_time();

            td_min = repro_max(td_min, t_last - s_now);
            td_max = repro_min(td_max, t_last - s_last);

            if( ping_pong_min_time[other_global_id] >= 0.0  &&
                    i >= Minimum_ping_pongs1 &&
                    s_now - s_last < ping_pong_min_time[other_global_id]*1.10 ) {
                MPI_Send(&invalid_time, 1, MPI_DOUBLE, p2, pp_tag, icmb_global_communicator());
                break;
            }
            i++;
            if( i == Number_ping_pongs1 ) {
                MPI_Send(&invalid_time, 1, MPI_DOUBLE, p2, pp_tag, icmb_global_communicator());
                break;
            }
            MPI_Send(&s_now, 1, MPI_DOUBLE, p2, pp_tag, icmb_global_communicator());

        }
    } else {
        i = 1;
        while( 1 ) {
            MPI_Send(&t_now, 1, MPI_DOUBLE, p1, pp_tag, icmb_global_communicator());
            MPI_Recv(&s_last, 1, MPI_DOUBLE, p1, pp_tag, icmb_global_communicator(), &status);
            t_last = t_now;
            t_now = hca_get_adjusted_time();

            if( s_last < 0.0 ) break;

            td_min = repro_max(td_min, s_last - t_now);
            td_max = repro_min(td_max, s_last - t_last);

            if( ping_pong_min_time[other_global_id] >= 0.0 &&
                    i >= Minimum_ping_pongs1 &&
                    t_now - t_last < ping_pong_min_time[other_global_id]*1.10 ) {
                MPI_Send(&invalid_time, 1, MPI_DOUBLE, p1, pp_tag, icmb_global_communicator());
                break;
            }
            i++;
        }
    }

    if( ping_pong_min_time[other_global_id] < 0.0) {
        ping_pong_min_time[other_global_id] = td_max-td_min;
    }
    else {
        ping_pong_min_time[other_global_id] = repro_min(ping_pong_min_time[other_global_id], td_max-td_min);
    }
    offset = (td_min+td_max)/2.0;

    free(ping_pong_min_time);

    return offset;
}



void compute_and_set_intercept(lm_t* lm, int client, int p_ref) {
    int my_rank = icmb_global_rank();

    if (my_rank == p_ref) {
        //compute intercept with SKaMPI ping-pong
        ping_pong_skampi(my_rank, client);
    } else
        if (my_rank == client) {
            //compute intercept with SKaMPI ping-pong
            double intercept_time;
            double offset = -ping_pong_skampi(p_ref, my_rank);

            intercept_time = hca_get_adjusted_time();
            lm->intercept = (lm->slope) * (-intercept_time) + offset;
        }
}

void compute_and_set_all_intercepts(lm_t* lm)
{
    int i;
    int my_rank = icmb_global_rank();
    int np = icmb_global_size();
    int master_rank = 0;

    if (my_rank != master_rank) {
        compute_and_set_intercept(lm, my_rank, master_rank);
    }
    else {
        for( i = 0; i < np; i++) {
            if (i != master_rank) {
                compute_and_set_intercept(lm, i, master_rank);
            }
        }
    }
}




void compute_rtt(int master_rank, int other_rank, const int n_pingpongs, double *rtt) {
    int my_rank = icmb_global_rank();
    int np = icmb_global_size();
    MPI_Status stat;
    int i;
    double tmp;
    double *rtts = NULL;
    double mean;

    if (my_rank == master_rank) {
        double tstart, tremote;

        /* warm up */
        for (i = 0; i < HCA_WARMUP_ROUNDS; i++) {
            tmp = hca_get_adjusted_time();
            MPI_Send(&tmp, 1, MPI_DOUBLE, other_rank, 0, icmb_global_communicator());
            MPI_Recv(&tmp, 1, MPI_DOUBLE, other_rank, 0, icmb_global_communicator(), &stat);
        }

        rtts  = (double*) malloc(n_pingpongs * sizeof(double));

        for (i = 0; i < n_pingpongs; i++) {
            tstart = hca_get_adjusted_time();
            MPI_Send(&tstart, 1, MPI_DOUBLE, other_rank, 0, icmb_global_communicator());
            MPI_Recv(&tremote, 1, MPI_DOUBLE, other_rank, 0, icmb_global_communicator(), &stat);
            rtts[i] = hca_get_adjusted_time() - tstart;
        }

    } else if( my_rank == other_rank ) {
        double tlocal = 0, troot;

        /* warm up */
        for (i = 0; i < HCA_WARMUP_ROUNDS; i++) {
            MPI_Recv(&tmp, 1, MPI_DOUBLE, master_rank, 0, icmb_global_communicator(), &stat);
            tmp = hca_get_adjusted_time();
            MPI_Send(&tmp, 1, MPI_DOUBLE, master_rank, 0, icmb_global_communicator());
        }

        for (i = 0; i < n_pingpongs; i++) {
            MPI_Recv(&troot, 1, MPI_DOUBLE, master_rank, 0, icmb_global_communicator(), &stat);
            tlocal = hca_get_adjusted_time();
            MPI_Send(&tlocal, 1, MPI_DOUBLE, master_rank, 0, icmb_global_communicator());
        }
    }

    if (my_rank == master_rank) {
        double upperq;
        double cutoff_val;
        double *rtts2;
        int n_datapoints;

        gsl_sort(rtts, 1, n_pingpongs);

        upperq = gsl_stats_quantile_from_sorted_data (rtts, 1, n_pingpongs, 0.75);
        cutoff_val = 1.5 * upperq;

        //    printf("cutoff=%.20f\n", cutoff_val);
        rtts2 = (double*)calloc(n_pingpongs, sizeof(double));
        n_datapoints = 0;
        for(i=0; i<n_pingpongs; i++) {
            if( rtts[i] <= cutoff_val ) {
                rtts2[i] = rtts[i];
                n_datapoints = i+1;
            } else {
                break;
            }
        }
        mean = gsl_stats_mean(rtts2, 1, n_datapoints);

        free(rtts);
        free(rtts2);

        MPI_Send(&mean, 1, MPI_DOUBLE, other_rank, 0, icmb_global_communicator());
    } else {
        MPI_Recv(&mean, 1, MPI_DOUBLE, master_rank, 0, icmb_global_communicator(), &stat);
    }

    *rtt = mean;
}


lm_t hca_learn_model(const int root_rank, const int other_rank,
        const reprompi_hca_params_t params, const double my_rtt) {
    int i, j;
    int my_rank = icmb_global_rank();
    int np = icmb_global_size();
    MPI_Status status;
    lm_t lm;

    lm.intercept = 0;
    lm.slope = 0;

    if (my_rank == root_rank) {

        double tlocal, tremote;

        for (j = 0; j < params.n_fitpoints; j++) {

            for (i = 0; i < params.n_exchanges; i++) {
                MPI_Recv(&tremote, 1, MPI_DOUBLE, other_rank, 0, icmb_global_communicator(), &status);
                tlocal = hca_get_adjusted_time();
                MPI_Ssend(&tlocal, 1, MPI_DOUBLE, other_rank, 0, icmb_global_communicator());
            }
        }
    } else {

        double *time_var, *local_time, *time_var2;
        double *xfit, *yfit;
        double cov00, cov01, cov11, sumsq;
        int fit;
        double dummy;
        double median, master_time;

        double mindiff =1e10, mindiff_ts;

        time_var  = (double*) calloc(params.n_exchanges, sizeof(double));
        time_var2 = (double*) calloc(params.n_exchanges, sizeof(double));
        local_time = (double*) calloc(params.n_exchanges, sizeof(double));

        xfit = (double*) calloc(params.n_fitpoints, sizeof(double));
        yfit = (double*) calloc(params.n_fitpoints, sizeof(double));

        for (j = 0; j < params.n_fitpoints; j++) {

            for (i = 0; i < params.n_exchanges; i++) {
                dummy = hca_get_adjusted_time();
                MPI_Ssend(&dummy, 1, MPI_DOUBLE, root_rank, 0, icmb_global_communicator());
                MPI_Recv(&master_time, 1, MPI_DOUBLE, root_rank, 0, icmb_global_communicator(),
                        &status);
                local_time[i] = hca_get_adjusted_time();
                time_var[i] = local_time[i] - master_time - my_rtt / 2.0;
                time_var2[i] = time_var[i];

                if (time_var[i] < mindiff) {

                    mindiff = time_var[i];
                    mindiff_ts = local_time[i];
                }
            }

            gsl_sort(time_var2, 1, params.n_exchanges);

            if( params.n_exchanges % 2 == 0 ) {
                median = gsl_stats_median_from_sorted_data(time_var2, 1, params.n_exchanges-1);
            } else {
                median = gsl_stats_median_from_sorted_data(time_var2, 1, params.n_exchanges);
            }

            for (i = 0; i < params.n_exchanges; i++) {
                if (time_var[i] == median) {
                    xfit[j] = local_time[i];
                    yfit[j] = time_var[i];
                    break;
                }
            }
        }

        fit = gsl_fit_linear(xfit, 1, yfit, 1, params.n_fitpoints, &lm.intercept,
                &lm.slope, &cov00, &cov01, &cov11, &sumsq);
        free(time_var);
        free(time_var2);
        free(local_time);
        free(xfit);
        free(yfit);
    }

    return lm;
}

inline double hca_get_normalized_time(double local_time) {
    return local_time- (local_time * lm.slope + lm.intercept);
}

inline int my_pow_2(int exp) {
    return (int)pow(2.0, (double)exp);
}

void print_models(int my_rank, lm_t *linear_models, int nprocs, int round) {
    int i;

    for(i=0; i<nprocs; i++) {
        printf("r%d: [%d]->[%d] [%14.9f,%14.9f]\n", round, my_rank, i, linear_models[i].intercept, linear_models[i].slope);
    }
    fflush(stdout);

}


void hca_init_synchronization_module(const reprompib_sync_options_t parsed_opts, const long nrep)
{
    int i;

    parameters.n_exchanges = parsed_opts.n_exchanges;
    parameters.n_fitpoints = parsed_opts.n_fitpoints;
    parameters.wait_time_sec = parsed_opts.wait_time_sec;
    parameters.window_size_sec = parsed_opts.window_size_sec;
    parameters.n_rep = nrep;

    invalid  = (int*)calloc(parameters.n_rep, sizeof(int));
    for(i = 0; i < parameters.n_rep; i++)
    {
        invalid[i] = 0;
    }
    repetition_counter = 0;

    initial_timestamp = get_time();
}



void hca_synchronize_clocks(void)
{
    int my_rank = icmb_global_rank();
    int nprocs = icmb_global_size();
    int i, j, p;

    int master_rank = 0;
    double *rtts_s;
    int n_pingpongs = 1000;
    int max_power_two, nrounds_step1;
    lm_t *linear_models, *tmp_linear_models;
    int running_power;
    int other_rank;
    double current_rtt;
    int nb_lm_to_comm;

    MPI_Datatype dtype[2] = { MPI_DOUBLE, MPI_DOUBLE };
    int blocklen[2] =  { 1, 1 };
    MPI_Aint disp[2] = {0, sizeof(double)};
    MPI_Datatype mpi_lm_t;
    MPI_Status stat;

    int *step_two_group_ranks;
    int step_two_nb_ranks;
    MPI_Group orig_group, step_two_group;
    MPI_Comm step_two_comm;


    rtts_s = (double*)calloc(nprocs, sizeof(double));

    nrounds_step1 = floor(log2((double)nprocs));
    max_power_two = (int)pow(2.0, (double)nrounds_step1);

    linear_models = (lm_t*)calloc(nprocs, sizeof(lm_t));
    tmp_linear_models = (lm_t*)calloc(nprocs, sizeof(lm_t));

    MPI_Type_create_struct ( 2, blocklen, disp, dtype, &mpi_lm_t);
    MPI_Type_commit ( &mpi_lm_t );

    for(i=0; i<nrounds_step1; i++) {

        running_power = my_pow_2(i+1);

        // get a client/server pair
        // : estimate rtt
        // : learn clock model
        // : gather clock model
        // : adjust model
        if( my_rank >= max_power_two ) {
            //
        } else
        {
            if( my_rank % running_power == 0 ) {

                // master
                other_rank = my_rank + my_pow_2(i);
                compute_rtt(my_rank, other_rank, n_pingpongs, &current_rtt);


                // compute model through linear regression
                hca_learn_model(my_rank, other_rank, parameters, current_rtt);
#ifdef ENABLE_LOGP_SYNC
                compute_and_set_intercept(NULL, other_rank, my_rank);
#endif

                // so I also need to receive some other models from my partner rank
                // there should be 2^i models to receive
                nb_lm_to_comm = my_pow_2(i);

                if( nb_lm_to_comm > 0 ) {
                    MPI_Recv(&tmp_linear_models[0], nb_lm_to_comm, mpi_lm_t, other_rank, 0, icmb_global_communicator(), &stat);

                    linear_models[other_rank] = tmp_linear_models[0];
                    for(j=1; j<nb_lm_to_comm; j++) {
                        linear_models[other_rank+j] = merge_linear_models(linear_models[other_rank], tmp_linear_models[j]);
                    }
                }

            }
            else if( my_rank % running_power == my_pow_2(i) ) {
                // client
                other_rank = my_rank - my_pow_2(i);
                compute_rtt(other_rank, my_rank, n_pingpongs, &current_rtt);

                // compute model through linear regression
                linear_models[my_rank] = hca_learn_model(other_rank, my_rank, parameters, current_rtt);
#ifdef ENABLE_LOGP_SYNC
                compute_and_set_intercept(&linear_models[my_rank], my_rank, other_rank);
#endif

                // I will need to send my models back to the master
                // there should be 2^i models to send
                // starting from my rank
                nb_lm_to_comm = my_pow_2(i);

                if( nb_lm_to_comm > 0 ) {
                    MPI_Send(&linear_models[my_rank], nb_lm_to_comm, mpi_lm_t, other_rank, 0, icmb_global_communicator());
                }
            }
        }
        MPI_Barrier(icmb_global_communicator());
    }


    MPI_Comm_group(icmb_global_communicator(), &orig_group);
    step_two_nb_ranks = nprocs - max_power_two + 1;
    step_two_group_ranks = (int*) calloc(step_two_nb_ranks, sizeof(int));

    step_two_group_ranks[0] = master_rank;
    j = 1;
    for (p = max_power_two; p < nprocs; p++) {
        step_two_group_ranks[j] = p;
        j++;
    }

    MPI_Group_incl(orig_group, step_two_nb_ranks, step_two_group_ranks,
            &step_two_group);

    MPI_Comm_create(icmb_global_communicator(), step_two_group, &step_two_comm);

    // now step 2
    // synchronize processes with ranks > 2^max_power_two
    if( nprocs > max_power_two ) {

        if( my_rank < max_power_two ) {
            if( my_rank + max_power_two < nprocs ) {

                other_rank = my_rank + max_power_two;
                compute_rtt(my_rank, other_rank, n_pingpongs, &current_rtt);

                // compute model through linear regression
                linear_models[other_rank] = hca_learn_model(my_rank, other_rank, parameters, current_rtt);
#ifdef ENABLE_LOGP_SYNC
                compute_and_set_intercept(NULL, other_rank, my_rank);
#endif
            }

        } else {
            other_rank = my_rank - max_power_two;
            compute_rtt(other_rank, my_rank, n_pingpongs, &current_rtt);

            // compute model through linear regression
            linear_models[my_rank] = hca_learn_model(other_rank, my_rank,
                    parameters, current_rtt);
#ifdef ENABLE_LOGP_SYNC
            compute_and_set_intercept(&linear_models[my_rank], my_rank, other_rank);
#endif
        }


        if( step_two_comm != MPI_COMM_NULL ) {
            // 0 in sub comm is master rank
            MPI_Gather(&linear_models[my_rank], 1, mpi_lm_t,
                    &tmp_linear_models[0], 1, mpi_lm_t, 0, step_two_comm);
            MPI_Comm_free(&step_two_comm);
        }

        if( my_rank == master_rank ) {
            // max_power_two ranks start in buffer at position 1
            for(j=1; j<step_two_nb_ranks; j++) {
                // now we have the time between
                // p and p-max_power_two
                p = max_power_two - 1 + j;
                if( j == 1 ) { // master rank
                    linear_models[p].slope     = tmp_linear_models[j].slope;
                    linear_models[p].intercept = tmp_linear_models[j].intercept;
                } else {
                    linear_models[p] = merge_linear_models(linear_models[j-1], tmp_linear_models[j]);
                }
            }
        }
    }

    MPI_Scatter(linear_models, 1, mpi_lm_t, &lm, 1, mpi_lm_t, master_rank, icmb_global_communicator());

#ifndef ENABLE_LOGP_SYNC
    compute_and_set_all_intercepts(&lm);
#endif
    MPI_Barrier(icmb_global_communicator());

    free(linear_models);
    free(tmp_linear_models);
    free(rtts_s);

    if( my_rank == master_rank ) {
        lm.slope = 0;
        lm.intercept = 0;
    }

}


void hca_init_synchronization(void) {
    int my_rank = icmb_global_rank();
    int master_rank = 0;

    repetition_counter = 0;
    if( my_rank == master_rank ) {
        start_sync = hca_get_adjusted_time() + parameters.wait_time_sec;
    }
    MPI_Bcast(&start_sync, 1, MPI_DOUBLE, master_rank, icmb_global_communicator());

}

void hca_start_synchronization(void)
{
    int is_first = 1;
    double global_time;

    while(1) {
        global_time = hca_get_normalized_time(hca_get_adjusted_time());

        if( global_time >= start_sync ) {
            if( is_first == 1 ) {
                invalid[repetition_counter] |= FLAG_START_TIME_HAS_PASSED;
            }
            break;
        }
        is_first = 0;
    }
}


void hca_stop_synchronization(void)
{
    double global_time;
    global_time = hca_get_normalized_time(hca_get_adjusted_time());

    if( global_time > start_sync + parameters.window_size_sec ) {
        invalid[repetition_counter] |= FLAG_SYNC_WIN_EXPIRED;
    }

    start_sync += parameters.window_size_sec;
    repetition_counter++;
}



int* hca_get_local_sync_errorcodes(void)
{
    return invalid;
}


void hca_cleanup_synchronization_module(void)
{
    free(invalid);
}


void hca_print_sync_parameters(FILE* f)
{
    fprintf (f, "#@sync=HCA\n");
    fprintf(f, "#@window_s=%.10f\n", parameters.window_size_sec);
    fprintf(f, "#@fitpoints=%d\n", parameters.n_fitpoints);
    fprintf(f, "#@exchanges=%d\n", parameters.n_exchanges);
    fprintf(f, "#@wait_time_s=%.10f\n", parameters.wait_time_sec);
#ifdef ENABLE_LOGP_SYNC
    fprintf(f, "#@hcasynctype=logp\n");
#else
    fprintf(f, "#@hcasynctype=linear\n");
#endif
}
