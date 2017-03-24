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
#include "reprompi_bench/option_parser/option_parser_constants.h"
#include "reprompi_bench/option_parser/option_parser_helpers.h"
#include "sk_sync.h"
#include "sk_parse_options.h"

reprompib_error_t sk_parse_options(sk_options_t* opts_p, int argc, char **argv) {
    int c;
    reprompib_error_t ret = SUCCESS;

    optind = 1;
    optopt = 0;
    opterr = 0; // ignore invalid options
    while (1) {

        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, reprompi_default_opts_str, reprompi_default_long_options,
                &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c) {
        case REPROMPI_ARGS_WINSYNC_WIN_SIZE: /* window size */
            opts_p->window_size_sec = atof(optarg) * 1e-6;
            break;

        case REPROMPI_ARGS_WINSYNC_WAITTIME: /* wait time before starting the first measurement  (in usec) */
            opts_p->wait_time_sec = atof(optarg) * 1e-6;
            break;
        case '?':
            break;
        }
    }

    if (opts_p->window_size_sec <= 0) {
        ret |= ERROR_WIN;
    }

    optind = 1;	// reset optind to enable option re-parsing
    opterr = 1; // reset opterr
    return ret;
}
