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
#include "collective_ops/collectives.h"
#include "prediction_methods/prediction_data.h"
#include "nrep_estimation.h"
#include "reprompi_bench/option_parser/option_parser_helpers.h"
#include "reprompi_bench/option_parser/option_parser_constants.h"
#include "reprompi_bench/option_parser/parse_common_options.h"
#include "parse_options.h"

static const double DEFAULT_THRES = 0.02;
static const int DEFAULT_WIN = 10;

enum {
    MIN = 0, MAX, STEP
} pred_steps;

static char * const pred_interval_opts[] = {
        [MIN] = "min",
        [MAX] = "max",
        [STEP] = "step",
        NULL
};

static void init_parameters(pred_options_t* opts_p) {
    int i;

    opts_p->prediction_params.n_rep_min = 0;
    opts_p->prediction_params.n_rep_max = 0;
    opts_p->prediction_params.n_rep_stride = 0;

    opts_p->prediction_params.n_methods = 1;
    opts_p->prediction_params.info[0].method = RSE;
    opts_p->prediction_params.info[0].method_thres = DEFAULT_THRES;
    opts_p->prediction_params.info[0].method_win = DEFAULT_WIN;

    for (i=1; i< N_PRED_METHODS; i++) {
        opts_p->prediction_params.info[i].method_thres = -1;
        opts_p->prediction_params.info[i].method_win = -1;
    }

}

void reprompib_free_parameters(pred_options_t* opts_p) {
    reprompib_free_common_parameters(&opts_p->options);
}

reprompib_error_t parse_nreps_interval(char* subopts, pred_options_t* opts_p) {
    reprompib_error_t ok = SUCCESS;
    char * value;

    while (*subopts != '\0') {
        switch (getsubopt(&subopts, pred_interval_opts, &value)) {
        case MIN:
            if (value == NULL) {
                ok |= ERROR_NREPS_MIN;
                break;
            }
            opts_p->prediction_params.n_rep_min = atol(value);
            break;
        case MAX:
            if (value == NULL) {
                ok |= ERROR_NREPS_MAX;
                break;
            }
            opts_p->prediction_params.n_rep_max = atol(value);
            break;
        case STEP:
            if (value == NULL) {
                ok |= ERROR_NREPS_STEP;
                break;
            }
            opts_p->prediction_params.n_rep_stride = atol(value);
            break;
        default:
            /* Unknown suboption. */
            ok |= ERROR_NREPS_INTERVAL_UNKNOWN;
            break;
        }
    }

    if (opts_p->prediction_params.n_rep_max - opts_p->prediction_params.n_rep_min < 0) /* the nreps interval option does not have valid values  */
    {
        ok |= ERROR_NREPS_INTERVAL_MINMAX;
    }

    return ok;
}


reprompib_error_t parse_prediction_methods(char* subopts, pred_options_t* opts_p) {
    reprompib_error_t ok = SUCCESS;
    char * value;
    int index;

    opts_p->prediction_params.n_methods = 0;
    while (*subopts != '\0') {
        index = getsubopt(&subopts, get_prediction_methods_list(), &value);
        if (index >= 0 && index < N_PRED_METHODS) {
            opts_p->prediction_params.info[opts_p->prediction_params.n_methods++].method = index;
        }
        else {
            ok |= ERROR_PRED_METHODS_LIST;
            break;
        }
        if (opts_p->prediction_params.n_methods > N_PRED_METHODS) {
            ok |= ERROR_PRED_METHODS_LIST;
            break;
        }
    }

    if (opts_p->prediction_params.n_methods <= 0) {
        ok |= ERROR_PRED_METHODS_LIST_EMPTY;
    }

    return ok;
}


