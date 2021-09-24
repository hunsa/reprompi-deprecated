/* ReproMPI Process Skew Measurement
 *
 * Copyright (c) 2021 Stefan Christians
 *
 * based on
 * ReproMPI Benchmark
   Copyright 2015 Alexandra Carpen-Amarie, Sascha Hunold
   Research Group for Parallel Computing
   Faculty of Informatics
   Vienna University of Technology, Austria
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
   This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <gsl/gsl_sort.h>
#include <gsl/gsl_statistics.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "reprompi_bench/option_parser/parse_extra_key_value_options.h"
#include "reprompi_bench/option_parser/parse_options.h"
#include "reprompi_bench/sync/joneskoenig_sync/jk_sync.h"
#include "reprompi_bench/sync/time_measurement.h"
#include "version.h"

#include "contrib/intercommunication/intercommunication.h"

#include "options_parser.h"
#include "output.h"

static const int OUTPUT_ROOT_PROC = 0;
static const char* MPI_CALL_NAME = "process_skew";
static const int OUTPUT_NITERATIONS_CHUNK = 3000; // approx. 1 MB per process

void print_command_line(int argc, char** argv)
{
    if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC)) {
        int i;
        printf("#Command-line arguments: ");
        for (i = 0; i < argc; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }
}

void print_header(const skew_options_t* skew_options, const reprompib_options_t* benchmark_options )
{
    if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC))
    {
        FILE * f;

        f = stdout;
        // print summary to stdout
        if ( benchmark_options->print_summary_methods >0)
        {
          int i;
          fprintf(f, "%14s %10s %10s ", "test", "total_nrep", "valid_nrep");

          for (i=0; i<reprompib_get_number_summary_methods(); i++)
          {
            summary_method_info_t* s = reprompib_get_summary_method(i);
            if ( benchmark_options->print_summary_methods & s->mask)
            {
              fprintf(f, "%10s_sec ", s->name);
            }
          }
          fprintf(f, "\n");
        }


        // print results to file (if specified)
        if (skew_options->output_file != NULL)
        {
            f = fopen(skew_options->output_file, "a");
        }

        if (skew_options->output_file != NULL || benchmark_options->print_summary_methods==0)
        {
            if (benchmark_options->verbose)
            {
                // print measurement times for each process
                fprintf(f, "%7s ", "process");
            }

            fprintf(f, "%14s %10s ", "test", "nrep");

            if (skew_options->use_window)
            {
                fprintf(f, "%10s ", "errorcode");
            }

            if (benchmark_options->verbose)
            {
                fprintf(f, "%17s %17s %14s\n", "loc_tstart_sec", "gl_tstart_sec", "skew_sec");
            }
            else
            {
                fprintf(f, "%14s\n", "max_skew_sec");
            }
        }

        if (skew_options->output_file != NULL) {
            fflush(f);
            fclose(f);
        }
    }
}

static void print_settings_to_file(FILE* f, const skew_options_t* skew_options, const reprompib_dictionary_t* params_dict, const reprompib_options_t* benchmark_options)
{
    // print_benchmark_common_settings_to_file
    fprintf(f, "#MPI calls:\n");
    fprintf(f, "#\t%s\n", MPI_CALL_NAME);

    // print_common_settings_to_file
    reprompib_print_dictionary(params_dict, f);
    fprintf(f, "#@reproMPIcommitSHA1=%s\n", git_commit);
    if (icmb_is_intercommunicator())
    {
        fprintf(f, "#@intercommunicator=true\n");
        fprintf(f, "#@intercommunicator_type=%s\n", icmb_intercommunicator_type());
        fprintf(f, "#@nprocs_intiator=%d\n", icmb_initiator_size());
        fprintf(f, "#@nprocs_responder=%d\n", icmb_responder_size());
    }
    else{
        fprintf(f, "#@nprocs=%d\n", icmb_local_size());
    }
    fprintf(f, "#@clocktype=global\n");
    print_time_parameters(f);
    jk_print_sync_parameters(f);

    // print_initial_settings
    fprintf(f, "#@nrep=%ld\n", benchmark_options->n_rep);

    // process skew options
    if (skew_options->use_window)
    {
        fprintf(f, "#@process_sync=window\n");
    }
    else if (skew_options->use_mpi_barrier)
    {
        fprintf(f, "#@process_sync=MPI_Barrier\n");
    }
    else if (skew_options->use_dissemination_barrier)
    {
        fprintf(f, "#@process_sync=BBarrier\n");
    }
    if (skew_options->use_double_barrier)
    {
        fprintf(f, "#@doublebarrier=true\n");
    }
}

void print_settings(const skew_options_t* skew_options, const reprompib_dictionary_t* params_dict, const reprompib_options_t* benchmark_options)
{
    if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC))
    {
        FILE* f = stdout;
        print_settings_to_file(f, skew_options, params_dict, benchmark_options);
        if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC)) {
            if (skew_options->output_file != NULL) {
                f = fopen(skew_options->output_file, "a");
                print_settings_to_file(f, skew_options, params_dict, benchmark_options);
                fflush(f);
                fclose(f);
            }
        }
    }
}

static void compute_starttimes_global_clocks(const skew_options_t* skew_options, const double* tstart_sec, long current_start_index, long current_nreps, double* max_process_skew, int* sync_errorcodes)
{

    // gather error codes in the  [current_start_index, current_start_index + current_nreps) interval
    int* local_errorcodes = NULL;
    if (skew_options->use_window)
    {
        local_errorcodes = jk_get_local_sync_errorcodes() + current_start_index;
        MPI_Reduce(local_errorcodes, sync_errorcodes, current_nreps, MPI_INT, MPI_MAX, icmb_lookup_global_rank(OUTPUT_ROOT_PROC), icmb_global_communicator());
    }

    double* start_sec = NULL;
    double* end_sec = NULL;
    if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC))
    {
        start_sec = (double*)malloc(current_nreps * sizeof(double));
        end_sec = (double*)malloc(current_nreps * sizeof(double));
    }

    double* norm_tstart_sec = (double*)malloc(current_nreps * sizeof(double));

    // normalize results in the  [current_start_index, current_start_index + current_nreps) interval
    int index;
    for (int i = 0; i < current_nreps; i++)
    {
        index = i + current_start_index;
        norm_tstart_sec[i] = jk_get_normalized_time(tstart_sec[index]);
    }

    // gather results at the root process and compute start times
    MPI_Reduce(norm_tstart_sec, start_sec, current_nreps, MPI_DOUBLE, MPI_MIN, icmb_lookup_global_rank(OUTPUT_ROOT_PROC), icmb_global_communicator());
    MPI_Reduce(norm_tstart_sec, end_sec, current_nreps, MPI_DOUBLE, MPI_MAX, icmb_lookup_global_rank(OUTPUT_ROOT_PROC), icmb_global_communicator());

    if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC))
    {
        for (int i = 0; i< current_nreps; i++)
        {
            index = i + current_start_index;
            max_process_skew[index] = end_sec[index] - start_sec[index];
        }

        free(start_sec);
        free(end_sec);
    }

    free(norm_tstart_sec);
}

static void print_summary(FILE* f,const skew_options_t* skew_options, const reprompib_options_t* benchmark_options, double* tstart_sec)
{
    double* max_process_skew = NULL;
    int* sync_errorcodes = NULL;
    if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC))
    {
        max_process_skew = (double*) malloc(benchmark_options->n_rep * sizeof(double));

        if (skew_options->use_window)
        {
            sync_errorcodes = (int*) malloc(benchmark_options->n_rep * sizeof(int));
            for (int i = 0; i < benchmark_options->n_rep; i++)
            {
                sync_errorcodes[i] = 0;
            }
        }
    }

    long current_start_index = 0;
    compute_starttimes_global_clocks(skew_options, tstart_sec, current_start_index, benchmark_options->n_rep, max_process_skew, sync_errorcodes);

    if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC)) {
        long valid_nreps = 0;

        // don't count measurements with out-of-window errors as valid,
        // but display the result anyway, because we want to see
        // the process skew
        if (skew_options->use_window)
        {
            for (int i = 0; i < benchmark_options->n_rep; i++)
            {
                if (sync_errorcodes[i] == 0)
                {
                    valid_nreps++;
                }
            }
        }
        else
        {
            valid_nreps = benchmark_options->n_rep;
        }

        gsl_sort(max_process_skew, 1, benchmark_options->n_rep);
        fprintf(f, "%14s %10ld %10ld ", MPI_CALL_NAME, benchmark_options->n_rep, valid_nreps);

        if (benchmark_options->print_summary_methods > 0)
        {
          for (int i=0; i<reprompib_get_number_summary_methods(); i++)
          {
            summary_method_info_t* s = reprompib_get_summary_method(i);

            if (benchmark_options->print_summary_methods & s->mask)
            {
              double value = 0;

              if (strcmp(s->name, "mean") == 0)
              {
                value = gsl_stats_mean(max_process_skew, 1, benchmark_options->n_rep);
              }
              else if (strcmp(s->name, "median") == 0)
              {
                value = gsl_stats_quantile_from_sorted_data (max_process_skew, 1, benchmark_options->n_rep, 0.5);
              }
              else if (strcmp(s->name, "min") == 0)
              {
                if (benchmark_options->n_rep > 0) {
                  value = max_process_skew[0];
                }
              }
              else if (strcmp(s->name, "max") == 0)
              {
                if (benchmark_options->n_rep > 0) {
                  value = max_process_skew[benchmark_options->n_rep-1];
                }
              }
              fprintf(f, "  %.10f ", value);
            }
          }
        }
        fprintf(f, "\n");

        free(sync_errorcodes);
        free(max_process_skew);
    }
}

static void print_skewtimes(FILE* f,const skew_options_t* skew_options, const reprompib_options_t* benchmark_options, double* tstart_sec)
{
    double* max_process_skew = NULL;
    int* sync_errorcodes = NULL;
    if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC))
    {
        max_process_skew = (double*) malloc(benchmark_options->n_rep * sizeof(double));

        if (skew_options->use_window)
        {
            sync_errorcodes = (int*) malloc(benchmark_options->n_rep * sizeof(int));
            for (int i = 0; i < benchmark_options->n_rep; i++)
            {
                sync_errorcodes[i] = 0;
            }
        }
    }

    long current_start_index = 0;
    compute_starttimes_global_clocks(skew_options, tstart_sec, current_start_index, benchmark_options->n_rep, max_process_skew, sync_errorcodes);

    if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC))
    {

        for (int i = 0; i < benchmark_options->n_rep; i++)
        {
            fprintf(f, "%14s %10d ", MPI_CALL_NAME, i);

            if (skew_options->use_window)
            {
                fprintf(f, "%10d ", sync_errorcodes[i]);
            }

            fprintf(f, "%14.10f\n", max_process_skew[i]);
        }

        free(sync_errorcodes);
        free(max_process_skew);
    }
}

static void print_measurements(FILE* f,const skew_options_t* skew_options, const reprompib_options_t* benchmark_options, double* tstart_sec)
{
    if (benchmark_options->verbose == 0) {
        print_skewtimes(f, skew_options, benchmark_options, tstart_sec);
    }
    else
    {
        // first we calculate the minimum start time for each sample
        // so that we know the skew of each process in that sample
        double* tmp_minimum_start_sec_global_time = (double*) malloc(benchmark_options->n_rep * sizeof(double));
        for (int i=0; i< benchmark_options->n_rep; ++i)
        {
            tmp_minimum_start_sec_global_time[i] = jk_get_normalized_time(tstart_sec[i]);
        }
        double* minimum_start_sec = NULL;
        if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC))
        {
            minimum_start_sec = (double*) malloc(benchmark_options->n_rep * sizeof(double));
        }
        MPI_Reduce(tmp_minimum_start_sec_global_time, minimum_start_sec, benchmark_options->n_rep, MPI_DOUBLE, MPI_MIN, icmb_lookup_global_rank(OUTPUT_ROOT_PROC), icmb_global_communicator());
        free(tmp_minimum_start_sec_global_time);

        // we gather data from processes in chunks of OUTPUT_NITERATIONS_CHUNK elements
        // the total number of chunks depends on the number of repetitions of the current exp benchmark_options->n_rep
        int nchunks = benchmark_options->n_rep/OUTPUT_NITERATIONS_CHUNK + (benchmark_options->n_rep % OUTPUT_NITERATIONS_CHUNK != 0);

        int chunk_nrep = 0;
        for (int chunk_id = 0; chunk_id < nchunks; chunk_id++)
        {
            //the last chunk may be smaller than OUTPUT_NITERATIONS_CHUNK
            if ((chunk_id == nchunks - 1) && (benchmark_options->n_rep % OUTPUT_NITERATIONS_CHUNK != 0))
            {
                chunk_nrep = benchmark_options->n_rep % OUTPUT_NITERATIONS_CHUNK;
            }
            else
            {
                chunk_nrep = OUTPUT_NITERATIONS_CHUNK;
            }

            int* errorcodes = NULL;
            if (skew_options->use_window)
            {
                if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC))
                {
                    errorcodes = (int*)malloc(chunk_nrep * icmb_global_size() * sizeof(int));
                    for (int i = 0; i < chunk_nrep * icmb_global_size(); i++)
                    {
                        errorcodes[i] = 0;
                    }
                }

                int* local_errorcodes = jk_get_local_sync_errorcodes();
                MPI_Gather(local_errorcodes + (chunk_id * OUTPUT_NITERATIONS_CHUNK), chunk_nrep, MPI_INT, errorcodes, chunk_nrep, MPI_INT, icmb_lookup_global_rank(OUTPUT_ROOT_PROC), icmb_global_communicator());
            }

            double* local_start_sec = NULL;
            double* global_start_sec = NULL;
            if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC))
            {
                local_start_sec = (double*) malloc(chunk_nrep * icmb_global_size() * sizeof(double));
                global_start_sec = (double*) malloc(chunk_nrep * icmb_global_size() * sizeof(double));
            }

            // gather measurement results
            MPI_Gather(tstart_sec + (chunk_id * OUTPUT_NITERATIONS_CHUNK), chunk_nrep, MPI_DOUBLE, local_start_sec, chunk_nrep, MPI_DOUBLE, icmb_lookup_global_rank(OUTPUT_ROOT_PROC), icmb_global_communicator());
            for (int i = 0; i < chunk_nrep; i++)
            {
                int current_rep_id = chunk_id * OUTPUT_NITERATIONS_CHUNK + i;
                tstart_sec[current_rep_id] = jk_get_normalized_time(tstart_sec[current_rep_id]);
            }

            MPI_Gather(tstart_sec + (chunk_id * OUTPUT_NITERATIONS_CHUNK), chunk_nrep, MPI_DOUBLE, global_start_sec, chunk_nrep, MPI_DOUBLE, icmb_lookup_global_rank(OUTPUT_ROOT_PROC), icmb_global_communicator());

            if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC))
            {

                for (int proc_id = 0; proc_id < icmb_global_size(); proc_id++)
                {
                    for (int i = 0; i < chunk_nrep; i++)
                    {
                        int current_rep_id = chunk_id * OUTPUT_NITERATIONS_CHUNK + i;
                        fprintf(f, "%7d %14s %10d ", proc_id, MPI_CALL_NAME, current_rep_id);

                        if (skew_options->use_window)
                        {
                            fprintf(f, "%10d ", errorcodes[proc_id * chunk_nrep + i]);
                        }

                        fprintf(f, "%17.10f %17.10f ", local_start_sec[proc_id * chunk_nrep + i], global_start_sec[proc_id * chunk_nrep + i]);
                        fprintf(f, "%14.10f\n", global_start_sec[proc_id * chunk_nrep + i] - minimum_start_sec[current_rep_id]);
                    }
                }

                free(local_start_sec);
                free(global_start_sec);
                free(errorcodes);
            }

        }

        free(minimum_start_sec);
    }
}

void print_result(const skew_options_t* skew_options, const reprompib_options_t* benchmark_options, double* tstart_sec)
{
    FILE* f = stdout;

    if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC))
    {
        if (skew_options->output_file != NULL)
        {
            f = fopen(skew_options->output_file, "a");
        }
    }

    if (benchmark_options->print_summary_methods >0)
    {
        print_summary(stdout, skew_options, benchmark_options, tstart_sec);
        if (skew_options->output_file != NULL)
        {
            print_measurements(f, skew_options, benchmark_options, tstart_sec);
        }
    }
    else
    {
        print_measurements(f, skew_options, benchmark_options, tstart_sec);
    }

    if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC))
    {
        if (skew_options->output_file != NULL)
        {
            fflush(f);
            fclose(f);
        }
    }
}

void print_final(const skew_options_t* skew_options, const time_t start_time, const time_t end_time)
{
    if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC)) {
        FILE* f;
        f = stdout;
        fprintf (f, "# Benchmark started at %s", asctime (localtime (&start_time)));
        fprintf (f, "# Execution time: %lds\n", (long int)(end_time-start_time));

        if (skew_options->output_file != NULL) {
            f = fopen(skew_options->output_file, "a");
            fprintf (f, "# Benchmark started at %s", asctime (localtime (&start_time)));
            fprintf (f, "# Execution time: %lds\n", (long int)(end_time-start_time));
            fflush(f);
            fclose(f);
        }
    }
}
