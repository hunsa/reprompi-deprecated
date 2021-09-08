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

// fix strdup warning
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <mpi.h>
#include "reprompi_bench/utils/keyvalue_store.h"
#include "reprompi_bench/misc.h"
#include "parse_extra_key_value_options.h"

static const int STRING_SIZE = 256;


enum {
  REPROMPI_ARGS_PARAMS = 700
};


static const struct option reprompi_params_long_options[] = {
        {"params", optional_argument, 0, REPROMPI_ARGS_PARAMS},
        { 0, 0, 0, 0 }
};
static const char reprompi_params_opts_str[] = "";



static void parse_keyvalue_list(char* args, reprompib_dictionary_t* dict) {
    char* params_tok;
    char *save_str, *s, *keyvalue_list;
    char *kv_str, *kv_s;
    char* key;
    char* val;
    int ok;

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
                    ok = reprompib_add_element_to_dict(dict, key, val);
                    if (ok != 0) {
                      reprompib_print_error_and_exit("Cannot add parameter to dictionary");
                    }
                }
                else {
                  reprompib_print_error_and_exit("Parameter already exists");
                }
            }
            else {
              reprompib_print_error_and_exit("Key-value parameters invalid");
            }
            params_tok = strtok_r(NULL, ",", &save_str);
        }

        free(keyvalue_list);
    }

    free(s);
    free(kv_s);
}


void reprompib_parse_extra_key_value_options(reprompib_dictionary_t* dict, int argc, char **argv) {
    int c;

    opterr = 0;
    while (1) {
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, reprompi_params_opts_str, reprompi_params_long_options,
                &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;
        switch (c) {
        case REPROMPI_ARGS_PARAMS: /* list of key-value parameters */
            parse_keyvalue_list(optarg, dict);
            break;
        case '?':
            break;
        }
    }
    optind = 1;	// reset optind to enable option re-parsing
    opterr = 1;	// reset opterr to catch invalid options
}




