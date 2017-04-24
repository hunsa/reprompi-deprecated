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
#include <math.h>
#include <time.h>
#include <assert.h>
#include <mpi.h>

#include <gsl/gsl_statistics.h>
#include <gsl/gsl_sort.h>

#include "reprompi_bench/option_parser/parse_common_options.h"
#include "reprompi_bench/sync/synchronization.h"
#include "reprompi_bench/output_management/bench_info_output.h"
#include "reprompi_bench/output_management/runtimes_computation.h"
#include "benchmark_job.h"
#include "parse_nrep_pred_options.h"
#include "nrep_pred.h"

static const int OUTPUT_ROOT_PROC = 0;

typedef enum nrep_pred_state {
  NREP_PRED_OK = 0,
  NREP_PRED_NOT_DONE
} nrep_pred_state_t;


typedef struct summary {
  double median;
  double min;
  double max;
} array_summary_t;

// can only be done at the root
static nrep_pred_state_t check_prediction_ready(const double* maxRuntimes_sec, const long measured_nreps,
    const double rse_threshold, const int root_proc) {
  int my_rank;
  double mean, sd, rse;
  nrep_pred_state_t state = NREP_PRED_OK;

  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  if (my_rank != root_proc) {
    return NREP_PRED_NOT_DONE;
  }

  mean = gsl_stats_mean(maxRuntimes_sec, 1, measured_nreps);
  sd = gsl_stats_sd(maxRuntimes_sec, 1, measured_nreps);

  assert(measured_nreps > 0);
  assert(mean > 0);
  rse = sd / (sqrt(measured_nreps) * mean);

  // check whether the current prediction round is enough (rse of the measured values < threshold)
  if (rse > rse_threshold) {
    state = NREP_PRED_NOT_DONE;
  }
  return state;
}

static int compute_array_summary(const double* my_array, const long array_length, array_summary_t* summ) {
  int i;
  double tmp_array[array_length];

  if (array_length <= 0) {   // incorrect length
    fprintf(stderr, "ERROR: Cannot compute summary for array length: %ld\n", array_length);
    return 1;
  }

  for (i = 0; i < array_length; i++) {
    tmp_array[i] = my_array[i];
  }

  //for (i=0; i< array_length; i++) {
  //  printf("%d %.10f\n", i, tmp_array[i]);
  //}

  gsl_sort(tmp_array, 1, array_length);
  summ->median = gsl_stats_quantile_from_sorted_data(tmp_array, 1, array_length, 0.5);
  summ->min = tmp_array[0];
  summ->max = tmp_array[array_length-1];
  return 0;
}

static long compute_nreps(const double ref_runtime_s, const double time_limit_s, const int root_proc) {
  long nreps;
  int my_rank;

  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  if (my_rank != root_proc) {
    return -1;
  }
  //printf("median=%.10f\n", ref_runtime_s);
  assert(ref_runtime_s != 0);

  nreps = time_limit_s / ref_runtime_s;
  return nreps;
}

static void nrep_pred_print_prediction_results(const job_t* job, const reprompib_common_options_t* opts,
    const nrep_pred_options_t* pred_params, const long measured_nreps, const long estimated_nreps,
    const array_summary_t* summ) {
  int my_rank;
  FILE* f;

  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  f = stdout;
  if (my_rank == OUTPUT_ROOT_PROC) {
    if (opts->output_file != NULL) {
      f = fopen(opts->output_file, "a");
    }

    fprintf(f, "%25s %10ld %10ld     %.10f     %.10f %10ld \n", get_call_from_index(job->call_index), job->count, measured_nreps,
        summ->min, summ->median, estimated_nreps);
  }

  if (opts->output_file != NULL) {
    fclose(f);
  }
}

void nrep_pred_print_results_header(const char* filename) {
  int my_rank;
  FILE* f;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  f = stdout;
  if (my_rank == OUTPUT_ROOT_PROC) {
    if (filename != NULL) {
      f = fopen(filename, "a");
    }
    fprintf(f, "%25s %10s %10s %16s %16s %10s\n", "test", "count", "meas_nreps", "meas_min_sec", "meas_median_sec", "nreps");
    if (filename != NULL) {
      fclose(f);
    }
  }
}

