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

// avoid getsubopt bug
#define _XOPEN_SOURCE 500

// fix strdup warning
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <mpi.h>
#include "collective_ops/collectives.h"
#include "reprompi_bench/misc.h"
#include "parse_common_options.h"

#include "contrib/intercommunication/intercommunication.h"

static const int LEN_MPICALLS_BATCH = 10;
static const int LEN_MSIZES_BATCH = 10;
static const int STRING_SIZE = 256;

static const int OUTPUT_ROOT_PROC = 0;

enum {
    MIN = 0, MAX, STEP
};

static char * const msize_interval_opts[] = {
        [MIN] = "min",
        [MAX] = "max",
        [STEP] = "step",
        NULL
};


enum {
  REPROMPI_ARGS_CALLS_LIST = 300,
  REPROMPI_ARGS_MSIZES_LIST,
  REPROMPI_ARGS_MSIZES_INTERVAL,
  REPROMPI_ARGS_INPUT_FILE,
  REPROMPI_ARGS_OUTPUT_FILE,
  REPROMPI_ARGS_ROOT_PROC,
  REPROMPI_ARGS_OPERATION,
  REPROMPI_ARGS_DATATYPE,
  REPROMPI_ARGS_PINGPONG_RANKS,
  REPROMPI_ARGS_SHUFFLE_JOBS
};


static const struct option reprompi_common_long_options[] = {
        { "calls-list",required_argument, 0, REPROMPI_ARGS_CALLS_LIST },
        { "msizes-list", required_argument, 0, REPROMPI_ARGS_MSIZES_LIST },
        { "msize-interval", required_argument, 0, REPROMPI_ARGS_MSIZES_INTERVAL },
        { "input-file", required_argument, 0, REPROMPI_ARGS_INPUT_FILE },
        { "output-file", required_argument, 0, REPROMPI_ARGS_OUTPUT_FILE },
        {"root-proc", required_argument, 0, REPROMPI_ARGS_ROOT_PROC},
        {"operation", required_argument, 0, REPROMPI_ARGS_OPERATION},
        {"datatype", required_argument, 0, REPROMPI_ARGS_DATATYPE},
        {"pingpong-ranks", required_argument, 0, REPROMPI_ARGS_PINGPONG_RANKS},
        {"shuffle-jobs", no_argument, 0, REPROMPI_ARGS_SHUFFLE_JOBS},
        { 0, 0, 0, 0 }
};
static const char reprompi_common_opts_str[] = "";


static void init_parameters(reprompib_common_options_t* opts_p) {
    opts_p->n_msize = 0;
    opts_p->n_calls = 0;
    opts_p->root_proc = 0;
    opts_p->enable_job_shuffling = 0;

    opts_p->msize_list = NULL;
    opts_p->list_mpi_calls = NULL;

    opts_p->input_file = NULL;
    opts_p->output_file = NULL;
    opts_p->operation = MPI_BOR;
    opts_p->datatype = MPI_BYTE;

    opts_p->pingpong_ranks[0] = -1;
    opts_p->pingpong_ranks[1] = -1;
}

void reprompib_free_common_parameters(const reprompib_common_options_t* opts_p) {
    if (opts_p->msize_list != NULL) {
        free(opts_p->msize_list);
    }
    if (opts_p->list_mpi_calls != NULL) {
        free(opts_p->list_mpi_calls);
    }
    if (opts_p->input_file != NULL) {
        free(opts_p->input_file);
    }
    if (opts_p->output_file != NULL) {
        free(opts_p->output_file);
    }
}


