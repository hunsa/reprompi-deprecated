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
#include <time.h>
#include "mpi.h"

#include "reprompi_bench/misc.h"
#include "reprompi_bench/sync/synchronization.h"
#include "reprompi_bench/sync/time_measurement.h"
#include "reprompi_bench/option_parser/parse_options.h"
#include "reprompi_bench/option_parser/parse_common_options.h"
#include "reprompi_bench/option_parser/option_parser_helpers.h"
#include "reprompi_bench/output_management/bench_info_output.h"
#include "reprompi_bench/utils/keyvalue_store.h"
#include "results_output.h"
#include "reproMPIbenchmark.h"

static const int OUTPUT_ROOT_PROC = 0;
static const int N_USER_VARS = 4;

static int first_print_call = 1;

static reprompib_dictionary_t params_dict;


void print_initial_settings(reprompib_options_t opts, print_sync_info_t print_sync_info, const reprompib_dictionary_t* dict) {
    int my_rank, np;
    FILE* f = stdout;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    if (my_rank == OUTPUT_ROOT_PROC) {
        print_common_settings_to_file(f, opts.common_opt, print_sync_info, dict);
        fprintf(f, "#@nrep=%ld\n", opts.n_rep);

        if (opts.common_opt.output_file != NULL) {
            f = fopen(opts.common_opt.output_file, "a");
            print_common_settings_to_file(f, opts.common_opt, print_sync_info, dict);
            fprintf(f, "#@nrep=%ld\n", opts.n_rep);
            fflush(f);
            fclose(f);
        }
    }

}


void reprompib_print_bench_output(reprompib_job_t job, double* tstart_sec, double* tend_sec,
        reprompib_sync_functions_t sync_f,
        reprompib_options_t opts,
        char* op, char* timername, char* timertype) {
    FILE* f = stdout;
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    if (my_rank == OUTPUT_ROOT_PROC) {
        if (opts.common_opt.output_file != NULL) {
            f = fopen(opts.common_opt.output_file, "a");
        }
    }

    if (first_print_call) {
        print_initial_settings(opts, sync_f.print_sync_info, &params_dict);
        print_results_header(opts, job);
        first_print_call = 0;
    }

    if (opts.n_print_summary_selected >0)  {
        print_summary(stdout, job, tstart_sec, tend_sec, sync_f.get_errorcodes,  sync_f.get_normalized_time,
                op, timername, timertype, opts.print_summary_methods);

        if (opts.common_opt.output_file != NULL) {
            print_measurement_results(f, job, tstart_sec, tend_sec,
                    sync_f.get_errorcodes,  sync_f.get_normalized_time,
                    opts.common_opt.verbose, op, timername, timertype);
        }

    }
    else {
        print_measurement_results(f, job, tstart_sec, tend_sec,
                sync_f.get_errorcodes,  sync_f.get_normalized_time,
                opts.common_opt.verbose, op, timername, timertype);
    }

    if (my_rank == OUTPUT_ROOT_PROC) {
        if (opts.common_opt.output_file != NULL) {
            fflush(f);
            fclose(f);
        }
    }

}


void reprompib_initialize_benchmark(int argc, char* argv[], reprompib_sync_functions_t* sync_f_p, reprompib_options_t *opts_p) {
    reprompib_error_t ret;
    int my_rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    // initialize time measurement functions
    init_timer();

    //initialize dictionary
    reprompib_init_dictionary(&params_dict);

    // parse arguments and set-up benchmarking jobs
    print_command_line_args(argc, argv);

    ret = reprompib_parse_options(opts_p, argc, argv, &params_dict);
    ret = ret & (~ERROR_MPI_CALL_LIST_EMPTY) & (~ERROR_MSIZE_LIST_EMPTY);   // ignore these options
    reprompib_validate_common_options_or_abort(ret, &(opts_p->common_opt), reprompib_print_benchmark_help);

    // initialize synchronization functions according to the configured synchronization method
    initialize_sync_implementation(sync_f_p);

    // start synchronization module
    ret = sync_f_p->init_sync_module(argc, argv, opts_p->n_rep);
    reprompib_validate_common_options_or_abort(ret, &(opts_p->common_opt), reprompib_print_benchmark_help);
    if (ret != 0) {
        reprompib_print_benchmark_help();
        MPI_Finalize();
        exit(1);
    }

}


void reprompib_initialize_job(int nrep, reprompib_job_t* job) {
    job->n_rep = nrep;

    job->testname = NULL;
    job->n_user_ivars = 0;
    job->n_user_svars = 0;
    job->user_ivars = NULL;
    job->user_svars = NULL;
    job->user_svar_names = NULL;
    job->user_ivar_names = NULL;

}

void reprompib_cleanup_job(reprompib_job_t job) {
    if (job.testname != NULL) {
        free(job.testname);
    }
    if (job.user_svars != NULL) {
        int i;
        for (i=0; i<job.n_user_svars; i++) {
            free(job.user_svars[i]);
            free(job.user_svar_names[i]);
        }
        free(job.user_svars);
        job.user_svars = NULL;
        job.n_user_svars = 0;
    }
    if (job.user_ivars != NULL) {
        int i;
        for (i=0; i<job.n_user_ivars; i++) {
            free(job.user_ivar_names[i]);
        }
        free(job.user_ivars);
        job.user_ivars = NULL;
        job.n_user_ivars = 0;
    }
}


void reprompib_add_svar_to_job(char* name, char* s, reprompib_job_t* job_p) {
    if (job_p->n_user_svars % N_USER_VARS == 0) {
        job_p->user_svars = (char**) realloc(job_p->user_svars, (job_p->n_user_svars + N_USER_VARS) * sizeof(char*));
        job_p->user_svar_names = (char**) realloc(job_p->user_svar_names, (job_p->n_user_svars + N_USER_VARS) * sizeof(char*));
    }

    job_p->user_svars[job_p->n_user_svars] = strdup(s);
    job_p->user_svar_names[job_p->n_user_svars] = strdup(name);
    job_p->n_user_svars++;

}


void reprompib_add_ivar_to_job(char* name, int v, reprompib_job_t* job_p) {
    if (job_p->n_user_ivars % N_USER_VARS == 0) {
        job_p->user_ivars = (int*) realloc(job_p->user_ivars, (job_p->n_user_ivars + N_USER_VARS) * sizeof(int));
        job_p->user_ivar_names = (char**) realloc(job_p->user_ivar_names, (job_p->n_user_ivars + N_USER_VARS) * sizeof(char*));
    }
    job_p->user_ivars[job_p->n_user_ivars] = v;
    job_p->user_ivar_names[job_p->n_user_ivars] = strdup(name);
    job_p->n_user_ivars++;
}

void reprompib_cleanup_benchmark(reprompib_options_t opts) {
    reprompib_free_parameters(&opts);
    reprompib_cleanup_dictionary(&params_dict);
}

