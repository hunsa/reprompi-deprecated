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

// avoid getsubopt bug
#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <string.h>
#include "mpi.h"
#include "reprompi_bench/misc.h"
#include "prediction_methods/prediction_data.h"
#include "nrep_estimation.h"
#include "reprompi_bench/option_parser/option_parser_helpers.h"
//#include "reprompi_bench/option_parser/option_parser_constants.h"
#include "parse_options.h"

static const double DEFAULT_THRES = 0.02;
static const int DEFAULT_WIN = 10;
static const int OUTPUT_ROOT_PROC = 0;

enum {
  MIN = 0, MAX, STEP
} pred_steps;

static char * const pred_interval_opts[] = {
    [MIN] = "min",
    [MAX] = "max",
    [STEP] = "step",
NULL };



enum {
  REPROMPI_ARGS_NREPPRED_HELP = 'h',
  REPROMPI_ARGS_NREPPRED_NREP_LIMITS = 600,
  REPROMPI_ARGS_NREPPRED_PRED_METHOD,
  REPROMPI_ARGS_NREPPRED_VAR_THRES,
  REPROMPI_ARGS_NREPPRED_VAR_WIN,
} reprompi_nrep_pred_getopt_ids_t;


const struct option pred_long_options[] = {
    { "rep-prediction", required_argument, 0, REPROMPI_ARGS_NREPPRED_NREP_LIMITS },
    { "pred-method", required_argument, 0, REPROMPI_ARGS_NREPPRED_PRED_METHOD },
    { "var-thres", required_argument, 0, REPROMPI_ARGS_NREPPRED_VAR_THRES },
    { "var-win", required_argument, 0, REPROMPI_ARGS_NREPPRED_VAR_WIN },
    { "help", no_argument, 0, REPROMPI_ARGS_NREPPRED_HELP},
    { 0, 0, 0, 0 } };
const char pred_opts_str[] = "h";


static void init_parameters(nrep_pred_params_t* opts_p) {
  int i;

  opts_p->n_rep_min = 0;
  opts_p->n_rep_max = 0;
  opts_p->n_rep_stride = 0;

  opts_p->n_methods = 1;
  opts_p->info[0].method = RSE;
  opts_p->info[0].method_thres = DEFAULT_THRES;
  opts_p->info[0].method_win = DEFAULT_WIN;

  for (i = 1; i < N_PRED_METHODS; i++) {
    opts_p->info[i].method_thres = -1;
    opts_p->info[i].method_win = -1;
  }

}


void reprompib_print_prediction_help(void) {
    int my_rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    if (my_rank == OUTPUT_ROOT_PROC) {
        printf("\nUSAGE: mpibenchmarkPredNreps [options]\n");
        printf("options:\n");
    }

    reprompib_print_common_help();

    if (my_rank == OUTPUT_ROOT_PROC) {
        printf("\nSpecific options for estimating the number of repetitions:\n");
        printf("%-40s %-40s\n %50s%s\n %50s%s\n %50s%s\n",
                "--rep-prediction min=<min>,max=<max>,step=<step>",
                "set the total number of repetitions to be estimated between <min> and <max>,",
                "", "so that at each iteration i, the number of measurements (nrep) is either",
                "", "nrep(0) = <min>, or nrep(i) = nrep(i-1) + <step> * 2^(i-1),  ", "",
                "e.g., --rep-prediction min=1,max=4,step=1");
        printf("%-40s %-40s\n", "--pred-method=m1,m2",
                " comma-separated list of prediction methods, i.e., rse, cov_mean, cov_median (default: rse)");
        printf("%-40s %-40s\n %50s%s\n", "--var-thres=thres1,thres2",
                " comma-separated list of thresholds corresponding to the specified prediction methods ",
                "", "(default: 0.01)");
        printf("%-40s %-40s\n %50s%s\n %50s%s\n %50s%s\n", "--var-win=win1,win2",
                " comma-separated list of (non-zero) windows corresponding to the specified prediction methods;",
                "", "rse does not rely on a measurement window, however a dummy window value is required ",
                "", "in this list when multiple methods are used ",
                "", "(default: 10)");

        printf("\nEXAMPLES: mpirun -np 4 ./bin/mpibenchmarkPredNreps --calls-list=MPI_Bcast --msizes-list=1024 --rep-prediction min=1,max=100,step=1\n");
        printf("\n          mpirun -np 4 ./bin/mpibenchmarkPredNreps --calls-list=MPI_Bcast --msizes-list=8,512,1024 --rep-prediction min=1,max=100,step=1 \n");
        printf("\n          mpirun -np 4 ./bin/mpibenchmarkPredNreps --calls-list=MPI_Bcast --msizes-list=1024 --rep-prediction min=1,max=100,step=1 --pred-method=cov_mean,rse --var-thres=0.01,0.1 --var-win=10,1\n");

        printf("\n\n");
    }
}



