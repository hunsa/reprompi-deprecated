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
#include "option_parser_helpers.h"
#include "parse_options.h"

static const int OUTPUT_ROOT_PROC = 0;

static char* const summary_opts[] = {
        [PRINT_MEAN] = "mean",
        [PRINT_MEDIAN] = "median",
        [PRINT_MIN] = "min",
        [PRINT_MAX] = "max",
        NULL
};


typedef enum reprompi_common_getopt_ids {
  REPROMPI_ARGS_VERBOSE = 'v',
  REPROMPI_ARGS_NREPS = 500,
  REPROMPI_ARGS_SUMMARY
} reprompi_common_getopt_ids_t;

static const struct option reprompi_default_long_options[] = {
        {"verbose", optional_argument, 0, REPROMPI_ARGS_VERBOSE},
        { "nrep", required_argument, 0, REPROMPI_ARGS_NREPS },
        {"summary", optional_argument, 0, REPROMPI_ARGS_SUMMARY},

        { 0, 0, 0, 0 }
};
static const char reprompi_default_opts_str[] = "v";



char* const* get_summary_opts_list(void) {

    return &(summary_opts[0]);
}


static void init_parameters(reprompib_options_t* opts_p) {
    int i;
    opts_p->verbose = 0;
    opts_p->n_rep = 0;

    opts_p->n_print_summary_selected = 0;
    opts_p->print_summary_methods = (int*)malloc(N_SUMMARY_METHODS *sizeof(int));
    for (i=0; i < N_SUMMARY_METHODS; i++) {
        opts_p->print_summary_methods[i] = 0;
    }

}

void reprompib_free_parameters(reprompib_options_t* opts_p) {
    if (opts_p->print_summary_methods != NULL) {
        free(opts_p->print_summary_methods);
    }
}


static void parse_summary_list(char* subopts, reprompib_options_t* opts_p) {
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
              reprompib_print_error_and_exit("Invalid list of summary methods (--summary=<list of comma-separated methods> [min, max, mean, median])");
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
}

void reprompib_parse_options(reprompib_options_t* opts_p, int argc, char** argv) {
    int c, err;
    long nreps;

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

        case REPROMPI_ARGS_NREPS: /* total number of (correct) repetitions */
            err = reprompib_str_to_long(optarg, &nreps);
            if (err || nreps <= 0) {
              reprompib_print_error_and_exit("Nreps value is negative or not correctly specified");
            }
            opts_p->n_rep = nreps;
            break;

        case REPROMPI_ARGS_SUMMARY: /* list of summary options */
            parse_summary_list(optarg, opts_p);
            break;


        case REPROMPI_ARGS_VERBOSE: /* verbose flag */
            opts_p->verbose = 1;
            break;
        case '?':
            break;
        }
    }


    if (opts_p->n_rep <= 0) {
      reprompib_print_error_and_exit("Nreps value is negative or not correctly specified");
    }

    optind = 1;	// reset optind to enable option re-parsing
    opterr = 1;	// reset opterr to catch invalid options
}


void reprompib_print_benchmark_help(void) {
    int my_rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    if (my_rank == OUTPUT_ROOT_PROC) {
        printf("\nUSAGE: mpibenchmark [options]\n");
        printf("options:\n");
    }

    reprompib_print_common_help();

    if (my_rank == OUTPUT_ROOT_PROC) {
        printf("\nSpecific options for the benchmark execution:\n");
        printf("%-40s %-40s\n", "-r | --repetitions=<nrep>",
                "set number of experiment repetitions");
        printf("%-40s %-40s\n %50s%s\n", "--summary=<args>",
                "list of comma-separated data summarizing methods (mean, median, min, max)", "",
                "e.g., --summary=mean,max");

        printf("\nEXAMPLES: mpirun -np 4 ./bin/mpibenchmark --calls-list=MPI_Bcast --msizes-list=8,512,1024 --nrep=5 --summary=mean,max,min\n");
        printf("\n          mpirun -np 4 ./bin/mpibenchmark --calls-list=MPI_Bcast --msizes-list=8,512,1024 --nrep=5\n");
        printf("\n          mpirun -np 4 ./bin/mpibenchmark --window-size=100 --calls-list=MPI_Bcast --msize-interval=min=1,max=8,step=1 --nrep=5\n");
        printf("\n          mpirun -np 4 ./bin/mpibenchmark --window-size=100 --calls-list=MPI_Bcast --msizes-list=1024 --nrep=5 --fitpoints=10 --exchanges=20\n");
        printf("\n          mpirun -np 4 ./bin/mpibenchmark --window-size=100 --calls-list=MPI_Bcast --msizes-list=1024 --nrep=5 --params=p1:1,p2:aaa,p3:34\n");
        printf("\n          mpirun -np 4 ./bin/mpibenchmark --calls-list=Sendrecv --msizes-list=10 --pingpong-ranks=0,3 --nrep=5 --summary \n");

        printf("\n\n");
    }
}

