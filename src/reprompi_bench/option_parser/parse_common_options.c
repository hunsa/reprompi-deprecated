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

// fix strdup warning
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <mpi.h>
#include "option_parser_helpers.h"
#include "option_parser_constants.h"
#include "collective_ops/collectives.h"
#include "reprompi_bench/utils/keyvalue_store.h"
#include "reprompi_bench/misc.h"
#include "parse_common_options.h"

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


static void init_parameters(reprompib_common_options_t* opts_p) {
    opts_p->verbose = 0;
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


static int reprompib_str_to_size_t(const char *str, size_t* result) {
  char *endptr;
  int error = 0;
  long res;

  errno = 0;
  res = strtol(str, &endptr, 10);

  /* Check for various possible errors */
  if ((errno == ERANGE && (res == LONG_MAX || res == LONG_MIN)) || (errno != 0 && res == 0)) {
    error = 1;
  }
  if (endptr == str) {  // no digits parsed
    error = 1;
  }
  if (res <= 0) {
    error = 1;
  }

  if (!error) {
    *result = (size_t)res;
  }
  else {
    *result = 0;
  }

  return error;
}

static reprompib_error_t parse_msize_interval(char* subopts, reprompib_common_options_t* opts_p) {
    reprompib_error_t ok = SUCCESS;
    char * value;
    size_t min = 0, max = 0, step = 1, i, index;
    int err;

    while (*subopts != '\0') {
        switch (getsubopt(&subopts, msize_interval_opts, &value)) {
        case MIN:
          if (value == NULL) {
            ok = ERROR_MSIZE_MIN;
            break;
          }
          err = reprompib_str_to_size_t(value, &min);
          if (err) {
            ok = ERROR_MSIZE_MIN;
          }
          break;
        case MAX:
          if (value == NULL) {
            ok = ERROR_MSIZE_MAX;
            break;
          }
          err = reprompib_str_to_size_t(value, &max);
          if (err) {
            ok = ERROR_MSIZE_MAX;
          }
          break;
        case STEP:
          if (value == NULL) {
            ok = ERROR_MSIZE_STEP;
            break;
          }
          err = reprompib_str_to_size_t(value, &step);
          if (err) {
            ok = ERROR_MSIZE_STEP;
          }
          break;
        default:
            /* Unknown suboption. */
            ok = ERROR_MSIZE_INTERVAL_UNKNOWN;
            break;
        }
    }

    if (ok != SUCCESS) {
      reprompib_print_error_and_exit("Invalid message sizes interval (--msize-interval min=<min>,max=<max>,step=<step>)");
    }

    if ((ssize_t)max - (ssize_t)min >= 0) /* the msize interval option has valid values  */
    {
        opts_p->n_msize = (max - min + step) / step;
        opts_p->msize_list = (size_t *) malloc(opts_p->n_msize * sizeof(size_t));
        for (i = min, index = 0; i <= max; i += step, index++) {
            opts_p->msize_list[index] = 1 << i;
        }

        /* return success */
        ok = SUCCESS;
    } else {
        ok = ERROR_MSIZE_INTERVAL_MINMAX;
        reprompib_print_error_and_exit("Invalid message sizes interval - min is larger than max (--msize-interval min=<min>,max=<max>,step=<step>)");
    }

    return ok;
}

static reprompib_error_t parse_msize_list(char* msizes, reprompib_common_options_t* opts_p) {
    reprompib_error_t ok = SUCCESS;
    char* msizes_tok;
    char* save_str;
    char* s;
    int index;

    save_str = (char*) malloc(STRING_SIZE * sizeof(char));
    s = save_str;

    /* Parse the list of message sizes */
    if (msizes != NULL) {
        index = 0;
        opts_p->msize_list = (size_t*) malloc(LEN_MSIZES_BATCH * sizeof(size_t));

        msizes_tok = strtok_r(msizes, ",", &save_str);
        while (msizes_tok != NULL) {

            size_t msize;
            int err;

            err = reprompib_str_to_size_t(msizes_tok, &msize);
            if (err) {
                ok = ERROR_MSIZE_LIST;
                break;
            }

            opts_p->msize_list[index++] = msize;
            msizes_tok = strtok_r(NULL, ",", &save_str);

            if (index % LEN_MSIZES_BATCH == 0) {
                opts_p->msize_list = (size_t*) realloc(opts_p->msize_list,
                        (index + LEN_MSIZES_BATCH) * sizeof(size_t));
            }

        }
        opts_p->n_msize = index;

        if (ok != SUCCESS) {
          reprompib_print_error_and_exit("Invalid list of message sizes (--msizes-list=<list of comma-separated positive integers>)");
        }

        // make sure we have valid message sizes to test
        if (index > 0) {
            opts_p->msize_list = (size_t*) realloc(opts_p->msize_list,
                    index * sizeof(size_t));
        } else {
            ok = ERROR_MSIZE_LIST_EMPTY;
            reprompib_print_error_and_exit("List of message sizes is empty (--msizes-list=<list of comma-separated positive integers>)");
        }
    }

    free(s);
    return ok;
}

static reprompib_error_t parse_call_list(char* subopts, reprompib_common_options_t* opts_p) {
    reprompib_error_t ok = SUCCESS;
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
            ok |= ERROR_MPI_CALL_LIST;
            break;
        }
    }

    if (opts_p->n_calls > 0) {
      opts_p->list_mpi_calls = (int*) realloc(opts_p->list_mpi_calls, opts_p->n_calls * sizeof(int));

    } else {
        free(opts_p->list_mpi_calls);
        opts_p->list_mpi_calls = NULL;
        ok |= ERROR_MPI_CALL_LIST_EMPTY;
    }

    return ok;
}