static void parse_msize_interval(char* subopts, reprompib_common_options_t* opts_p) {
    char * value;
    long min = 0, max = 0, step = 1, i, index;
    int err;

    while (*subopts != '\0') {
        switch (getsubopt(&subopts, msize_interval_opts, &value)) {
        case MIN:
          err = reprompib_str_to_long(value, &min);
          if (err || min <= 0) {
            reprompib_print_error_and_exit( "Min message size in interval is null or not correctly specified");
          }
          break;
        case MAX:
          err = reprompib_str_to_long(value, &max);
          if (err || max <= 0) {
            reprompib_print_error_and_exit( "Max message size in interval is null or not correctly specified");
          }
          break;
        case STEP:
          err = reprompib_str_to_long(value, &step);
          if (err || step <= 0) {
            reprompib_print_error_and_exit( "Message size step in interval is null or not correctly specified");
          }
          break;
        default:
            /* Unknown suboption. */
            reprompib_print_error_and_exit("Unknown option");
            break;
        }
    }

    if ((ssize_t)max - (ssize_t)min >= 0) /* the msize interval option has valid values  */
    {
        opts_p->n_msize = (max - min + step) / step;
        opts_p->msize_list = (size_t *) malloc(opts_p->n_msize * sizeof(size_t));
        for (i = min, index = 0; i <= max; i += step, index++) {
            opts_p->msize_list[index] = 1 << i;
        }

    } else {
        reprompib_print_error_and_exit("Invalid message sizes interval - min is larger than max (--msize-interval min=<min>,max=<max>,step=<step>)");
    }

}

static void parse_msize_list(char* msizes, reprompib_common_options_t* opts_p) {
    char* msizes_tok;
    char* save_str;
    char* s;
    int index = 0;

    save_str = (char*) malloc(STRING_SIZE * sizeof(char));
    s = save_str;

    /* Parse the list of message sizes */
    if (msizes != NULL) {
        index = 0;
        opts_p->msize_list = (size_t*) malloc(LEN_MSIZES_BATCH * sizeof(size_t));

        msizes_tok = strtok_r(msizes, ",", &save_str);
        while (msizes_tok != NULL) {

            long msize;
            int err;

            err = reprompib_str_to_long(msizes_tok, &msize);
            if (err || msize <=0) {
                reprompib_print_error_and_exit("Invalid list of message sizes (--msizes-list=<list of comma-separated positive integers>)");
            }

            opts_p->msize_list[index++] = msize;
            msizes_tok = strtok_r(NULL, ",", &save_str);

            if (index % LEN_MSIZES_BATCH == 0) {
                opts_p->msize_list = (size_t*) realloc(opts_p->msize_list,
                        (index + LEN_MSIZES_BATCH) * sizeof(size_t));
            }

        }
        opts_p->n_msize = index;

        // make sure we have valid message sizes to test
        if (index > 0) {
            opts_p->msize_list = (size_t*) realloc(opts_p->msize_list,
                    index * sizeof(size_t));
        } else {
            reprompib_print_error_and_exit("List of message sizes is empty (--msizes-list=<list of comma-separated positive integers>)");
        }
    }

    free(s);
}

static void parse_call_list(char* subopts, reprompib_common_options_t* opts_p) {
    char * value;
    int mpicall_index, i;

    opts_p->list_mpi_calls = (int*) calloc(LEN_MPICALLS_BATCH, sizeof(int));
    opts_p->n_calls = 0;

    i = 0;
    while (*subopts != '\0') {
        mpicall_index = getsubopt(&subopts, get_mpi_calls_list(), &value);  // returns -1 on error

        if (mpicall_index >=0 && mpicall_index < N_MPI_CALLS) { // valid mpi call
          opts_p->list_mpi_calls[i++] = mpicall_index;
          opts_p->n_calls++;

          if (i % LEN_MPICALLS_BATCH == 0) {
            opts_p->list_mpi_calls = (int*) realloc(opts_p->list_mpi_calls, (i + LEN_MPICALLS_BATCH) * sizeof(int));
          }
        }
        else {
            reprompib_print_error_and_exit("List of MPI calls is incorrect (--calls-list=<list of comma-separated MPI calls>)");
        }
    }

    if (opts_p->n_calls > 0) {
      opts_p->list_mpi_calls = (int*) realloc(opts_p->list_mpi_calls, opts_p->n_calls * sizeof(int));

    } else {
        free(opts_p->list_mpi_calls);
        opts_p->list_mpi_calls = NULL;
        reprompib_print_error_and_exit("List of MPI calls is empty (--calls-list=<list of comma-separated MPI calls>)");
    }

}

