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
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_sort.h>
#include "mpi.h"

#include "benchmark_job.h"
#include "reprompi_bench/sync/synchronization.h"
#include "reprompi_bench/option_parser/parse_options.h"
#include "collective_ops/collectives.h"
#include "runtimes_computation.h"
#include "results_output.h"

#include "contrib/intercommunication/intercommunication.h"

static const int OUTPUT_ROOT_PROC = 0;
static const int OUTPUT_NITERATIONS_CHUNK = 3000; // approx. 1 MB per process

typedef enum output_msize {
  OUTPUT_MSIZE_BYTES = 0,
  OUTPUT_COUNT
} output_msize_t;

#ifdef OPTION_PRINT_MSIZES_BYTES
static const output_msize_t OUTPUT_MSIZE_TYPE = OUTPUT_MSIZE_BYTES;
#else
static const output_msize_t OUTPUT_MSIZE_TYPE = OUTPUT_COUNT;
#endif

void print_results_header(const reprompib_options_t* opts, const char* output_file_path, int verbose) {

    if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC)) {
        FILE * f;
        char msize_str[16];

        if (OUTPUT_MSIZE_TYPE == OUTPUT_MSIZE_BYTES) {
          strcpy(msize_str, "msize_bytes");
        } else {
          strcpy(msize_str, "count");
        }

        f = stdout;
        // print summary to stdout
        if (opts->print_summary_methods >0) {
          int i;
          fprintf(f, "%50s %12s %10s %10s ", "test", msize_str, "total_nrep", "valid_nrep");

          for (i=0; i<reprompib_get_number_summary_methods(); i++) {
            summary_method_info_t* s = reprompib_get_summary_method(i);
            if (opts->print_summary_methods & s->mask) {
              fprintf(f, "%10s_sec ", s->name);
            }
          }
          fprintf(f, "\n");
        }


        // print results to file (if specified)
        if (output_file_path != NULL) {
            f = fopen(output_file_path, "a");
        }

        if (output_file_path != NULL || opts->print_summary_methods == 0) {
            if (verbose == 1) {    // print measurement times for each process
                fprintf(f, "process ");
            }

#ifdef ENABLE_WINDOWSYNC
            fprintf(f, "%50s %10s %12s %10s ", "test", "nrep", msize_str, "errorcode");
#else
            fprintf(f, "%50s %10s %12s ", "test", "nrep", msize_str);
#endif

            if (verbose == 1) {
#ifdef ENABLE_WINDOWSYNC
                fprintf(f, "%14s %14s %14s %14s \n", "loc_tstart_sec", "loc_tend_sec", "gl_tstart_sec", "gl_tend_sec");
#else
                fprintf(f,  "%14s %14s \n", "loc_tstart_sec", "loc_tend_sec");
#endif
            } else {
                fprintf(f,  "%14s \n", "runtime_sec");
            }
        }

        if (output_file_path != NULL) {
            fflush(f);
            fclose(f);
        }

    }

}






void print_runtimes(FILE* f, job_t job, double* tstart_sec, double* tend_sec,
        sync_errorcodes_t get_errorcodes, sync_normtime_t get_global_time) {

    double* maxRuntimes_sec;
    int i;
    long current_start_index;
    size_t msize_value;
#ifdef ENABLE_WINDOWSYNC
    int* sync_errorcodes = NULL;
#endif

    if (OUTPUT_MSIZE_TYPE == OUTPUT_MSIZE_BYTES) {
      // print msize in bytes
      msize_value = job.msize;
    } else {    // print counts
      msize_value = job.count;
    }


    maxRuntimes_sec = NULL;
    if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC)) {
        maxRuntimes_sec = (double*) malloc(job.n_rep * sizeof(double));

#ifdef ENABLE_WINDOWSYNC
        sync_errorcodes = (int*) malloc(job.n_rep * sizeof(int));
        for (i = 0; i < job.n_rep; i++) {
            sync_errorcodes[i] = 0;
        }
#endif
    }

    current_start_index = 0;

#ifdef ENABLE_WINDOWSYNC
    compute_runtimes_global_clocks(tstart_sec, tend_sec, current_start_index, job.n_rep, OUTPUT_ROOT_PROC,
            get_errorcodes, get_global_time,
            maxRuntimes_sec, sync_errorcodes);
#else
    compute_runtimes_local_clocks(tstart_sec, tend_sec, current_start_index, job.n_rep, OUTPUT_ROOT_PROC,
            maxRuntimes_sec);