int main(int argc, char* argv[]) {
  int my_rank, procs;
  long i, jindex, current_index;
  double* tstart_sec;
  double* tend_sec;
  double* maxRuntimes_sec = NULL;
  reprompib_error_t ret;
  collective_params_t coll_params;
  int stop_meas;
  basic_collective_params_t coll_basic_info;
  time_t start_time, end_time;
  reprompib_sync_functions_t sync_f;
  reprompib_dictionary_t params_dict;

  nrep_pred_options_t pred_params;
  job_list_t jlist;
  int round, max_nreps;
  reprompib_common_options_t opts;
  long estimated_nreps;
  double* round_maxRuntimes_sec;
  long round_start_index;

  /* start up MPI
   * */
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &procs);

  start_time = time(NULL);

  // initialize global dictionary
  reprompib_init_dictionary(&params_dict);

  // initialize synchronization functions according to the configured synchronization method
  initialize_sync_implementation(&sync_f);

  // parse arguments and set-up benchmarking jobs
  // print_command_line_args(argc, argv);
  ret = reprompib_parse_common_options(&opts, argc, argv, &params_dict);
  reprompib_validate_common_options_or_abort(ret, &opts, reprompib_print_benchmark_help);
  if (ret != SUCCESS) {
    fprintf(stderr, "ERROR: parsing command line arguments\n");
    MPI_Finalize();
    exit(0);
  }
  // start synchronization module
  ret = sync_f.init_sync_module(argc, argv, pred_params.max_nrep);
  if (ret != SUCCESS) {
    fprintf(stderr, "ERROR: parsing command line arguments\n");
    MPI_Finalize();
    exit(0);
  }

  // parse command-line arguments for the prediction module
  nrep_pred_parse_params(argc, argv, &pred_params);
  max_nreps = 0;
  for (i = 0; i < pred_params.n_pred_rounds; i++) {
    max_nreps += pred_params.nrep_per_pred_round[i];
  }
  if (pred_params.max_nrep < max_nreps) {
    max_nreps = pred_params.max_nrep;
  }

  // generate list of jobs ((mpifunc, count) tuples) with nrep=0 for each of them
  generate_job_list(&opts, 0, &jlist);

  init_collective_basic_info(opts, procs, &coll_basic_info);

  // execute the benchmark jobs
  for (jindex = 0; jindex < jlist.n_jobs; jindex++) {
    long current_nreps;
    job_t job;
    job = jlist.jobs[jlist.job_indices[jindex]];

    if (jindex == 0) {
      print_common_settings(opts, sync_f.print_sync_info, &params_dict);
      nrep_pred_print_cli_args_to_file(opts.output_file, &pred_params);
      nrep_pred_print_results_header(opts.output_file);
    }

    tstart_sec = (double*) malloc(max_nreps * sizeof(double));
    tend_sec = (double*) malloc(max_nreps * sizeof(double));
    maxRuntimes_sec = (double*) malloc(max_nreps * sizeof(double));

    collective_calls[job.call_index].initialize_data(coll_basic_info, job.count, &coll_params);

    current_index = 0;
    for (round = 0; round < pred_params.n_pred_rounds; round++) {

      // make sure we don't perform more measurements than max_nreps
      current_nreps = pred_params.nrep_per_pred_round[round];
      if (current_index + pred_params.nrep_per_pred_round[round] > max_nreps) {
        current_nreps = max_nreps - current_index;
      }

      // start a new round of measurements
      for (i = 0; i < current_nreps; i++) {
        sync_f.start_sync();

        tstart_sec[current_index] = sync_f.get_time();
        collective_calls[job.call_index].collective_call(&coll_params);
        tend_sec[current_index] = sync_f.get_time();
        current_index++;

        sync_f.stop_sync();
      }

      // gather the run-times obtained in this round to the root
      round_start_index = current_index - current_nreps;
      round_maxRuntimes_sec = maxRuntimes_sec + round_start_index;
      compute_runtimes_local_clocks(tstart_sec, tend_sec, round_start_index, current_nreps,
          OUTPUT_ROOT_PROC, round_maxRuntimes_sec);

      // verify the prediction stopping conditions and broadcast them to all processes
      stop_meas = check_prediction_ready(maxRuntimes_sec, current_index, pred_params.threshold, OUTPUT_ROOT_PROC);
      MPI_Bcast(&stop_meas, 1, MPI_INT, OUTPUT_ROOT_PROC, MPI_COMM_WORLD);
      if (stop_meas == NREP_PRED_OK) {
        break;
      }

    }

    // estimate needed nreps based on the measured run-times in maxRuntimes_sec
    if (my_rank == OUTPUT_ROOT_PROC) {  // data is only on the root process
      array_summary_t summ;

      ret = compute_array_summary(maxRuntimes_sec, current_index, &summ);
      if (ret) {
        fprintf(stderr, "ERROR: Cannot compute summary for %ld elements\n", current_index);
        MPI_Finalize();
        exit(1);
      }

      estimated_nreps = compute_nreps(summ.min, pred_params.time_limit_s, OUTPUT_ROOT_PROC);

      if (estimated_nreps < pred_params.min_nrep) {
        fprintf(stderr, "WARNING: %s, count=%ld: Estimated nreps too small (%ld). Using specified min: %ld\n",
            get_call_from_index(job.call_index), job.count,
            estimated_nreps,
            pred_params.min_nrep);
        estimated_nreps = pred_params.min_nrep;
      } else if (estimated_nreps > pred_params.max_nrep) {
        fprintf(stderr, "WARNING: %s, count=%ld: Estimated nreps too large (%ld). Using specified max: %ld\n",
            get_call_from_index(job.call_index), job.count,
            estimated_nreps,
            pred_params.max_nrep);

        estimated_nreps = pred_params.max_nrep;
      }

      // print_results
      nrep_pred_print_prediction_results(&job, &opts, &pred_params, current_index, estimated_nreps, &summ);
    }

    collective_calls[job.call_index].cleanup_data(&coll_params);

    free(tstart_sec);
    free(tend_sec);
    free(maxRuntimes_sec);
  }

  sync_f.clean_sync_module();
  end_time = time(NULL);
  print_final_info(opts, start_time, end_time);

  cleanup_job_list(jlist);
  reprompib_free_common_parameters(&opts);

  reprompib_cleanup_dictionary(&params_dict);

  /* shut down MPI */
  MPI_Finalize();

  return 0;
}