void parse_nreps_interval(char* subopts, nrep_pred_params_t* opts_p) {
  char * value;
  int err;
  long min, max, step;

  while (*subopts != '\0') {
    switch (getsubopt(&subopts, pred_interval_opts, &value)) {
    case MIN:
      err = reprompib_str_to_long(value, &min);
      if (err || min <= 0) {
        reprompib_print_error_and_exit("Min nreps value is null or not correctly specified");
      }
      opts_p->n_rep_min = min;
      break;
    case MAX:
      err = reprompib_str_to_long(value, &max);
      if (err || max <= 0) {
        reprompib_print_error_and_exit("Max nreps value is null or not correctly specified");
      }
      opts_p->n_rep_max = max;
      break;
    case STEP:
      err = reprompib_str_to_long(value, &step);
      if (err || step <= 0) {
        reprompib_print_error_and_exit("Step value for nreps is null or not correctly specified");
      }
      opts_p->n_rep_stride = step;
      break;

    default:
      /* Unknown suboption. */
      reprompib_print_error_and_exit(
          "Nreps interval is not correctly specified (--rep-prediction min=<min>,max=<max>,step=<step>)");
      break;
    }
  }

  if (opts_p->n_rep_max - opts_p->n_rep_min < 0) /* the nreps interval option does not have valid values  */
  {
    reprompib_print_error_and_exit(
        "Nreps interval is not correctly specified - max value should be smaller than min (--rep-prediction min=<min>,max=<max>,step=<step>)");
  }

}

void parse_prediction_methods(char* subopts, nrep_pred_params_t* opts_p) {
  char * value;
  int index;

  opts_p->n_methods = 0;
  while (*subopts != '\0') {
    index = getsubopt(&subopts, get_prediction_methods_list(), &value);
    if (index >= 0 && index < N_PRED_METHODS) {
      opts_p->info[opts_p->n_methods++].method = index;
    } else {
      reprompib_print_error_and_exit(
          "Unknown prediction method (--pred-method=<comma-separated list of prediction methods>, i.e., rse, cov_mean, cov_median (default: rse))");
      break;
    }
    if (opts_p->n_methods > N_PRED_METHODS) {
      reprompib_print_error_and_exit(
          "Specified list of prediction methods is incorrect (--pred-method=<comma-separated list of prediction methods>, i.e., rse, cov_mean, cov_median (default: rse))");
      break;
    }
  }

  if (opts_p->n_methods <= 0) {
    reprompib_print_error_and_exit(
        "Specified list of prediction methods is empty (--pred-method=<comma-separated list of prediction methods>, i.e., rse, cov_mean, cov_median (default: rse))");
  }

}

void parse_prediction_thresholds(char* subopts, nrep_pred_params_t* opts_p) {
  char* thres_tok;
  char* save_str;
  char* s;
  int index;
  const int THRES_STR_LEN = 32;

  save_str = (char*) malloc(THRES_STR_LEN * sizeof(char));
  s = save_str;

  /* Parse the list of message sizes */
  if (subopts != NULL) {
    index = 0;

    thres_tok = strtok_r(subopts, ",", &save_str);
    while (thres_tok != NULL) {
      double thr = atof(thres_tok);

      if (thr <= 0 || index >= N_PRED_METHODS) {
        reprompib_print_error_and_exit(
            "Specified list of prediction thresholds is incorrect (--var-thres=<comma-separated list of thresholds corresponding to the specified prediction methods>)");
        thres_tok = strtok_r(NULL, ",", &save_str);
        break;
      }

      opts_p->info[index++].method_thres = thr;
      thres_tok = strtok_r(NULL, ",", &save_str);
    }

    // make sure we have valid thresholds to test
    if (index == 0) {
      reprompib_print_error_and_exit(
          "Specified list of prediction thresholds is empty (--var-thres=<comma-separated list of thresholds corresponding to the specified prediction methods>)");
    }
  }

  free(s);
}

