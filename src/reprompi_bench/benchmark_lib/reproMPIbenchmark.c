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

#include "reprompi_bench/sync/synchronization.h"
#include "reprompi_bench/sync/time_measurement.h"
#include "reprompi_bench/option_parser/parse_options.h"
#include "reprompi_bench/option_parser/parse_extra_key_value_options.h"
#include "reprompi_bench/output_management/bench_info_output.h"
#include "reprompi_bench/utils/keyvalue_store.h"
#include "results_output.h"
#include "reproMPIbenchmark.h"

static const int OUTPUT_ROOT_PROC = 0;
static const int N_USER_VARS = 4;
static const int HASHTABLE_SIZE=100;

static int first_print_call = 1;

static reprompib_dictionary_t params_dict;

void print_initial_settings(long nrep, print_sync_info_t print_sync_info) {
  int my_rank, np;
  FILE* f = stdout;

  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &np);

  if (my_rank == OUTPUT_ROOT_PROC) {
    fprintf(f, "#@nrep=%ld\n", nrep);
    print_common_settings_to_file(f, print_sync_info, &params_dict);
  }
}

void reprompib_print_bench_output(const reprompib_job_t* job_p,
    const reprompib_sync_functions_t* sync_f, const reprompib_options_t* opts) {
  FILE* f = stdout;
  reprompib_lib_output_info_t output_info;
  int my_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  output_info.verbose = opts->verbose;
  output_info.print_summary_methods = opts->print_summary_methods;

  if (first_print_call) {
    print_initial_settings(opts->n_rep, sync_f->print_sync_info);
    print_results_header(&output_info, job_p);
    first_print_call = 0;
  }

  if (opts->print_summary_methods > 0) {
    print_summary(stdout, &output_info, job_p, sync_f->get_errorcodes, sync_f->get_normalized_time);
  } else {
    print_measurement_results(f, &output_info, job_p, sync_f->get_errorcodes, sync_f->get_normalized_time);
  }

}

void reprompib_initialize_benchmark(int argc, char* argv[], reprompib_sync_functions_t* sync_f_p,
    reprompib_options_t *opts_p) {
  int my_rank;
  reprompib_sync_options_t sync_opts;

  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  // initialize time measurement functions
  init_timer();

  //initialize dictionary
  reprompib_init_dictionary(&params_dict, HASHTABLE_SIZE);

  // parse arguments and set-up benchmarking jobs
  print_command_line_args(argc, argv);

  // parse extra parameters into the global dictionary
  reprompib_parse_extra_key_value_options(&params_dict, argc, argv);

  // parse the benchmark-specific arguments (nreps, summary)
  reprompib_parse_options(opts_p, argc, argv);

  // initialize synchronization functions according to the configured synchronization method
  initialize_sync_implementation(sync_f_p);

  sync_f_p->parse_sync_params(argc, argv, &sync_opts);

  // start synchronization module
  sync_f_p->init_sync_module(sync_opts, opts_p->n_rep);
}

void reprompib_initialize_job(const long nrep,
    const double* tstart, const double* tend,
    const char* operation, const char* timername, const char* timertype,
    reprompib_job_t* job_p) {
  job_p->n_rep = nrep;
  job_p->tstart_sec = tstart;
  job_p->tend_sec = tend;

  job_p->timername = strdup(timername);
  job_p->timertype = strdup(timertype);
  job_p->op = strdup(operation);    // reduce method of timings over processes

  job_p->n_user_ivars = 0;
  job_p->n_user_svars = 0;
  job_p->user_ivars = NULL;
  job_p->user_svars = NULL;
  job_p->user_svar_names = NULL;
  job_p->user_ivar_names = NULL;

}

void reprompib_cleanup_job(reprompib_job_t* job_p) {
  if (job_p->user_svars != NULL) {
    int i;
    for (i = 0; i < job_p->n_user_svars; i++) {
      free(job_p->user_svars[i]);
      free(job_p->user_svar_names[i]);
    }
    free(job_p->user_svars);
    job_p->user_svars = NULL;
    job_p->n_user_svars = 0;
  }
  if (job_p->user_ivars != NULL) {
    int i;
    for (i = 0; i < job_p->n_user_ivars; i++) {
      free(job_p->user_ivar_names[i]);
    }
    free(job_p->user_ivars);
    job_p->user_ivars = NULL;
    job_p->n_user_ivars = 0;
  }

  free(job_p->timername);
  free(job_p->timertype);
  free(job_p->op);

}

void reprompib_add_svar_to_job(char* name, char* s, reprompib_job_t* job_p) {
  if (job_p->n_user_svars % N_USER_VARS == 0) {
    job_p->user_svars = (char**) realloc(job_p->user_svars, (job_p->n_user_svars + N_USER_VARS) * sizeof(char*));
    job_p->user_svar_names = (char**) realloc(job_p->user_svar_names,
        (job_p->n_user_svars + N_USER_VARS) * sizeof(char*));
  }

  job_p->user_svars[job_p->n_user_svars] = strdup(s);
  job_p->user_svar_names[job_p->n_user_svars] = strdup(name);
  job_p->n_user_svars++;

}

void reprompib_add_ivar_to_job(char* name, int v, reprompib_job_t* job_p) {
  if (job_p->n_user_ivars % N_USER_VARS == 0) {
    job_p->user_ivars = (int*) realloc(job_p->user_ivars, (job_p->n_user_ivars + N_USER_VARS) * sizeof(int));
    job_p->user_ivar_names = (char**) realloc(job_p->user_ivar_names,
        (job_p->n_user_ivars + N_USER_VARS) * sizeof(char*));
  }
  job_p->user_ivars[job_p->n_user_ivars] = v;
  job_p->user_ivar_names[job_p->n_user_ivars] = strdup(name);
  job_p->n_user_ivars++;
}



int reprompib_add_parameter_to_bench(const char* key, const char* val) {
  int ret;
  ret = reprompib_add_element_to_dict(&params_dict, key, val);
  return ret;
}

void reprompib_cleanup_benchmark(reprompib_options_t* opts_p) {
  reprompib_free_parameters(opts_p);
  reprompib_cleanup_dictionary(&params_dict);
}

