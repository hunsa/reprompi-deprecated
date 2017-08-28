/*  ReproMPI Benchmark
 *
 *  Copyright 2015 Alexandra Carpen-Amarie, Sascha Hunold
    Research Group for Parallel Computing
    Faculty of Informatics
    Vienna University of Technology, Austria

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
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_sort.h>
#include "mpi.h"

#include "reprompi_bench/sync/synchronization.h"
#include "reprompi_bench/output_management/runtimes_computation.h"
#include "reproMPIbenchmark.h"
#include "results_output.h"

static const int OUTPUT_ROOT_PROC = 0;

void print_results_header_to_file(FILE* f, const reprompib_lib_output_info_t* output_info_p,
    const reprompib_job_t* job_p) {
    int my_rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    if (my_rank == OUTPUT_ROOT_PROC) {
        int i;

        for (i=0; i<job_p->n_user_svars; i++) {
            fprintf(f, "%30s ", job_p->user_svar_names[i]);
        }
        for (i=0; i<job_p->n_user_ivars; i++) {
            fprintf(f, "%10s ", job_p->user_ivar_names[i]);
        }

        fprintf(f, "%20s %4s", "measure_type", "proc");

        if (output_info_p->verbose == 1 && output_info_p->n_summary_methods == 0) {
#ifdef ENABLE_WINDOWSYNC
            fprintf(f, " %12s", "errorcode");
#endif

#ifdef ENABLE_WINDOWSYNC
            fprintf(f," %8s %16s %16s %16s %16s\n", "nrep", "loc_tstart_sec", "loc_tend_sec", "gl_tstart_sec", "gl_tend_sec");
#else
            fprintf(f," %8s %16s %16s\n",  "nrep", "loc_tstart_sec", "loc_tend_sec");
#endif
        } else {

            // print summary
            if (output_info_p->n_summary_methods >0) {
                fprintf(f, " %8s %8s ", "total_nrep", "valid_nrep");
                for (i = 0; i < output_info_p->n_summary_methods; i++) {
                    fprintf(f, "%14s_sec ", output_info_p->summary_methods_names[i]);
                }
                fprintf(f, "\n");
            }
            else {
#ifdef ENABLE_WINDOWSYNC
                fprintf(f, " %12s", "errorcode");
#endif
                fprintf(f, " %8s %16s\n", "nrep", "runtime_sec");
            }
        }

    }

}

void print_results_header(const reprompib_lib_output_info_t* output_info_p, const reprompib_job_t* job_p) {
    FILE* f = stdout;
    int my_rank;
    int summary = 0;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    // print results to file (if specified)
    if (my_rank == OUTPUT_ROOT_PROC) {
        if (output_info_p->output_file != NULL) {
            f = fopen(output_info_p->output_file, "a");
        }
    }


    if (output_info_p->n_summary_methods >0 &&
        output_info_p->output_file == NULL) {
        summary = 1;
    }
    print_results_header_to_file(f, output_info_p, job_p);
    if (my_rank == OUTPUT_ROOT_PROC) {
        if (output_info_p->output_file != NULL) {
            fclose(f);

            // print summary to stdout
            if (output_info_p->n_summary_methods >0) {
                print_results_header_to_file(stdout, output_info_p, job_p);
            }
        }
    }

}


void compute_runtimes_local_clocks_with_reduction(
        const double* tstart_sec, const double* tend_sec,
        long current_start_index, long current_nreps,
        double* maxRuntimes_sec, char* op) {

    double* local_runtimes = NULL;
    int i, index;
    int my_rank, np;
    MPI_Op operation = MPI_MAX;

    if (strcmp("min", op) == 0) {
        operation = MPI_MIN;
    }
    if (strcmp("mean", op) == 0) {
        operation = MPI_SUM;
    }

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    // compute local runtimes for the [current_start_index, current_start_index + current_nreps) interval
    local_runtimes = (double*) malloc(current_nreps * sizeof(double));
    for (i = 0; i < current_nreps; i++) {
        index = i + current_start_index;
        local_runtimes[i] = tend_sec[index] - tstart_sec[index];
    }

    // reduce local measurement results on the root
    MPI_Reduce(local_runtimes, maxRuntimes_sec, current_nreps,
            MPI_DOUBLE, operation, OUTPUT_ROOT_PROC, MPI_COMM_WORLD);

    if (my_rank == OUTPUT_ROOT_PROC) {
        if (strcmp("mean", op) == 0) {      // reduce with sum and then compute mean
            for (i = 0; i < current_nreps; i++) {
                maxRuntimes_sec[i] = maxRuntimes_sec[i]/np;
            }
        }
    }
    free(local_runtimes);
}



void print_runtimes(FILE* f, const reprompib_job_t* job_p,
        sync_errorcodes_t get_errorcodes, sync_normtime_t get_global_time) {

    double* maxRuntimes_sec;
    int i;
    int my_rank;
    int* sync_errorcodes;
    long current_start_index;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    maxRuntimes_sec = NULL;
    sync_errorcodes = NULL;
    if (my_rank == OUTPUT_ROOT_PROC) {
        maxRuntimes_sec = (double*) malloc(job_p->n_rep * sizeof(double));

#ifdef ENABLE_WINDOWSYNC
        sync_errorcodes = (int*) malloc(job_p->n_rep * sizeof(int));
        for (i = 0; i < job_p->n_rep; i++) {
            sync_errorcodes[i] = 0;
        }
#endif
    }

    current_start_index = 0;

#ifdef ENABLE_WINDOWSYNC
    compute_runtimes_global_clocks(job_p->tstart_sec, job_p->tend_sec, current_start_index, job_p->n_rep, OUTPUT_ROOT_PROC,
            get_errorcodes, get_global_time,
            maxRuntimes_sec, sync_errorcodes);
#else
    compute_runtimes_local_clocks_with_reduction(job_p->tstart_sec, job_p->tend_sec, current_start_index, job_p->n_rep,
            maxRuntimes_sec, job_p->op);
#endif

    if (my_rank == OUTPUT_ROOT_PROC) {
        for (i = 0; i < job_p->n_rep; i++) {
            int j;

            for (j=0; j<job_p->n_user_svars; j++) {
                fprintf(f, "%30s ", job_p->user_svars[j]);
            }
            for (j=0; j<job_p->n_user_ivars; j++) {
                fprintf(f, "%10d ", job_p->user_ivars[j]);
            }

#if defined(ENABLE_WINDOWSYNC) && !defined(ENABLE_BARRIERSYNC)    // measurements with window-based synchronization
            fprintf(f, "%20s %4s %12d %8d %16.10f\n", job_p->timername, "all",
                    sync_errorcodes[i],i,
                    maxRuntimes_sec[i]);
#else   // measurements with Barrier-based synchronization
            fprintf(f, "%20s %4s %8d %16.10f\n", job_p->timername, "all",
                    i, maxRuntimes_sec[i]);
#endif
        }

#ifdef ENABLE_WINDOWSYNC
        free(sync_errorcodes);
#endif

        free(maxRuntimes_sec);
    }
}



void print_runtimes_allprocs(FILE* f, const reprompib_job_t* job_p,
    const double* global_start_sec, const double* global_end_sec,
    int* errorcodes) {

    double* maxRuntimes_sec;
    int i;
    int my_rank;
    int np;
    int proc_id;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    maxRuntimes_sec = NULL;
    if (my_rank == OUTPUT_ROOT_PROC) {
        maxRuntimes_sec = (double*) malloc(job_p->n_rep * np * sizeof(double));

        for (proc_id = 0; proc_id < np; proc_id++) {
            for (i = 0; i < job_p->n_rep; i++) {
                maxRuntimes_sec[proc_id * job_p->n_rep + i] = global_end_sec[proc_id * job_p->n_rep + i] -
                        global_start_sec[proc_id * job_p->n_rep+ i];
            }
        }

        for (proc_id = 0; proc_id < np; proc_id++) {
            for (i = 0; i < job_p->n_rep; i++) {
                int j;

                for (j=0; j<job_p->n_user_svars; j++) {
                    fprintf(f, "%30s ", job_p->user_svars[j]);
                }
                for (j=0; j<job_p->n_user_ivars; j++) {
                    fprintf(f, "%10d ", job_p->user_ivars[j]);
                }

#if defined(ENABLE_WINDOWSYNC) && !defined(ENABLE_BARRIERSYNC)    // measurements with window-based synchronization
                fprintf(f, "%20s %4d %12d %8d %16.10f\n", job_p->timername,
                        proc_id, errorcodes[proc_id * job_p->n_rep + i], i,
                        maxRuntimes_sec[proc_id * job_p->n_rep + i]);
#else   // measurements with Barrier-based synchronization
                fprintf(f, "%20s %4d %8d %16.10f\n", job_p->timername,
                        proc_id,i, maxRuntimes_sec[proc_id * job_p->n_rep + i]);
#endif
            }
        }

        free(maxRuntimes_sec);
    }
}


void print_measurement_results(FILE* f,
    const reprompib_lib_output_info_t* output_info_p,
    const reprompib_job_t* job_p,
    const sync_errorcodes_t get_errorcodes,
    const sync_normtime_t get_global_time) {

    int i, proc_id;
    double* local_start_sec = NULL;
    double* local_end_sec = NULL;
    double* global_start_sec = NULL;
    double* global_end_sec = NULL;
    double* tmp_local_start_sec = NULL;
    double* tmp_local_end_sec = NULL;
    int my_rank, np;
    int* errorcodes = NULL;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    if (output_info_p->verbose == 0 && strcmp(job_p->timertype, "all") != 0) {
        print_runtimes(f, job_p, get_errorcodes, get_global_time);

    }
    else {

#ifdef ENABLE_WINDOWSYNC
        if (my_rank == OUTPUT_ROOT_PROC)
        {
            errorcodes = (int*)malloc(job_p->n_rep * np * sizeof(int));
            for (i = 0; i < job_p->n_rep * np; i++) {
                errorcodes[i] = 0;
            }
        }

#ifndef ENABLE_BARRIERSYNC	// gather measurement results
        {
            int* local_errorcodes = get_errorcodes();

            MPI_Gather(local_errorcodes, job_p->n_rep, MPI_INT,
                    errorcodes, job_p->n_rep, MPI_INT, OUTPUT_ROOT_PROC, MPI_COMM_WORLD);
        }
#endif

#endif

        if (my_rank == OUTPUT_ROOT_PROC) {
            local_start_sec = (double*) malloc(
                job_p->n_rep * np * sizeof(double));
            local_end_sec = (double*) malloc(
                job_p->n_rep * np * sizeof(double));
            global_start_sec = (double*) malloc(
                job_p->n_rep * np * sizeof(double));
            global_end_sec = (double*) malloc(
                job_p->n_rep * np * sizeof(double));
        }

        // gather measurement results
        MPI_Gather(job_p->tstart_sec, job_p->n_rep, MPI_DOUBLE,
            local_start_sec, job_p->n_rep, MPI_DOUBLE, OUTPUT_ROOT_PROC, MPI_COMM_WORLD);

        MPI_Gather(job_p->tend_sec, job_p->n_rep, MPI_DOUBLE,
            local_end_sec, job_p->n_rep, MPI_DOUBLE, OUTPUT_ROOT_PROC, MPI_COMM_WORLD);

        tmp_local_start_sec = (double*) malloc(
            job_p->n_rep * np * sizeof(double));
        tmp_local_end_sec = (double*) malloc(
            job_p->n_rep * np * sizeof(double));
        for (i = 0; i < job_p->n_rep; i++) {
            tmp_local_start_sec[i] = get_global_time(job_p->tstart_sec[i]);
            tmp_local_end_sec[i] = get_global_time(job_p->tend_sec[i]);
        }
        MPI_Gather(tmp_local_start_sec, job_p->n_rep, MPI_DOUBLE,
            global_start_sec, job_p->n_rep, MPI_DOUBLE, OUTPUT_ROOT_PROC, MPI_COMM_WORLD);

        MPI_Gather(tmp_local_end_sec, job_p->n_rep, MPI_DOUBLE,
            global_end_sec, job_p->n_rep, MPI_DOUBLE, OUTPUT_ROOT_PROC, MPI_COMM_WORLD);

        free(tmp_local_start_sec);
        free(tmp_local_end_sec);

        if (output_info_p->verbose == 0  && strcmp(job_p->timertype, "all") == 0) {

            print_runtimes_allprocs(f, job_p, global_start_sec, global_end_sec, errorcodes);
        }

        else {      // verbose == 1

            if (my_rank == OUTPUT_ROOT_PROC) {
                for (proc_id = 0; proc_id < np; proc_id++) {
                    for (i = 0; i < job_p->n_rep; i++) {
                        int j;
                        for (j=0; j< job_p->n_user_svars; j++) {
                            fprintf(f, "%30s ", job_p->user_svars[j]);
                        }
                        for (j=0; j<job_p->n_user_ivars; j++) {
                            fprintf(f, "%10d ", job_p->user_ivars[j]);
                        }

#ifdef ENABLE_WINDOWSYNC
                        fprintf(f, "%20s %4d %12d %8d %16.10f %16.10f %16.10f %16.10f\n",
                            job_p->timername, proc_id,
                                errorcodes[proc_id * job_p->n_rep + i], i,
                                local_start_sec[proc_id * job_p->n_rep + i],
                                local_end_sec[proc_id * job_p->n_rep + i],
                                global_start_sec[proc_id * job_p->n_rep + i],
                                global_end_sec[proc_id * job_p->n_rep + i]);

#else
                        fprintf(f, "%20s %4d %8d %16.10f %16.10f\n", job_p->timername,
                                proc_id, i,
                                local_start_sec[proc_id * job_p->n_rep + i],
                                local_end_sec[proc_id * job_p->n_rep + i]);
#endif
                    }
                }
            }
        }

        if (my_rank == OUTPUT_ROOT_PROC) {
            free(local_start_sec);
            free(local_end_sec);
            free(global_start_sec);
            free(global_end_sec);
#ifdef ENABLE_WINDOWSYNC
            free(errorcodes);
#endif

        }

    }
}



void print_summary(FILE* f,
        const reprompib_lib_output_info_t* output_info_p,
        const reprompib_job_t* job_p,
        const sync_errorcodes_t get_errorcodes,
        const sync_normtime_t get_global_time) {

    double* maxRuntimes_sec;
    int i, j;
    int my_rank, np;
    int* sync_errorcodes;
    long current_start_index;
    int n_results = 0, proc;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    maxRuntimes_sec = NULL;
    sync_errorcodes = NULL;
    if (my_rank == OUTPUT_ROOT_PROC) {
        maxRuntimes_sec = (double*) malloc(job_p->n_rep * np * sizeof(double));

#ifdef ENABLE_WINDOWSYNC
        sync_errorcodes = (int*) malloc(job_p->n_rep * np * sizeof(int));
        for (i = 0; i < job_p->n_rep * np; i++) {
            sync_errorcodes[i] = 0;
        }
#endif
    }

    current_start_index = 0;

    if (strcmp(job_p->timertype, "all") !=0) { // one runtime for each nrep id (reduced over processes)

#ifdef ENABLE_WINDOWSYNC
        compute_runtimes_global_clocks(job_p->tstart_sec, job_p->tend_sec,
                current_start_index, job_p->n_rep, OUTPUT_ROOT_PROC,
                get_errorcodes, get_global_time,
                maxRuntimes_sec, sync_errorcodes);
#else
        compute_runtimes_local_clocks_with_reduction(job_p->tstart_sec, job_p->tend_sec, current_start_index, job_p->n_rep,
                maxRuntimes_sec, job_p->op);
#endif
        n_results = 1;

    }
    else {  // one runtime for each process and each nrep
        double* local_start_sec = NULL;
        double* local_end_sec = NULL;
        double* global_start_sec = NULL;
        double* global_end_sec = NULL;
        double* tmp_local_start_sec = NULL;
        double* tmp_local_end_sec = NULL;

#ifdef ENABLE_WINDOWSYNC
#ifndef ENABLE_BARRIERSYNC  // gather measurement results
        {
            int* local_errorcodes = get_errorcodes();

            MPI_Gather(local_errorcodes, job_p->n_rep, MPI_INT,
                    sync_errorcodes, job_p->n_rep, MPI_INT, 0, MPI_COMM_WORLD);
        }
#endif

#endif

        if (my_rank == OUTPUT_ROOT_PROC) {
            local_start_sec = (double*) malloc(
                job_p->n_rep * np * sizeof(double));
            local_end_sec = (double*) malloc(
                job_p->n_rep * np * sizeof(double));
            global_start_sec = (double*) malloc(
                job_p->n_rep * np * sizeof(double));
            global_end_sec = (double*) malloc(
                job_p->n_rep * np * sizeof(double));
        }

        // gather measurement results
        MPI_Gather(job_p->tstart_sec, job_p->n_rep, MPI_DOUBLE, local_start_sec,
            job_p->n_rep, MPI_DOUBLE, OUTPUT_ROOT_PROC, MPI_COMM_WORLD);

        MPI_Gather(job_p->tend_sec, job_p->n_rep, MPI_DOUBLE, local_end_sec, job_p->n_rep,
                MPI_DOUBLE, OUTPUT_ROOT_PROC, MPI_COMM_WORLD);

        tmp_local_start_sec = (double*) malloc(
            job_p->n_rep * np * sizeof(double));
        tmp_local_end_sec = (double*) malloc(
            job_p->n_rep * np * sizeof(double));
        for (i = 0; i < job_p->n_rep; i++) {
            tmp_local_start_sec[i] = get_global_time(job_p->tstart_sec[i]);
            tmp_local_end_sec[i] = get_global_time(job_p->tend_sec[i]);
        }
        MPI_Gather(tmp_local_start_sec, job_p->n_rep, MPI_DOUBLE, global_start_sec,
            job_p->n_rep, MPI_DOUBLE, OUTPUT_ROOT_PROC, MPI_COMM_WORLD);

        MPI_Gather(tmp_local_end_sec, job_p->n_rep, MPI_DOUBLE, global_end_sec, job_p->n_rep,
                MPI_DOUBLE, OUTPUT_ROOT_PROC, MPI_COMM_WORLD);

        free(tmp_local_start_sec);
        free(tmp_local_end_sec);


        if (my_rank == OUTPUT_ROOT_PROC) {
            int proc_id;

            for (proc_id = 0; proc_id < np; proc_id++) {
                for (i = 0; i < job_p->n_rep; i++) {
                    maxRuntimes_sec[proc_id * job_p->n_rep + i] = global_end_sec[proc_id * job_p->n_rep + i] -
                            global_start_sec[proc_id * job_p->n_rep+ i];
                }
            }

            free(local_start_sec);
            free(local_end_sec);
            free(global_start_sec);
            free(global_end_sec);
        }

        n_results = np;
    }



    if (my_rank == OUTPUT_ROOT_PROC) {
        long nreps;

        for (proc = 0; proc < n_results; proc++) {
            double* current_proc_runtimes;
            int* current_error_codes;

            nreps = 0;

            current_proc_runtimes = maxRuntimes_sec + (proc * job_p->n_rep);
            current_error_codes = sync_errorcodes + (proc * job_p->n_rep);

            // remove measurements with out-of-window errors
#ifdef ENABLE_WINDOWSYNC
            for (i = 0; i < job_p->n_rep; i++) {
                if (current_error_codes[i] == 0) {
                    if (nreps < i) {
                        current_proc_runtimes[nreps] = current_proc_runtimes[i];
                    }
                    nreps++;
                }
            }
#else
            nreps = job_p->n_rep;
#endif

            gsl_sort(current_proc_runtimes, 1, nreps);

            for (j=0; j<job_p->n_user_svars; j++) {
                fprintf(f, "%30s ", job_p->user_svars[j]);
            }
            for (j=0; j<job_p->n_user_ivars; j++) {
                fprintf(f, "%10d ", job_p->user_ivars[j]);
            }

            if (strcmp(job_p->timertype, "all") != 0) {
                fprintf(f, "%20s %4s %10ld %10ld ", job_p->timername, "all", job_p->n_rep, nreps);
            }
            else {
                fprintf(f, "%20s %4d %10ld %10ld ", job_p->timername, proc, job_p->n_rep, nreps);
            }
            for (i = 0; i < output_info_p->n_summary_methods; i++) {
                    if (strcmp(output_info_p->summary_methods_names[i], "mean") == 0) {
                        double mean = gsl_stats_mean(current_proc_runtimes, 1, nreps);
                        fprintf(f, "  %16.10f ", mean);
                    }
                    else if (strcmp(output_info_p->summary_methods_names[i], "median") == 0) {
                        double median;
                        median =  gsl_stats_quantile_from_sorted_data (current_proc_runtimes, 1, nreps, 0.5);
                        fprintf(f, "  %16.10f ", median);
                    }
                    else if (strcmp(output_info_p->summary_methods_names[i], "min") == 0) {
                        double min = 0;
                        if (nreps > 0) {
                            min = current_proc_runtimes[0];
                        }
                        fprintf(f, "  %16.10f ", min);
                    }
                    else  if (strcmp(output_info_p->summary_methods_names[i], "max") == 0) {
                        double max = 0;
                        if (nreps > 0) {
                            max = current_proc_runtimes[nreps-1];
                        }
                        fprintf(f, "  %16.10f ", max);
                    }
                    else {
                      fprintf (stderr, "ERROR: Unknown summary operation: %s" , output_info_p->summary_methods_names[i]);
                    }
            }
            fprintf(f, "\n");
        }


#ifdef ENABLE_WINDOWSYNC
        free(sync_errorcodes);
#endif

        free(maxRuntimes_sec);
    }
}