#endif

    if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC)) {

        for (i = 0; i < job.n_rep; i++) {

#if defined(ENABLE_WINDOWSYNC) && !defined(ENABLE_BARRIERSYNC)    // measurements with window-based synchronization
            fprintf(f, "%50s %10d %12ld %10d %14.10f\n", get_call_from_index(job.call_index), i,
                    msize_value, sync_errorcodes[i],
                    maxRuntimes_sec[i]);
#else   // measurements with Barrier-based synchronization
            fprintf(f, "%50s %10d %12ld %14.10f\n", get_call_from_index(job.call_index), i,
                    msize_value, maxRuntimes_sec[i]);
#endif
        }

#ifdef ENABLE_WINDOWSYNC
        free(sync_errorcodes);
#endif

        free(maxRuntimes_sec);
    }
}



void print_measurement_results(FILE* f, job_t job, double* tstart_sec, double* tend_sec,
        sync_errorcodes_t get_errorcodes, sync_normtime_t get_global_time,
        int verbose) {

    int i, proc_id;
    double* local_start_sec = NULL;
    double* local_end_sec = NULL;
    double* global_start_sec = NULL;
    double* global_end_sec = NULL;
    int chunk_id, nchunks;
    int current_rep_id, chunk_nrep = 0;
    size_t msize_value;
#ifdef ENABLE_WINDOWSYNC
    int* errorcodes = NULL;
#endif

    if (OUTPUT_MSIZE_TYPE == OUTPUT_MSIZE_BYTES) {
      // print msize in bytes
      msize_value = job.msize;
    } else {    // print counts
      msize_value = job.count;
    }

    if (verbose == 0) {
        print_runtimes(f, job, tstart_sec, tend_sec, get_errorcodes,
                get_global_time);
    } else {

        // we gather data from processes in chunks of OUTPUT_NITERATIONS_CHUNK elements
        // the total number of chunks depends on the number of repetitions of the current exp job.n_rep
        nchunks = job.n_rep/OUTPUT_NITERATIONS_CHUNK + (job.n_rep % OUTPUT_NITERATIONS_CHUNK != 0);

        for (chunk_id = 0; chunk_id < nchunks; chunk_id++) {
            //the last chunk may be smaller than OUTPUT_NITERATIONS_CHUNK
            if ((chunk_id == nchunks - 1) &&
                    (job.n_rep % OUTPUT_NITERATIONS_CHUNK != 0)) {
                chunk_nrep = job.n_rep % OUTPUT_NITERATIONS_CHUNK;
            }
            else {
                chunk_nrep = OUTPUT_NITERATIONS_CHUNK;
            }

#ifdef ENABLE_WINDOWSYNC
            if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC))
            {
              errorcodes = (int*)malloc(chunk_nrep * icmb_global_size() * sizeof(int));
              for (i = 0; i < chunk_nrep * icmb_global_size(); i++) {
                errorcodes[i] = 0;
              }
            }
#ifndef ENABLE_BARRIERSYNC	// gather measurement results
            {
                int* local_errorcodes = get_errorcodes();

                MPI_Gather(local_errorcodes + (chunk_id * OUTPUT_NITERATIONS_CHUNK), chunk_nrep, MPI_INT,
                        errorcodes, chunk_nrep, MPI_INT, icmb_lookup_global_rank(OUTPUT_ROOT_PROC), icmb_global_communicator());
            }
#endif

#endif

            if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC)) {
                local_start_sec = (double*) malloc(
                        chunk_nrep * icmb_global_size() * sizeof(double));
                local_end_sec = (double*) malloc(
                        chunk_nrep * icmb_global_size() * sizeof(double));
                global_start_sec = (double*) malloc(
                        chunk_nrep * icmb_global_size() * sizeof(double));
                global_end_sec = (double*) malloc(
                        chunk_nrep * icmb_global_size() * sizeof(double));
            }

            // gather measurement results
            MPI_Gather(tstart_sec + (chunk_id * OUTPUT_NITERATIONS_CHUNK), chunk_nrep, MPI_DOUBLE, local_start_sec,
                    chunk_nrep, MPI_DOUBLE, icmb_lookup_global_rank(OUTPUT_ROOT_PROC), icmb_global_communicator());

            MPI_Gather(tend_sec + (chunk_id * OUTPUT_NITERATIONS_CHUNK), chunk_nrep, MPI_DOUBLE, local_end_sec, chunk_nrep,
                    MPI_DOUBLE, icmb_lookup_global_rank(OUTPUT_ROOT_PROC), icmb_global_communicator());

            for (i = 0; i < chunk_nrep; i++) {
                current_rep_id = chunk_id * OUTPUT_NITERATIONS_CHUNK + i;
                tstart_sec[current_rep_id] = get_global_time(tstart_sec[current_rep_id]);
                tend_sec[current_rep_id] = get_global_time(tend_sec[current_rep_id]);
            }
            MPI_Gather(tstart_sec + (chunk_id * OUTPUT_NITERATIONS_CHUNK), chunk_nrep, MPI_DOUBLE, global_start_sec,
                    chunk_nrep, MPI_DOUBLE, icmb_lookup_global_rank(OUTPUT_ROOT_PROC), icmb_global_communicator());

            MPI_Gather(tend_sec + (chunk_id * OUTPUT_NITERATIONS_CHUNK), chunk_nrep, MPI_DOUBLE, global_end_sec, chunk_nrep,
                    MPI_DOUBLE, icmb_lookup_global_rank(OUTPUT_ROOT_PROC), icmb_global_communicator());

            if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC)) {

                for (proc_id = 0; proc_id < icmb_global_size(); proc_id++) {
                    for (i = 0; i < chunk_nrep; i++) {
                        current_rep_id = chunk_id * OUTPUT_NITERATIONS_CHUNK + i;
#ifdef ENABLE_WINDOWSYNC
                        fprintf(f, "%7d %50s %10d %12ld %10d %14.10f %14.10f %14.10f %14.10f\n", proc_id,
                                get_call_from_index(job.call_index), current_rep_id, msize_value,
                                errorcodes[proc_id * chunk_nrep + i],
                                local_start_sec[proc_id * chunk_nrep + i],
                                local_end_sec[proc_id * chunk_nrep + i],
                                global_start_sec[proc_id * chunk_nrep + i],
                                global_end_sec[proc_id * chunk_nrep + i]);

#else
                        fprintf(f, "%7d %50s %10d %12ld %14.10f %14.10f\n", proc_id,
                                get_call_from_index(job.call_index), current_rep_id, msize_value,
                                local_start_sec[proc_id * chunk_nrep + i],
                                local_end_sec[proc_id * chunk_nrep + i]);
#endif
                    }
                }

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
}