static void parse_operation(char* arg, reprompib_common_options_t* opts_p) {
    if (arg != NULL && strlen(arg) > 0) {
        if (strcmp("MPI_BOR", arg) == 0) {
            opts_p->operation = MPI_BOR;
        }
        else if (strcmp("MPI_BAND", arg) == 0) {
            opts_p->operation = MPI_BAND;
        }
        else if (strcmp("MPI_LOR", arg) == 0) {
            opts_p->operation = MPI_LOR;
        }
        else if (strcmp("MPI_LAND", arg) == 0) {
            opts_p->operation = MPI_LAND;
        }
        else if (strcmp("MPI_MAX", arg) == 0) {
            opts_p->operation = MPI_MAX;
        }
        else if (strcmp("MPI_MIN", arg) == 0) {
            opts_p->operation = MPI_MIN;
        }
        else if (strcmp("MPI_SUM", arg) == 0) {
            opts_p->operation = MPI_SUM;
        }
        else if (strcmp("MPI_PROD", arg) == 0) {
            opts_p->operation = MPI_PROD;
        }
        else {
          reprompib_print_error_and_exit("Unknown MPI operation");
        }
    }
    else {
      reprompib_print_error_and_exit("Invalid MPI operation");
    }
}


static void parse_datatype(char* arg, reprompib_common_options_t* opts_p) {
    if (arg != NULL && strlen(arg) > 0) {
        if (strcmp("MPI_BYTE", arg) == 0) {
          opts_p->datatype = MPI_BYTE;
        }
        else if (strcmp("MPI_CHAR", arg) == 0) {
          opts_p->datatype = MPI_CHAR;
        }
        else if (strcmp("MPI_INT", arg) == 0) {
          opts_p->datatype = MPI_INT;
        }
        else if (strcmp("MPI_FLOAT", arg) == 0) {
          opts_p->datatype = MPI_FLOAT;
        }
        else if (strcmp("MPI_DOUBLE", arg) == 0) {
          opts_p->datatype = MPI_DOUBLE;
        }
        else {
          reprompib_print_error_and_exit("Unknown MPI datatype");
        }
    }
    else {
      reprompib_print_error_and_exit("Invalid MPI operation");
    }
}


static void parse_pingpong_ranks(char* optarg, reprompib_common_options_t* opts_p) {
    char* ranks_tok;
    char* save_str;
    char* s;
    int index;
    int rank;
    char *endptr;

    save_str = (char*) malloc(STRING_SIZE * sizeof(char));
    s = save_str;

    /* Parse the list of ping-pong ranks */
    ranks_tok = strtok_r(optarg, ",", &save_str);
    index = 0;
    while (ranks_tok != NULL) {

      errno = 0;    /* To distinguish success/failure after strtol */
      rank = strtol(ranks_tok, &endptr, 10);

      if (errno != 0 || endptr == ranks_tok)  {
        reprompib_print_error_and_exit("Invalid rank specified for the ping-pong operation");
      }
      if (index == 0)
      {
          if (rank < 0 || rank >= icmb_initiator_size())
          {
              reprompib_print_error_and_exit("Invalid rank specified for the ping-pong operation");
          }
      }
      else if (index == 1)
      {
          if (rank < 0 || rank >= icmb_responder_size())
          {
              reprompib_print_error_and_exit("Invalid rank specified for the ping-pong operation");
          }
      }
      else if (index >= 2)
      {
          // cannot have more than two pingpong ranks
          reprompib_print_error_and_exit("Cannot have more than two ping-pong ranks");
      }
      else
      {
        opts_p->pingpong_ranks[index++] = rank;
        ranks_tok = strtok_r(NULL, ",", &save_str);
      }
    }

    free(s);

    if (index != 2) { // we need two ranks for the pingpong
      reprompib_print_error_and_exit("Invalid ping-pong ranks (only two different positive integers that are smaller than the total number of processes are accepted)");
    }

    if (opts_p->pingpong_ranks[0] == opts_p->pingpong_ranks[1]) { // the pingpong ranks should be different
      reprompib_print_error_and_exit("Invalid ping-pong ranks (only two different positive integers that are smaller than the total number of processes are accepted)");
    }
}