reprompib_error_t parse_prediction_thresholds(char* subopts, pred_options_t* opts_p) {
    reprompib_error_t ok = SUCCESS;
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
                ok |= ERROR_THRES_LIST;
                thres_tok = strtok_r(NULL, ",", &save_str);
                break;
            }

            opts_p->prediction_params.info[index++].method_thres = thr;
            thres_tok = strtok_r(NULL, ",", &save_str);
        }

        // make sure we have valid thresholds to test
        if (index == 0) {
            ok |= ERROR_THRES_LIST_EMPTY;
        }
    }

    free(s);
    return ok;
}


reprompib_error_t parse_prediction_windows(char* subopts, pred_options_t* opts_p) {
    reprompib_error_t ok = SUCCESS;
    char* win_tok;
    char* save_str;
    char* s;
    int index;
    const int THRES_STR_LEN = 32;

    save_str = (char*) malloc(THRES_STR_LEN * sizeof(char));
    s = save_str;

    /* Parse the list of message sizes */
    if (subopts != NULL) {
        index = 0;

        win_tok = strtok_r(subopts, ",", &save_str);
        while (win_tok != NULL) {
            int thr = atoi(win_tok);

            if (thr <= 0 || index >= N_PRED_METHODS) {
                ok |= ERROR_PRED_WIN_LIST;
                win_tok = strtok_r(NULL, ",", &save_str);
                break;
            }

            opts_p->prediction_params.info[index++].method_win = thr;
            win_tok = strtok_r(NULL, ",", &save_str);
        }

        // make sure we have valid thresholds to test
        if (index == 0) {
            ok |= ERROR_PRED_WIN_LIST_EMPTY;
        }
    }

    free(s);
    return ok;
}



reprompib_error_t reprompib_parse_options(pred_options_t* opts_p, int argc, char** argv, reprompib_dictionary_t* dict) {
    int c, i;
    reprompib_error_t ret = SUCCESS;
    int printhelp = 0;

    init_parameters(opts_p);

    ret |= reprompib_parse_common_options(&opts_p->options, argc, argv, dict);
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

        case REPROMPI_ARGS_NREPPRED_NREP_LIMITS:
            /* Settings for estimating the number of repetitions */
            ret |= parse_nreps_interval(optarg, opts_p);
            break;

        case REPROMPI_ARGS_NREPPRED_PRED_METHOD:
            /* List of methods estimating the number of repetitions */
            ret |= parse_prediction_methods(optarg, opts_p);
            break;

        case REPROMPI_ARGS_NREPPRED_VAR_THRES:
            /* List of method thresholds for estimating the number of repetitions */
            ret |= parse_prediction_thresholds(optarg, opts_p);
            break;

        case REPROMPI_ARGS_NREPPRED_VAR_WIN:
            /* List of method windows for estimating the number of repetitions */
            ret |= parse_prediction_windows(optarg, opts_p);
            break;

        case REPROMPI_ARGS_HELP:
            reprompib_print_prediction_help();
            printhelp = 1;
            break;

        case '?':
            break;
        }
    }

    if (opts_p->options.input_file == NULL) {

        if (opts_p->prediction_params.n_rep_max <= 0) {
            ret |= ERROR_NREP_NULL;
        }
    }

    for (i=0; i< N_PRED_METHODS; i++) {
        if (i < opts_p->prediction_params.n_methods && opts_p->prediction_params.info[i].method_thres < 0) {
            ret |= ERROR_THRES_LIST;
        }
        if (i< opts_p->prediction_params.n_methods && opts_p->prediction_params.info[i].method_win < 0) {
            ret |= ERROR_PRED_WIN_LIST;
        }
        if (i >= opts_p->prediction_params.n_methods && opts_p->prediction_params.info[i].method_thres >= 0) {
            ret |= ERROR_THRES_LIST;
        }
        if (i >= opts_p->prediction_params.n_methods && opts_p->prediction_params.info[i].method_win >= 0) {
            ret |= ERROR_PRED_WIN_LIST;
        }
    }

    if (printhelp) {
        ret = SUCCESS;
    }


    optind = 1;	// reset optind to enable option re-parsing
    opterr = 1;	// reset opterr to catch invalid options

    return ret;
}




