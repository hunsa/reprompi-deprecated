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
	 STARTUP_ARGS_CLIENT = 200,
	 STARTUP_ARGS_PORT,
	 STARTUP_ARGS_SERVER,
	 STARTUP_ARGS_SPLIT,
	 STARTUP_ARGS_WORKERS,
	 STARTUP_ARGS_HELP,
};

static const struct option startup_long_options[] = {
        { "client", no_argument, 0, STARTUP_ARGS_CLIENT },
        { "port", required_argument, 0, STARTUP_ARGS_PORT },
        { "server", no_argument, 0, STARTUP_ARGS_SERVER },
        { "split", optional_argument, 0, STARTUP_ARGS_SPLIT },
        { "workers", required_argument, 0, STARTUP_ARGS_WORKERS },
        { "help", no_argument, 0, STARTUP_ARGS_HELP },
        { NULL, 0, NULL, 0 }
};

static const char startup_short_options[] = "h";

/*
 * display help message for startup options
 */
void icmb_print_intercommunication_help() {
    printf("\nIntercommunication startup options:\n");
    printf("(startup options are mutually exclusive)\n");
    printf("%-25s %-.54s\n", "--server", "starts as server");
    printf("%-25s %-.54s\n", "--client [--port='...']", "starts as client");
    printf("%-29s %-.50s\n", "", "if no nameserver is running, the server's port");
    printf("%-29s %-.50s\n", "", "name must be entered manually");
    printf("%-25s %-.54s\n", "--workers=<numprocs>", "spawns <numprocs> worker processes");
    printf("%-25s %-.54s\n", "--split[=<numprocs>]", "splits processes into two groups");
    printf("%-29s %-.50s\n", "", "the highest <numprocs> ranks are moved to the new");
    printf("%-29s %-.50s\n", "", "group");
    printf("%-29s %-.50s\n", "", "if <numprocs> is not given, processes are split by");
    printf("%-29s %-.50s\n", "", "even and odd ranks");
}

void icmb_parse_intercommunication_options (int argc, char** argv)
{
    // we expect to be started after MPI_Init was called
    int is_initialized;
    MPI_Initialized(&is_initialized);
    assert(is_initialized);

    char port_name[MPI_MAX_PORT_NAME] = "";
    int isClient = 0;
    int isServer = 0;
    int doSplit = 0;
    int workers = 0;
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

			case STARTUP_ARGS_CLIENT:
				isClient = 1;
				break;

			case STARTUP_ARGS_PORT:
				strcpy(port_name, optarg);
				break;

			case STARTUP_ARGS_SERVER:
				isServer = 1;
				break;

			case STARTUP_ARGS_SPLIT:
				doSplit = 1;
                // read optional argument seprated either by '=' or whitespace
                const char *tmp_optarg = optarg;
                if(!optarg && NULL != argv[optind] && '-' != argv[optind][0]) {
                    tmp_optarg = argv[optind++];
                }
				if(tmp_optarg) {
					workers = atoi(tmp_optarg);
				}
				break;

            case STARTUP_ARGS_WORKERS:
                workers = atoi(optarg);
                break;

            case STARTUP_ARGS_HELP:
                callHelp = 1;
                break;

            default:
                spawn_argv[spawn_argc++] = strdup(argv[optind-1]);
                break;
		}
	}

	// add non-option argumets to spawn options
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
        if(isClient && isServer) {
            icmb_error_and_exit("Can not be client and server at the same time", ICMB_ERROR_SAME_CLIENT_SERVER);
        }

        if((isClient || isServer) && doSplit) {
            icmb_error_and_exit("Can not split from client or server", ICMB_ERROR_SPLIT_FROM_CLIENT_SERVER);
        }

        if((isClient || isServer) && workers) {
            icmb_error_and_exit("Can not spawn from client or server", ICMB_ERROR_SPAWN_FROM_CLIENT_SERVER);
        }

        int size;
        MPI_Comm_size(MPI_COMM_WORLD, &size);
        if(doSplit && (workers >= size)){
            icmb_warning("Reducing requested size of responder group (%d) to maximum number of available processes (%d)", workers, size-1);
            workers = (size-1);
        }

        // construct communicators and launch processes as needed
        if(isServer)
        {
            icmb_launch_server();
        }
        else if(isClient)
        {
            icmb_launch_client(port_name);
        }
        else if (doSplit)
        {
            icmb_launch_split(workers);
        }
        else if (workers)
        {
            // this is master process spawning workers
            icmb_launch_master(workers, argv[0], spawn_argv);
        }
        else{
            // this might be a worker process spawned by master
            icmb_launch_worker();
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