void print_summary(FILE* f, job_t job, double* tstart_sec, double* tend_sec,
        sync_errorcodes_t get_errorcodes, sync_normtime_t get_global_time,
        const int print_summary_methods) {

    double* maxRuntimes_sec;
    long current_start_index;
    size_t msize_value;
#ifdef ENABLE_WINDOWSYNC
    int i;
    int* sync_errorcodes = NULL;
#endif

    if (OUTPUT_MSIZE_TYPE == OUTPUT_MSIZE_BYTES) {
      // print msize in bytes
      msize_value = job.msize;
    } else {    // print counts
      msize_value = job.count;
    }


    maxRuntimes_sec = NULL;
    if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC)) {
        maxRuntimes_sec = (double*) malloc(job.n_rep * sizeof(double));

#ifdef ENABLE_WINDOWSYNC
        sync_errorcodes = (int*) malloc(job.n_rep * sizeof(int));
        for (i = 0; i < job.n_rep; i++) {
            sync_errorcodes[i] = 0;
        }
#endif
    }

    current_start_index = 0;

#ifdef ENABLE_WINDOWSYNC
    compute_runtimes_global_clocks(tstart_sec, tend_sec, current_start_index, job.n_rep, OUTPUT_ROOT_PROC,
            get_errorcodes, get_global_time,
            maxRuntimes_sec, sync_errorcodes);
#else
    compute_runtimes_local_clocks(tstart_sec, tend_sec, current_start_index, job.n_rep, OUTPUT_ROOT_PROC,
            maxRuntimes_sec);
#endif


    if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC)) {
        long nreps = 0;

        // remove measurements with out-of-window errors
#ifdef ENABLE_WINDOWSYNC
        for (i = 0; i < job.n_rep; i++) {
            if (sync_errorcodes[i] == 0) {
                if (nreps < i) {
                    maxRuntimes_sec[nreps] = maxRuntimes_sec[i];
                }
                nreps++;
            }
        }
#else
        nreps = job.n_rep;
#endif

        gsl_sort(maxRuntimes_sec, 1, nreps);
        fprintf(f, "%50s %12ld %10ld %10ld ", get_call_from_index(job.call_index), msize_value, job.n_rep, nreps);

        if (print_summary_methods > 0) {
          int i;
          for (i=0; i<reprompib_get_number_summary_methods(); i++) {
            summary_method_info_t* s = reprompib_get_summary_method(i);

            if (print_summary_methods & s->mask) {
              double value = 0;

              if (strcmp(s->name, "mean") == 0) {
                value = gsl_stats_mean(maxRuntimes_sec, 1, nreps);
              }
              else if (strcmp(s->name, "median") == 0) {
                value = gsl_stats_quantile_from_sorted_data (maxRuntimes_sec, 1, nreps, 0.5);
              }
              else if (strcmp(s->name, "min") == 0) {
                if (nreps > 0) {
                  value = maxRuntimes_sec[0];
                }
              }
              else if (strcmp(s->name, "max") == 0) {
                if (nreps > 0) {
                  value = maxRuntimes_sec[nreps-1];
                }
              }
              fprintf(f, "  %.10f ", value);
            }
          }
        }
        fprintf(f, "\n");

#ifdef ENABLE_WINDOWSYNC
        free(sync_errorcodes);
#endif

        free(maxRuntimes_sec);
    }
}
