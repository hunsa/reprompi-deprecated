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
#include <stdlib.h>
#include "mpi.h"

#include "runtimes_computation.h"
#include "reprompi_bench/sync/synchronization.h"


void compute_runtimes_local_clocks(double* tstart_sec, double* tend_sec,
        long current_start_index, long current_nreps, int root_proc,
        double* maxRuntimes_sec) {

    double* local_runtimes = NULL;
    int i, index;
    int my_rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    // compute local runtimes for the [current_start_index, current_start_index + current_nreps) interval
    local_runtimes = (double*) malloc(current_nreps * sizeof(double));
    for (i = 0; i < current_nreps; i++) {
        index = i + current_start_index;
        local_runtimes[i] = tend_sec[index] - tstart_sec[index];
    }

    // reduce local measurement results on the root
    MPI_Reduce(local_runtimes, maxRuntimes_sec, current_nreps,
            MPI_DOUBLE, MPI_MAX, root_proc, MPI_COMM_WORLD);

    free(local_runtimes);
}



void compute_runtimes_global_clocks(double* tstart_sec, double* tend_sec,
        long current_start_index, long current_nreps, int root_proc,
        sync_errorcodes_t get_errorcodes, sync_normtime_t get_global_time,
        double* maxRuntimes_sec, int* sync_errorcodes) {

    double* start_sec = NULL;
    double* end_sec = NULL;
    double* norm_tstart_sec;
    double* norm_tend_sec;
    int i, index;
    int my_rank;
    int* local_errorcodes;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    // gather error codes in the  [current_start_index, current_start_index + current_nreps) interval
    local_errorcodes = get_errorcodes() + current_start_index;

    MPI_Reduce(local_errorcodes, sync_errorcodes, current_nreps,
            MPI_INT, MPI_MAX, root_proc, MPI_COMM_WORLD);

    if (my_rank == root_proc)
    {
        start_sec = (double*)malloc(current_nreps * sizeof(double));
        end_sec = (double*)malloc(current_nreps * sizeof(double));
    }

    norm_tstart_sec = (double*)malloc(current_nreps * sizeof(double));
    norm_tend_sec = (double*)malloc(current_nreps * sizeof(double));

    // normalize results in the  [current_start_index, current_start_index + current_nreps) interval
    for (i = 0; i < current_nreps; i++) {
        index = i + current_start_index;
        norm_tstart_sec[i] = get_global_time(tstart_sec[index]);
        norm_tend_sec[i] = get_global_time(tend_sec[index]);
    }

    // gather results at the root process and compute runtimes
    MPI_Reduce(norm_tstart_sec, start_sec, current_nreps, MPI_DOUBLE, MPI_MIN, root_proc, MPI_COMM_WORLD);
    MPI_Reduce(norm_tend_sec, end_sec, current_nreps, MPI_DOUBLE, MPI_MAX, root_proc, MPI_COMM_WORLD);

    if (my_rank == root_proc) {
        for (i = 0; i< current_nreps; i++) {
            maxRuntimes_sec[i] = end_sec[i] - start_sec[i];
        }


        free(start_sec);
        free(end_sec);
    }

    free(norm_tstart_sec);
    free(norm_tend_sec);

}

