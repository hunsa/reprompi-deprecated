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
#include <getopt.h>
#include <string.h>
#include "mpi.h"

#include "reprompi_bench/misc.h"
#include "option_parser_constants.h"
#include "parse_common_options.h"
#include "parse_options.h"


static char* const summary_opts[] = {
        [PRINT_MEAN] = "mean",
        [PRINT_MEDIAN] = "median",
        [PRINT_MIN] = "min",
        [PRINT_MAX] = "max",
        NULL
};


char* const* get_summary_opts_list(void) {

    return &(summary_opts[0]);
}


static void init_parameters(reprompib_options_t* opts_p) {
    int i;

    opts_p->n_rep = 0;

    opts_p->n_print_summary_selected = 0;
    opts_p->print_summary_methods = (int*)malloc(N_SUMMARY_METHODS *sizeof(int));
    for (i=0; i < N_SUMMARY_METHODS; i++) {
        opts_p->print_summary_methods[i] = 0;
    }

}

void reprompib_free_parameters(reprompib_options_t* opts_p) {

    reprompib_free_common_parameters(&(opts_p->common_opt));
    if (opts_p->print_summary_methods != NULL) {
        free(opts_p->print_summary_methods);
    }
}


static reprompib_error_t parse_summary_list(char* subopts, reprompib_options_t* opts_p) {
    reprompib_error_t ok = SUCCESS;
    char * value;
    int index;

    if (subopts != NULL) {
        while (*subopts != '\0') {
            index = getsubopt(&subopts, summary_opts, &value);

            if (index >=0 && index < N_SUMMARY_METHODS) {
                opts_p->print_summary_methods[index] = 1;
                opts_p->n_print_summary_selected++;
            }
            else {
                ok |= ERROR_SUMMARY_METHOD;
            }
        }
    }
    if (opts_p->n_print_summary_selected <= 0) {
        int i;

        opts_p->n_print_summary_selected = N_SUMMARY_METHODS;
        for (i=0; i < N_SUMMARY_METHODS; i++) {
            opts_p->print_summary_methods[i] = 1;
        }
    }

    return ok;
}

reprompib_error_t reprompib_parse_options(reprompib_options_t* opts_p, int argc, char** argv, reprompib_dictionary_t* dict) {
    int c;
    reprompib_error_t ret = SUCCESS;
    int printhelp = 0;

    init_parameters(opts_p);
    ret |= reprompib_parse_common_options(&opts_p->common_opt, argc, argv, dict);
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

        case REPROMPI_ARGS_NREPS: /* total number of (correct) repetitions */
            opts_p->n_rep = atol(optarg);
            break;

        case REPROMPI_ARGS_SUMMARY: /* list of summary options */
            ret |= parse_summary_list(optarg, opts_p);
            break;

        case REPROMPI_ARGS_HELP:
            reprompib_print_benchmark_help();
            printhelp = 1;
            break;
        case '?':
            break;
        }
    }

    if (opts_p->common_opt.input_file == NULL) {

        if (opts_p->n_rep <= 0) {
            ret |= ERROR_NREP_NULL;
        }
    }


    if (printhelp) {
        ret = SUCCESS;
    }

    optind = 1;	// reset optind to enable option re-parsing
    opterr = 1;	// reset opterr to catch invalid options

    return ret;
}

