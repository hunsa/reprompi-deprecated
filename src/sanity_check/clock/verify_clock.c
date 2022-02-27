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

// allow nanosleep with c99
#define _POSIX_C_SOURCE 200809L


#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "mpi.h"

#include "reprompi_bench/sync/time_measurement.h"

#include "contrib/intercommunication/intercommunication.h"

static const double MAX_TIMER_THRESHOLD_PERCENT = 1;

static const int OUTPUT_ROOT_PROC = 0;

static const struct option verify_clock_long_options[] = {
        { "help", no_argument, 0, 'h' },
        { "sleep", required_argument, 0, 's' },
        { "nreps", required_argument, 0, 'n' },
        { "print", no_argument, 0, 'p' },
        { NULL, 0, NULL, 0 }
};

static const char verify_clock_short_options[] = "s:n:hp";

static void print_help (char* command)
{
    if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC))
    {
        printf("Usage: mpiexec -n <numprocs> %s [options]\n", command);

        printf("\nverify clock options:\n");
        printf("%-25s %-.54s\n", "--sleep=<s>", "reference time to be meaasured in seconds");
        printf("%-25s %-.54s\n", "--nreps=<s>", "number of times to measure sleep time on each process");
        printf("%-25s %-.54s\n", "--print", "whether to print the time measured on each process (flag)");

        icmb_print_intercommunication_help();
    }
}

static void parse_verify_clock_options (int argc, char** argv, double* sleep_time_s, int* nreps, int* print_all)
{
    // we expect to be started after MPI_Init was called
    int is_initialized;
    MPI_Initialized(&is_initialized);
    assert(is_initialized);

    int c;
    opterr = 0;

	while(1) {
		c = getopt_long(argc, argv, verify_clock_short_options, verify_clock_long_options, NULL);
		if(c == -1) {
			break;
		}

		switch(c) {

            case 'h':
                print_help(argv[0]);
                icmb_exit(0);
                break;

            case 's':
                *sleep_time_s = atof(optarg);
				break;

            case 'n':
                *nreps = atoi(optarg);
				break;

            case 'p':
                *print_all = 1;
				break;
		}
	}

	// in case sleep_time_s was passed as none-option argument
    if (optind < argc)
    {
        *sleep_time_s = atof(argv[optind++]);
    }

   	// in case nreps was passed as none-option argument
    if (optind < argc)
    {
        *nreps = atoi(argv[optind++]);
    }

   	// in case print was passed as none-option argument
    if (optind < argc)
    {
        *print_all = atoi(argv[optind++]);
    }

    optind = 1; // reset optind to enable option re-parsing
    opterr = 1; // reset opterr to catch invalid options
}


int main(int argc, char* argv[]) {
  int my_rank, nprocs, p;
  int master_rank = 0;

  double *all_runtimes = NULL;
  double *local_runtimes;
  struct timespec sleep_time;
  double sleep_time_s = -1.0;
  double start_time;

  int step;
  int nreps = 1;
  int print_all = 0;

  /* start up MPI */
  MPI_Init(&argc, &argv);

  // parse command line options to launch inter-communicators
  icmb_parse_intercommunication_options(argc, argv);

  parse_verify_clock_options(argc, argv, &sleep_time_s, &nreps, &print_all);

  my_rank = icmb_global_rank();
  nprocs = icmb_global_size();

  if (sleep_time_s <= 0) {
      icmb_error_and_exit("sleep_time_s should be a positive number", ICMB_ERROR_WRONG_ARGUMENT);
  }

  if (nreps <= 0) {
      icmb_error_and_exit("nreps should be a positive integer", ICMB_ERROR_WRONG_ARGUMENT);
  }

  if (print_all != 0 && print_all != 1) {
      icmb_error_and_exit("print_times can take one of these values: 0 - do not print, 1 - print all times", ICMB_ERROR_WRONG_ARGUMENT);
  }

  // wait time
  sleep_time.tv_sec = (int) sleep_time_s;
  sleep_time.tv_nsec = (sleep_time_s - sleep_time.tv_sec) * 1e9;

  local_runtimes = (double*) calloc(nreps, sizeof(double));
  if (my_rank == master_rank) {
    all_runtimes = (double*) calloc(nprocs * nreps, sizeof(double));
  }

  for (step = 0; step < nreps; step++) {
    start_time = get_time();
    nanosleep(&sleep_time, NULL);
    local_runtimes[step] = get_time() - start_time;
  }

  if (print_all && my_rank == master_rank) {
    printf("%5s %10s %12s\n", "proc", "step", "meas_time_s");
  }

  // gather measurement results
  MPI_Gather(local_runtimes, nreps, MPI_DOUBLE, all_runtimes, nreps, MPI_DOUBLE, 0, icmb_global_communicator());

  if (my_rank == master_rank) {
    int passed = 1;

    if (print_all) {
      for (p = 0; p < nprocs; p++) {
        for (step = 0; step < nreps; step++) {
          printf("%5d %10d %4.10f\n", p, step, all_runtimes[p * nreps + step]);
        }
      }
    }
    for (p = 0; p < nprocs; p++) {
      for (step = 0; step < nreps; step++) {
        double error = fabs(1 - all_runtimes[p * nreps + step] / sleep_time_s) * 100;
        if (error > MAX_TIMER_THRESHOLD_PERCENT) {
          fprintf(stderr,
              "WARNING: clock rel. error for process %d is %.4f%% (ref_time_s=%4.10f, measured_time_s=%4.10f)\n", p, error,
              sleep_time_s, all_runtimes[p * nreps + step]);
          passed = 0;
          break;
        }
      }
    }
    if (passed) {
      printf("# Clock verification test ... Passed\n");
    } else {
      printf("# Clock verification test ... FAILED\n");
    }

  }

  free(local_runtimes);
  if (my_rank == master_rank) {
    free(all_runtimes);
  }

  MPI_Finalize();

  return 0;
}
