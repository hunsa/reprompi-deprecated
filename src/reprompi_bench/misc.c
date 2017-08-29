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


#include "mpi.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <errno.h>

#include "misc.h"

static const int OUTPUT_ROOT_PROC = 0;


double repro_min(double a, double b) {
    if (a < b)
        return a;
    else
        return b;
}

double repro_max(double a, double b) {
    if (a > b)
        return a;
    else
        return b;
}


void shuffle(int *array, size_t n) {
    srand(time(NULL));

    if (n > 1) {
        size_t i;
        for (i = 0; i < n - 1; i++) {
            size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
            int t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}


int reprompib_str_to_long(const char *str, long* result) {
  char *endptr;
  int error = 0;
  long res;

  errno = 0;
  res = strtol(str, &endptr, 10);

  /* Check for various possible errors */
  if ((errno == ERANGE && (res == LONG_MAX || res == LONG_MIN)) || (errno != 0 && res == 0)) {
    error = 1;
  }
  if (endptr == str) {  // no digits parsed
    error = 1;
  }
  if (!error) {
    *result = res;
  }
  else {
    *result = 0;
  }

  return error;
}




void reprompib_print_error_and_exit(const char* error_str) {
  int my_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  if (my_rank == OUTPUT_ROOT_PROC) {
    fprintf(stderr, "\nERROR: %s\n\n", error_str);
  }
  MPI_Finalize();
  exit(1);
}


