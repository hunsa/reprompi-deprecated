/* Inter-Communicator MPI Benchmarking
 * Copyright (c) 2021 Stefan Christians
 * SPDX-License-Identifier: MIT
 */

// allow strdup with c99
#define _POSIX_C_SOURCE 200809L

#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "attributes.h"
#include "error_output.h"
#include "launchers.h"

enum {
	 STARTUP_ARGS_HELP = 200,
	 STARTUP_ARGS_SPLIT,
	 STARTUP_ARGS_SPAWN,
     STARTUP_ARGS_CONNECT,
};

static const struct option startup_long_options[] = {
        { "help", no_argument, 0, STARTUP_ARGS_HELP },
        { "split", optional_argument, 0, STARTUP_ARGS_SPLIT },
        { "spawn", required_argument, 0, STARTUP_ARGS_SPAWN },
        { "connect", required_argument, 0, STARTUP_ARGS_CONNECT },
        { NULL, 0, NULL, 0 }
};

static const char startup_short_options[] = "h";

/*
 * display help message for startup options
 */
void icmb_print_intercommunication_help() {
    printf("\nIntercommunication startup options:\n");
    printf("(startup options are mutually exclusive)\n");
    printf("%-25s %-.54s\n", "--split[=<numprocs>]", "splits processes into two groups");
    printf("%-29s %-.50s\n", "", "the highest <numprocs> ranks are moved to the new");
    printf("%-29s %-.50s\n", "", "group");
    printf("%-29s %-.50s\n", "", "if <numprocs> is not given, processes are split");
    printf("%-29s %-.50s\n", "", "by even and odd ranks");
    printf("%-25s %-.54s\n", "--spawn=<numprocs>", "starts as master,");
    printf("%-29s %-.50s\n", "", "spawns <numprocs> worker processes");
    printf("%-25s %-.54s\n", "--connect=<numprocs>", "starts as client,");
    printf("%-29s %-.50s\n", "", "connects to <numprocs> server processes");
}

void icmb_parse_intercommunication_options (int argc, char** argv)
{
    // we expect to be started after MPI_Init was called
    int is_initialized;
    MPI_Initialized(&is_initialized);
    assert(is_initialized);

    int doSplit = 0;
    int siblings = 0;
    int workers = 0;
    int servers = 0;
    int callHelp = 0;

	int c;
    opterr = 0;

    // prepare command line arguments to be passed to spawned processes
    // (first element (command) is omitted, but a last element of NULL
    // must be added, so total maximum size is argc)
    char** spawn_argv = (char **) malloc((argc) * sizeof(char*));
    int spawn_argc = 0;

	while(1) {
		c = getopt_long(argc, argv, startup_short_options, startup_long_options, NULL);
		if(c == -1) {
			break;
		}

		switch(c) {

            case STARTUP_ARGS_HELP:
                callHelp = 1;
                break;

            case STARTUP_ARGS_SPLIT:
				doSplit = 1;
                // read optional argument seprated either by '=' or whitespace
                const char *tmp_optarg = optarg;
                if(!optarg && NULL != argv[optind] && '-' != argv[optind][0]) {
                    tmp_optarg = argv[optind++];
                }
				if(tmp_optarg) {
					siblings = atoi(tmp_optarg);
				}
				break;

            case STARTUP_ARGS_SPAWN:
                workers = atoi(optarg);
                break;

            case STARTUP_ARGS_CONNECT:
                servers = atoi(optarg);
                break;

            default:
                spawn_argv[spawn_argc++] = strdup(argv[optind-1]);
                break;
		}
	}

	// add non-option arguments to spawn options
    for (; optind < argc; optind++){
        spawn_argv[spawn_argc++] = strdup(argv[optind]);
    }

    optind = 1; // reset optind to enable option re-parsing
    opterr = 1; // reset opterr to catch invalid options

    // just pass through if help was called
    // (make sure benchmark help calls icmb_print_intercommunication_help later on)
    if (!callHelp)
    {
        // add terminating NULL-element to spawning arguments
        spawn_argv[spawn_argc++] = NULL;

        // sanity checks for startup options
        if ((workers && servers) || (servers && doSplit) || (workers && doSplit))
        {
            icmb_error_and_exit("Conflicting arguments: choose one of split, spawn, or connect.", ICMB_ERROR_CONFLICTING_ARGUMENTS);
        }

        int size;
        MPI_Comm_size(MPI_COMM_WORLD, &size);
        if(doSplit && (siblings >= size)){
            icmb_warning("Reducing requested size of responder group (%d) to maximum number of available processes (%d)", siblings, size-1);
            siblings = (size-1);
        }

        // construct communicators and launch processes as needed
        if (doSplit)
        {
            icmb_launch_split(siblings);
        }
        else if (workers)
        {
            icmb_launch_master(workers, argv[0], spawn_argv);
        }
        else if (servers)
        {
            icmb_launch_client(servers, argv[0], spawn_argv);
        }
        else
        {
            // try to launch a spawned process (worker or server)
            icmb_launch_spawned();
        }

        // fall back on standard intra-communicator benchmark
        if (MPI_COMM_NULL == icmb_get_benchmarkcommunicator_attribute())
        {
            icmb_launch_standard();
        }

    } /* if (!callHelp) */

    // free spawning arguments
    for (int i=0; i < spawn_argc; ++i)
    {
        free(spawn_argv[i]);
    }
    free(spawn_argv);
}