static reprompib_error_t parse_keyvalue_list(char* args, reprompib_common_options_t* opts_p, reprompib_dictionary_t* dict) {
    reprompib_error_t ok = SUCCESS;
    char* params_tok;
    char *save_str, *s, *keyvalue_list;
    char *kv_str, *kv_s;
    char* key;
    char* val;

    save_str = (char*) malloc(STRING_SIZE * sizeof(char));
    kv_str = (char*) malloc(STRING_SIZE * sizeof(char));
    s = save_str;
    kv_s = kv_str;


    /* Parse the list of message sizes */
    if (args != NULL) {

        keyvalue_list = strdup(args);
        params_tok = strtok_r(keyvalue_list, ",", &save_str);
        while (params_tok != NULL) {
            key = strtok_r(params_tok, ":", &kv_str);
            val = strtok_r(NULL, ":", &kv_str);

            if (key!=NULL && val!= NULL) {
                if (!reprompib_dict_has_key(dict, key)) {
                    ok |= reprompib_add_element_to_dict(dict, key, val);
                }
                else {
                  ok |= ERROR_KEY_VAL_PARAM;
                  break;
                }
            }
            else {
                ok |= ERROR_KEY_VAL_PARAM;
                break;
            }
            params_tok = strtok_r(NULL, ",", &save_str);
        }

        free(keyvalue_list);
    }

    free(s);
    free(kv_s);
    return ok;
}


static reprompib_error_t parse_operation(char* arg, reprompib_common_options_t* opts_p) {
    reprompib_error_t ok = SUCCESS;

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
            ok = ERROR_MPI_OP;
        }
    }
    else {
        ok = ERROR_MPI_OP;
    }
    return ok;
}


static reprompib_error_t parse_datatype(char* arg, reprompib_common_options_t* opts_p) {
    reprompib_error_t ok = SUCCESS;
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
          ok = ERROR_DATATYPE;
        }
    }
    else {
        ok = ERROR_DATATYPE;
    }

    return ok;
}


static reprompib_error_t parse_pingpong_ranks(char* optarg, reprompib_common_options_t* opts_p) {
    reprompib_error_t ok = SUCCESS;
    char* ranks_tok;
    char* save_str;
    char* s;
    int index;
    int rank;
    char *endptr;
    int nprocs, my_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    save_str = (char*) malloc(STRING_SIZE * sizeof(char));
    s = save_str;

    /* Parse the list of ping-pong ranks */
    ranks_tok = strtok_r(optarg, ",", &save_str);
    index = 0;
    while (ranks_tok != NULL) {

      errno = 0;    /* To distinguish success/failure after strtol */
      rank = strtol(ranks_tok, &endptr, 10);

      if (errno != 0 || endptr == ranks_tok)  {
        ok = ERROR_UNKNOWN_OPTION;
        break;
      }
      if (rank < 0 || rank >= nprocs) {
        ok = ERROR_UNKNOWN_OPTION;
        break;
      } else {

        if (index >= 2) { // cannot have more than two pingpong ranks
          ok = ERROR_UNKNOWN_OPTION;
          break;
        }
        opts_p->pingpong_ranks[index++] = rank;
        ranks_tok = strtok_r(NULL, ",", &save_str);
      }
    }

    free(s);

    if (index != 2) { // we need two ranks for the pingpong
      ok = ERROR_UNKNOWN_OPTION;
    }

    if (opts_p->pingpong_ranks[0] == opts_p->pingpong_ranks[1]) { // the pingpong ranks should be different
      ok = ERROR_UNKNOWN_OPTION;
    }

    if (ok != SUCCESS) {
      if (my_rank == OUTPUT_ROOT_PROC) {
        fprintf(stderr, "\nERROR: Invalid ping-pong ranks (only two different positive integers that are smaller than the total number of processes are accepted)\n");
      }
      MPI_Finalize();
      exit(0);
    }
    return ok;
}

