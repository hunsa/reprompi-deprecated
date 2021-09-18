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


#include "mpi.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <errno.h>

#include "contrib/intercommunication/intercommunication.h"

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
    icmb_error_and_exit(error_str, 1);
}


/*
 * returns the greatest common divisor of two integers
 *
 * Stein's binary algorithm 1961
 * has more iteriations than Euclid's, but is faster
 * because each iteration is simpler (no divisions)
 */
long gcd(long a, long b)
{
    // find power of 2
    int k = 0;
    while ( !(a&1) && !(b&1) ) // while a and b are both even
    {
        // halve a and b
        a = a>>1;
        b = b>>1;
        ++k;
    }

    // initialize
    // a and b have been divided by 2^k,
    // and at least one of them is odd
    long t;
    if ((a&1)) // if a is odd
    {
        t = -b;
        t = t<<1; // doble t again, just to have single entry point to loop
    }
    else
    {
        t = a;
    }

    // loop
    while (0 != t)
    {
        // at this point, t is even and nonzero

        // halve t
        t = t>>1;

        if (!(t&1)) // if t is even
        {
            continue;
        }

        // reset max(a,b)
        // (the larger of a and b is replaced by |t|)
        if (t>0)
        {
            a = t;
        }
        else{
            b = -t;
        }

        // subtract
        t = a - b;
    }

    // result is a * 2^k
    return a<<k;
}

/*
 * returns the least common multiple of two integers
 */
long lcm(long a,long b){
    if(a>b)
        return (a/gcd(a,b))*b;
    else
        return (b/gcd(a,b))*a;
}
