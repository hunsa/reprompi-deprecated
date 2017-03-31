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
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <string.h>
#include <mpi.h>

#include "reprompi_bench/option_parser/option_parser_helpers.h"
#include "parse_nrep_pred_options.h"

static const int OUTPUT_ROOT_PROC = 0;

static const double DEFAULT_THRES = 0.01;
static const int NPRED_ROUNDS = 2;
static const int NREP_PER_PRED_ROUNDS = 5;

enum nrep_pred_getopt_ids {
  MAXNREP = 1000,
  MINNREP,
  THRESHOLD,
  NREP_PER_PRED_ROUND,
  TIME_LIMIT
};
static const struct option pred_long_options[] = {
    { "max-nrep", required_argument, 0, MAXNREP },
    { "min-nrep", required_argument, 0, MINNREP },
    { "threshold", required_argument, 0, THRESHOLD },
    { "nrep-per-pred-round", required_argument, 0, NREP_PER_PRED_ROUND },
    { "time-limit", required_argument, 0, TIME_LIMIT },
    { 0, 0, 0, 0 }
};
static const char pred_opts_str[] = "h";

static void init_parameters(nrep_pred_options_t* opts_p) {
  int i;

  opts_p->max_nrep = 0;
  opts_p->min_nrep = 0;

  opts_p->time_limit_s = 0;
  opts_p->threshold = DEFAULT_THRES;
  opts_p->n_pred_rounds = NPRED_ROUNDS;

  opts_p->nrep_per_pred_round = (int*) calloc(opts_p->n_pred_rounds, sizeof(int));
  for (i = 0; i < opts_p->n_pred_rounds; i++) {
    opts_p->nrep_per_pred_round[i] = NREP_PER_PRED_ROUNDS;
  }
}

void reprompib_nrep_pred_print_help(void) {
  int my_rank;

  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  if (my_rank == OUTPUT_ROOT_PROC) {
    printf("\nUSAGE: nrep_pred [options]\n");
    printf("options:\n");
  }

  reprompib_print_common_help();

  if (my_rank == OUTPUT_ROOT_PROC) {
    printf("\nSpecific options for estimating the number of repetitions:\n");
    printf("%-40s %-40s\n", "--min-nrep=<nrep>", "minimum number of repetitions regardless of run-times");
    printf("%-40s %-40s\n", "--max-nrep=<nrep>", "maximum number of repetitions regardless of run-times");
    printf("%-40s %-40s\n", "--time-limit=<time_s>",
        "total run-time of an experiment (nrep measurements of one MPI call for a fixed message size)");
    printf("%-40s %-40s\n", "--threshold=<th>",
        "relative standard error threshold (limits the number of prediction measurements) (default: 0.01)");
    printf("%-40s %-40s\n", "--nrep-per-pred-round=<nreps>",
        "number of measurements before attempting to predict nrep (default: 5)");

    printf(
        "\nEXAMPLES: mpirun -np 4 ./bin/nrep_pred --calls-list=MPI_Bcast --msizes-list=8,512 --max-nrep=1000 --min-nrep=3 --time-limit=0.01\n");
    printf(
        "\n          mpirun -np 4 ./bin/nrep_pred --calls-list=MPI_Bcast --msizes-list=512 --max-nrep=1000 --min-nrep=3 --threshold=0.02 --time-limit=0.01 --nrep-per-pred-round=5\n");

    printf("\n\n");
  }
}

void nrep_pred_parse_params(int argc, char** argv, nrep_pred_options_t* opts_p) {
  int c, i;
  int printhelp = 0;

  init_parameters(opts_p);

  opterr = 0; // ignore invalid options (may be recognized by the main benchmark parser)
  while (1) {

    /* getopt_long stores the option index here. */
    int option_index = 0;

    c = getopt_long(argc, argv, pred_opts_str, pred_long_options, &option_index);

    //printf("c=%d\n", c);
    /* Detect the end of the options. */
    if (c == -1)
      break;

    switch (c) {
    case MAXNREP:
      /* Max number of repetitions */
      opts_p->max_nrep = atol(optarg);
      break;

    case MINNREP:
      /* Max number of repetitions */
      opts_p->min_nrep = atol(optarg);
      break;

    case THRESHOLD:
      /* Threshold for estimating the number of repetitions */
      opts_p->threshold = atof(optarg);
      break;

    case NREP_PER_PRED_ROUND:
      /* Number of repetitions for each batch of measurements  */
      for (i = 0; i < opts_p->n_pred_rounds; i++) {
        opts_p->nrep_per_pred_round[i] = atoi(optarg);
      }
      break;
    case TIME_LIMIT:
      /* Time limit for the total number of measurements */
      opts_p->time_limit_s = atof(optarg);
      break;

    case 'h':
      reprompib_nrep_pred_print_help();
      printhelp = 1;
      break;

    case '?':
      break;
    }
  }

  // check for errors
  if (opts_p->max_nrep <= 0) {
    reprompib_print_error_and_exit("Invalid max_nrep (should be positive)");
  }
  if (opts_p->min_nrep <= 0) {
    reprompib_print_error_and_exit("Invalid min_nrep (should be positive)");
  }
  if (opts_p->min_nrep >= opts_p->max_nrep) {
    reprompib_print_error_and_exit("Invalid min_nrep (should be smaller than max_nrep)");
  }
  if (opts_p->threshold <= 0) {
    reprompib_print_error_and_exit("Invalid prediction threshold (should be positive)");
  }
  if (opts_p->time_limit_s <= 0) {
    reprompib_print_error_and_exit("Invalid time limit (should be positive)");
  }
  if (opts_p->nrep_per_pred_round[0] <= 1) {
    reprompib_print_error_and_exit("Invalid prediction nrep per round (should be >1)");
  }

  optind = 1;	// reset optind to enable option re-parsing
  opterr = 1;	// reset opterr to catch invalid options
}

void nrep_pred_free_params(nrep_pred_options_t* opts_p) {
  free(opts_p->nrep_per_pred_round);
}

void nrep_pred_print_cli_args_to_file(const char* filename, const nrep_pred_options_t* opts) {
  int my_rank;
  FILE* f;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  f = stdout;
  if (my_rank == OUTPUT_ROOT_PROC) {
    if (filename != NULL) {
      f = fopen(filename, "a");
    }
    fprintf(f, "#@pred_nrep_min=%ld\n", opts->min_nrep);
    fprintf(f, "#@pred_nrep_max=%ld\n", opts->max_nrep);
    fprintf(f, "#@pred_nrep_threshold=%lf\n", opts->threshold);
    fprintf(f, "#@pred_nrep_time_limit_s=%lf\n", opts->time_limit_s);
    fprintf(f, "#\n");
    if (filename != NULL) {
      fclose(f);
    }
  }
}