void parse_prediction_windows(char* subopts, nrep_pred_params_t* opts_p) {
  char* win_tok;
  char* save_str;
  char* s;
  int index;
  const int THRES_STR_LEN = 32;
  int err;

  save_str = (char*) malloc(THRES_STR_LEN * sizeof(char));
  s = save_str;

  /* Parse the list of message sizes */
  if (subopts != NULL) {
    index = 0;

    win_tok = strtok_r(subopts, ",", &save_str);
    while (win_tok != NULL) {
      long win;
      err = reprompib_str_to_long(win_tok, &win);
      if (err || win <= 0 || index >= N_PRED_METHODS) {
        reprompib_print_error_and_exit(
            "Specified list of prediction windows is incorrect (--var-win=<comma-separated list of window sizes corresponding to the specified prediction methods>)");
        win_tok = strtok_r(NULL, ",", &save_str);
        break;
      }

      opts_p->info[index++].method_win = win;
      win_tok = strtok_r(NULL, ",", &save_str);
    }

    // make sure we have valid windows
    if (index == 0) {
      reprompib_print_error_and_exit(
          "Specified list of prediction windows is empty (--var-win=<comma-separated list of window sizes corresponding to the specified prediction methods>)");
    }
  }

  free(s);
}

void reprompib_parse_options(int argc, char** argv, nrep_pred_params_t* opts_p) {
  int c, i;
  int printhelp = 0;

  init_parameters(opts_p);

  opterr = 0;
  while (1) {

    /* getopt_long stores the option index here. */
    int option_index = 0;

    c = getopt_long(argc, argv, pred_opts_str, pred_long_options, &option_index);

    /* Detect the end of the options. */
    if (c == -1)
      break;

    switch (c) {

    case REPROMPI_ARGS_NREPPRED_NREP_LIMITS:
      /* Settings for estimating the number of repetitions */
      parse_nreps_interval(optarg, opts_p);
      break;

    case REPROMPI_ARGS_NREPPRED_PRED_METHOD:
      /* List of methods estimating the number of repetitions */
      parse_prediction_methods(optarg, opts_p);
      break;

    case REPROMPI_ARGS_NREPPRED_VAR_THRES:
      /* List of method thresholds for estimating the number of repetitions */
      parse_prediction_thresholds(optarg, opts_p);
      break;

    case REPROMPI_ARGS_NREPPRED_VAR_WIN:
      /* List of method windows for estimating the number of repetitions */
      parse_prediction_windows(optarg, opts_p);
      break;

    case REPROMPI_ARGS_NREPPRED_HELP:
      reprompib_print_prediction_help();
      printhelp = 1;
      break;

    case '?':
      break;
    }
  }

  if (opts_p->n_rep_max <= 0) {
    reprompib_print_error_and_exit("Max nreps value is null or not correctly specified");
  }

  for (i = 0; i < N_PRED_METHODS; i++) {
    if (i < opts_p->n_methods && opts_p->info[i].method_thres < 0) {
      reprompib_print_error_and_exit(
          "Prediction method threshold list contains invalid or not the same number of values as the specified number of methods");
    }
    if (i < opts_p->n_methods && opts_p->info[i].method_win < 0) {
      reprompib_print_error_and_exit(
          "Prediction method window list contains invalid or not the same number of values as the specified number of methods");
    }
    if (i >= opts_p->n_methods && opts_p->info[i].method_thres >= 0) {
      reprompib_print_error_and_exit("Prediction method threshold list contains too many values");
    }
    if (i >= opts_p->n_methods && opts_p->info[i].method_win >= 0) {
      reprompib_print_error_and_exit("Prediction method window list contains too many values");
    }
  }

  if (printhelp) {
    MPI_Finalize();
    exit(0);
  }

  optind = 1;	// reset optind to enable option re-parsing
  opterr = 1;	// reset opterr to catch invalid options

}