reprompib_error_t reprompib_parse_common_options(reprompib_common_options_t* opts_p, int argc, char **argv, reprompib_dictionary_t* dict) {
    int c;
    reprompib_error_t ret = SUCCESS;
    int printhelp = 0;
    int nprocs, my_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    init_parameters(opts_p);

    opterr = 0;
    while (1) {
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, reprompi_default_opts_str, reprompi_default_long_options,
                &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c) {

        case REPROMPI_ARGS_VERBOSE: /* verbose flag */
            opts_p->verbose = 1;
            break;

        case REPROMPI_ARGS_INPUT_FILE: /* input file */
            opts_p->input_file = (char*)malloc((strlen(optarg)+1) * sizeof(char));
            strcpy(opts_p->input_file, optarg);
            break;

        case REPROMPI_ARGS_CALLS_LIST: /* list of calls */

            ret |= parse_call_list(optarg, opts_p);
            break;

        case REPROMPI_ARGS_MSIZES_LIST: /* list of message sizes */
            ret |= parse_msize_list(optarg, opts_p);
            break;

        case REPROMPI_ARGS_MSIZES_INTERVAL:
            /* Interval of power of 2 message sizes */
            ret |= parse_msize_interval(optarg, opts_p);
            break;
        case REPROMPI_ARGS_PARAMS: /* list of key-value parameters */
            ret |= parse_keyvalue_list(optarg, opts_p, dict);
            break;
        case REPROMPI_ARGS_ROOT_PROC: /* set root node for collective function */
            opts_p->root_proc = atoi(optarg);
            break;
        case REPROMPI_ARGS_OPERATION: /* set operation for collective function */
            ret |= parse_operation(optarg, opts_p);
            break;
        case REPROMPI_ARGS_DATATYPE: /* set operation for collective function */
            ret |= parse_datatype(optarg, opts_p);
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

    if (opts_p->input_file == NULL) {

        if (opts_p->n_calls <= 0) {
            ret |= ERROR_MPI_CALL_LIST_EMPTY;
        }

        if (opts_p->n_msize <= 0) {
            ret |= ERROR_MSIZE_LIST_EMPTY;
        }
    }

    if (opts_p->root_proc < 0 || opts_p->root_proc > nprocs - 1) {
        ret |= ERROR_ROOT_PROC;
    }

    if (opts_p->output_file != NULL) {
        long output_file_error = SUCCESS;
        if (my_rank == OUTPUT_ROOT_PROC) {
            FILE *f;
            f = fopen(opts_p->output_file, "w");
            if (f != NULL) {
                fclose (f);
            }
            else {
                output_file_error = ERROR_OUTPUT_FILE_UNAVAILABLE;
            }
        }

        MPI_Bcast(&output_file_error, 1, MPI_LONG, OUTPUT_ROOT_PROC, MPI_COMM_WORLD);

        if (output_file_error != SUCCESS) {
            opts_p->output_file = NULL;
            ret |= output_file_error;
        }
    }


    if (printhelp) {
        ret = SUCCESS;
    }

    optind = 1;	// reset optind to enable option re-parsing
    opterr = 1;	// reset opterr to catch invalid options
    return ret;
}

void reprompib_validate_common_options_or_abort(const reprompib_error_t errorcodes, const reprompib_common_options_t* opts_p, const print_help_t help_func) {
    reprompib_error_t e = errorcodes;
    int my_rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    if (my_rank == OUTPUT_ROOT_PROC) {
        reprompib_error_t error_id = 1ULL;

        if (e | 0ULL) {
            help_func();
        }
        while (e | 0ULL) {
            if (e & 1ULL) {
                printf(">>>>>>>>>>>>>>>> Error: %s\n", get_error_message(error_id));
            }
            e = e >> 1;
            error_id = error_id << 1;
        }

    }

    if (errorcodes != SUCCESS) {
        reprompib_free_common_parameters(opts_p);

        /* shut down MPI */
        MPI_Finalize();
        exit(0);
    }
}

