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
#include <assert.h>

#include "reprompi_bench/misc.h"
#include "reprompi_bench/sync/sync_info.h"
#include "jk_parse_options.h"

void jk_parse_options(int argc, char **argv, reprompib_sync_options_t* opts_p) {
    int c;

    reprompi_init_sync_parameters(opts_p);

    optind = 1;
    optopt = 0;
    opterr = 0; // ignore invalid options
    while (1) {

        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, reprompi_sync_opts_str, reprompi_sync_long_options,
                &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c) {
        case REPROMPI_ARGS_WINSYNC_WIN_SIZE: /* window size (in usec)*/
            opts_p->window_size_sec = atof(optarg) * 1e-6;
            break;

        case REPROMPI_ARGS_WINSYNC_NFITPOINTS: /* number of fit points for the linear model */
            opts_p->n_fitpoints = atoi(optarg);
            break;

        case REPROMPI_ARGS_WINSYNC_NEXCHANGES: /* number of exchanges for the linear model */
            opts_p->n_exchanges = atoi(optarg);
            break;

        case REPROMPI_ARGS_WINSYNC_WAITTIME: /* wait time before starting the first measurement  (in usec) */
            opts_p->wait_time_sec = atof(optarg) * 1e-6;
            break;

        case '?':
            break;
        }
    }

    // check for errors
    if (opts_p->window_size_sec <= 0) {
      reprompib_print_error_and_exit("Invalid window size (should be positive)");
    }
    if (opts_p->wait_time_sec <= 0) {
      reprompib_print_error_and_exit("Invalid wait time before the first window (should be positive)");
    }
    if (opts_p->n_fitpoints <= 0) {
      reprompib_print_error_and_exit("Invalid number of fitpoints (should be a positive integer)");
    }
    if (opts_p->n_exchanges <= 0) {
      reprompib_print_error_and_exit("Invalid number of ping-pong exchanges (should be a positive integer)");
    }

    optind = 1;	// reset optind to enable option re-parsing
    opterr = 1; // reset opterr
}

