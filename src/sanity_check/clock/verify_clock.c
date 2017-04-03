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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "mpi.h"

#include "reprompi_bench/sync/time_measurement.h"

static const double MAX_TIMER_THRESHOLD_PERCENT = 1;

int main(int argc, char* argv[]) {
  int my_rank, nprocs, p;
  int master_rank = 0;

  double *all_runtimes = NULL;
  double *local_runtimes;
  struct timespec sleep_time;
  double sleep_time_s, start_time;
  ;

  int step;
  int nreps = 1;
  int print_all = 0;

  /* start up MPI */
  MPI_Init(&argc, &argv);

  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  if (argc < 2 || strcmp(argv[1], "-h") == 0 ) {
    if (my_rank == master_rank) {
      printf("\nUsage: mpirun -np 4 %s sleep_time_s nreps [print_times]\n", argv[0]);
      printf("\t - sleep_time_s - reference time to be measured in seconds\n");
      printf("\t - nreps - number of times to measure sleep_time_s on each process\n");
      printf("\t - print_times - print the time measured on each process [values: 1/0]\n");
    }
    MPI_Finalize();
    exit(0);
  }

  sleep_time_s = atof(argv[1]);
  if (sleep_time_s <= 0) {
    if (my_rank == master_rank) {
      printf("ERROR: sleep_time_s should be a positive number\n");
    }
    MPI_Finalize();
    exit(0);
  }

  if (argc >= 3) {
    nreps = atoi(argv[2]);
  } else {
    nreps = -1;
  }
  if (nreps <= 0) {
    if (my_rank == master_rank) {
      printf("ERROR: nreps should be a positive integer\n");
    }
    MPI_Finalize();
    exit(0);
  }

  if (argc == 4) {
    print_all = atoi(argv[3]);
  }
  if (print_all != 0 && print_all != 1) {
    if (my_rank == master_rank) {
      printf("ERROR: print_times can take one of these values: 0 - do not print, 1 - print all times\n");
    }
    MPI_Finalize();
    exit(0);
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
  MPI_Gather(local_runtimes, nreps, MPI_DOUBLE, all_runtimes, nreps, MPI_DOUBLE, 0, MPI_COMM_WORLD);

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
