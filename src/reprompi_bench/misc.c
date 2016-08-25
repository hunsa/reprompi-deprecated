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

#include "misc.h"

static char * const error_messages[] =
        { "Call list contains invalid MPI calls",
                "Call list is empty",
                "Message sizes list contains invalid elements",
                "Message sizes list is empty",
                "Number of repetitions null or not specified",
                "Min message size in interval is null or not correctly specified",
                "Max message size in interval is null or not correctly specified",
                "Message size step in interval is null or not correctly specified",
                "Message size interval contains unknown suboptions",
                "Message size interval min value is not smaller than the max value",
                "Unknown option",
                "Window size is invalid",
                "Number of fitpoints is invalid",
                "Number of exchanges is invalid",
                "Batch size is invalid or not smaller than the total number of repetitions specified with the -r option",
                "Min number of repetitions in interval is null or not correctly specified",
                "Max number of repetitions in interval is null or not correctly specified",
                "Number of repetitions step in interval is null or not correctly specified",
                "Number of repetitions interval contains unknown suboptions",
                "Number of repetitions interval min value is not smaller than the max value",
                "Prediction methods list contains unknown or too many methods",
                "Prediction methods list is empty",
                "Prediction method threshold list contains invalid or not the same number of values as the specified number of methods",
                "Prediction method threshold list is empty",
                "Prediction method window list contains invalid or not the same number of values as the specified number of methods",
                "Prediction method window list is empty",
                "Data summary list contains invalid functions",
                "Data summary list is empty",
                "Key-value parameters invalid",
                "Output file cannot be opened by process 0",
                "Datatype is invalid",
                "MPI operation is invalid",
                "Root process is invalid"
                                };
static const int N_ERRORS = sizeof(error_messages) / sizeof(error_messages[0]);

char* get_error_message(reprompib_error_t error) {
    int index = (int)(log(error)/log(2));
    if (index < 0 || index >= N_ERRORS) {
        return "";
    }

    return error_messages[index];
}

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