void reprompib_parse_common_options(reprompib_common_options_t* opts_p, int argc, char **argv) {
    int c;

    init_parameters(opts_p);

    opterr = 0;
    while (1) {
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, reprompi_common_opts_str, reprompi_common_long_options,
                &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;
        switch (c) {

        case REPROMPI_ARGS_INPUT_FILE: /* input file */
            opts_p->input_file = (char*)malloc((strlen(optarg)+1) * sizeof(char));
            strcpy(opts_p->input_file, optarg);
            break;

        case REPROMPI_ARGS_CALLS_LIST: /* list of calls */
            parse_call_list(optarg, opts_p);
            break;

        case REPROMPI_ARGS_MSIZES_LIST: /* list of message sizes */
            parse_msize_list(optarg, opts_p);
            break;

        case REPROMPI_ARGS_MSIZES_INTERVAL:
            /* Interval of power of 2 message sizes */
            parse_msize_interval(optarg, opts_p);
            break;
        case REPROMPI_ARGS_ROOT_PROC: /* set root node for collective function */
            opts_p->root_proc = atoi(optarg);
            break;
        case REPROMPI_ARGS_OPERATION: /* set operation for collective function */
            parse_operation(optarg, opts_p);
            break;
        case REPROMPI_ARGS_DATATYPE: /* set operation for collective function */
            parse_datatype(optarg, opts_p);
            break;
        case REPROMPI_ARGS_SHUFFLE_JOBS: /* enable job shuffling */
            opts_p->enable_job_shuffling = 1;
            break;
        case REPROMPI_ARGS_OUTPUT_FILE: /* path to an output file */
            opts_p->output_file = (char*)malloc((strlen(optarg)+1) * sizeof(char));
            strcpy(opts_p->output_file, optarg);
            break;
        case REPROMPI_ARGS_PINGPONG_RANKS: /* set the ranks between which to run the ping-pong operations*/
            parse_pingpong_ranks(optarg, opts_p);
            break;
        case '?':
            break;
        }
    }

    // check for errors
    if (opts_p->root_proc < 0 || opts_p->root_proc >= icmb_initiator_size()) {
      reprompib_print_error_and_exit("Invalid root process (should be >= 0 and smaller than the total number of processes)");
    }

    if (opts_p->input_file == NULL) {
      if (opts_p->n_msize <= 0) {
        reprompib_print_error_and_exit("List of message sizes is empty (--msizes-list=<list of comma-separated positive integers>)");
      }
      if (opts_p->n_calls <= 0) {
        reprompib_print_error_and_exit("List of MPI calls is empty (--calls-list=<list of comma-separated MPI calls>)");
      }
    }

    if (opts_p->output_file != NULL) {
        long output_file_error = 0;
        if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC)) {
            FILE *f;
            f = fopen(opts_p->output_file, "w");
            if (f != NULL) {
                fclose (f);
            }
            else {
              output_file_error = 1;
            }
        }

        MPI_Bcast(&output_file_error, 1, MPI_LONG, icmb_lookup_global_rank(OUTPUT_ROOT_PROC), icmb_global_communicator());
        if (output_file_error != 0) {
            opts_p->output_file = NULL;
            reprompib_print_error_and_exit("Cannot open output file");
        }
    }

    optind = 1;	// reset optind to enable option re-parsing
    opterr = 1;	// reset opterr to catch invalid options
}




