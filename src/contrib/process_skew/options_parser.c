/* ReproMPI Process Skew Measurement
 *
 * Copyright (c) 2021 Stefan Christians
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
   This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

// allow strdup with c99
#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <getopt.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "contrib/intercommunication/intercommunication.h"

#include "options_parser.h"

static const int OUTPUT_ROOT_PROC = 0;

enum process_skew_getopt_ids
{
    SKEW_ARGS_HELP = 'h',
    SKEW_ARGS_OUTPUT = 'o',
    SKEW_ARGS_OUTPUT_FILE = 'f',
    SKEW_ARGS_WINDOW = 'w',
    SKEW_ARGS_MPI_BARRIER = 'm',
    SKEW_ARGS_DISSEMINATION_BARRIER = 'b',
    SKEW_ARGS_DOUBLE_BARRIER = 'd',
};

static const char skew_short_options[] = "ho:wmbd";

static const struct option skew_long_options[] = {
        { "help", no_argument, 0, SKEW_ARGS_HELP },
        { "output", required_argument, 0, SKEW_ARGS_OUTPUT },
        { "output-file", required_argument, 0, SKEW_ARGS_OUTPUT_FILE },
        { "window", no_argument, 0, SKEW_ARGS_WINDOW },
        { "barrier", no_argument, 0, SKEW_ARGS_MPI_BARRIER },
        { "bbarrier", no_argument, 0, SKEW_ARGS_DISSEMINATION_BARRIER },
        { "double", no_argument, 0, SKEW_ARGS_DOUBLE_BARRIER },
        { NULL, 0, NULL, 0 }
};

static void init_options(skew_options_t* opt)
{
    opt->use_window = 0;
    opt->use_mpi_barrier = 0;
    opt->use_dissemination_barrier = 0;
    opt->use_double_barrier = 0;
    opt->output_file = NULL;
}

static void print_process_skew_help(char* command) {
    if(icmb_has_initiator_rank(OUTPUT_ROOT_PROC))
    {
        printf("Usage: mpiexec -n <numprocs> %s [options]\n", command);

        printf("\nMeasures process skew after synchronizing processes.\n");
        printf("(Clocks are synchronized using JK-algorithm).\n");

        printf("\noptions for measuring process skew:\n");
        printf("%-25s %-.54s\n", "-h,--help", "prints this help message");
        printf("%-25s %-.54s\n", "-o,--output=<path>", "results file");
        printf("%-25s %-.54s\n", "-w,--window", "use window synchronization (JK)");
        printf("%-25s %-.54s\n", "-m,--barrier", "use MPI_Barrier synchronization");
        printf("%-25s %-.54s\n", "-b,--bbarrier", "use dissemination barrier synchronization");
        printf("%-25s %-.54s\n", "-d,--double", "use double barriers");

        printf("\noptions for benchmark execution:\n");
        printf("%-25s %-.54s\n", "--nrep=<nrep>", "set number of experiment repetitions");
        printf("%-25s %-.54s\n", "--summary=<args>", "list of comma-separated data summarizing methods");
        printf("%-29s %-.50s\n", "", "(mean, median, min, max)");

        printf("\noptions for window based synchronization:\n");
        printf("%-25s %-.54s\n", "--window-size=<win>", "window size in microseconds (default: 1 ms)");
        printf("%-25s %-.54s\n", "--wait-time=<wait>", "wait time in microseconds (default: 1 ms)");

        printf("\noptions for linear clock skew model:\n");
        printf("%-25s %-.54s\n", "--fitpoints=<nfit>", "number of fitpoints (default: 20)");
        printf("%-25s %-.54s\n", "--exchanges=<nexc>", "number of exchanges (default: 10)");

        icmb_print_intercommunication_help();
    }
}

void free_process_skew_options ( skew_options_t* opt )
{
    free(opt->output_file);
    opt->output_file = NULL;
}

void parse_process_skew_options(skew_options_t* opt, int argc, char **argv)
{
    // we expect to be started after MPI_Init was called
    int is_initialized;
    MPI_Initialized(&is_initialized);
    assert(is_initialized);

    // initialize options
    init_options(opt);

    // parse command line arguments
	int c;
    opterr = 0;
	while(1) {
		c = getopt_long(argc, argv, skew_short_options, skew_long_options, NULL);
		if(c == -1) {
			break;
		}

		switch(c) {

			case SKEW_ARGS_HELP:
                print_process_skew_help(argv[0]);
                icmb_exit(0);
				break;

			case SKEW_ARGS_OUTPUT:
			case SKEW_ARGS_OUTPUT_FILE:
                opt->output_file = (char*)malloc((strlen(optarg)+1) * sizeof(char));
                strcpy(opt->output_file, optarg);
				break;

            case SKEW_ARGS_WINDOW:
                opt->use_window = 1;
				break;

			case SKEW_ARGS_MPI_BARRIER:
                opt->use_mpi_barrier = 1;
				break;

            case SKEW_ARGS_DISSEMINATION_BARRIER:
                opt->use_dissemination_barrier = 1;
                break;

            case SKEW_ARGS_DOUBLE_BARRIER:
                opt->use_double_barrier = 1;
                break;

            default:
                break;
		}
	}

    optind = 1;	// reset optind to enable option re-parsing
    opterr = 1;	// reset opterr to catch invalid options

    // sanity checks
    if (opt->use_window)
    {
        opt->use_mpi_barrier = 0;
        opt ->use_dissemination_barrier = 0;
        opt->use_double_barrier = 0;
    }

    if (opt->use_dissemination_barrier)
    {
        opt->use_window = 0;
        opt->use_mpi_barrier = 0;
    }

    if (opt->use_mpi_barrier)
    {
        opt->use_window = 0;
        opt->use_dissemination_barrier = 0;
    }

    if (!opt->use_window && !opt->use_mpi_barrier && !opt->use_dissemination_barrier)
    {
        // default: MPI_Barrier
        opt->use_mpi_barrier = 1;
    }
}
