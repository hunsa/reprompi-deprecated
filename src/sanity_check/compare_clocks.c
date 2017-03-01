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
#include "parse_test_options.h"

int main(int argc, char* argv[]) {
    int my_rank, nprocs, p;
    reprompib_st_opts_t opts;
    int master_rank;
    reprompib_st_error_t ret;

    double *all_rdtsc_times = NULL;
    double *all_wtime_times = NULL;
    double *rdtsc_times;
    double *wtime_times;

    int step;
    int n_wait_steps = 11;
    double wait_time_s = 0.1;

    /* start up MPI */
    MPI_Init(&argc, &argv);
    master_rank = 0;

    ret = parse_test_options(&opts, argc, argv);
    validate_test_options_or_abort(ret, &opts);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    n_wait_steps = opts.n_rep + 1;

    wtime_times = (double*) calloc(n_wait_steps, sizeof(double));
    rdtsc_times = (double*) calloc(n_wait_steps, sizeof(double));

    if (my_rank == master_rank) {
        all_wtime_times = (double*) calloc(nprocs * n_wait_steps,
                sizeof(double));


        all_rdtsc_times = (double*) calloc(nprocs * n_wait_steps,
                sizeof(double));
    }

    for (step = 0; step < n_wait_steps; step++) {
        rdtsc_times[step] = get_time();
        wtime_times[step] = MPI_Wtime();

        // wait 1 second
        struct timespec sleep_time;
        sleep_time.tv_sec = 0;
        sleep_time.tv_nsec = wait_time_s * 1e9;

        nanosleep(&sleep_time, NULL);
    }

    if (my_rank == master_rank) {
        printf("wait_time_s p wtime rdtsc\n");
    }

    // gather measurement results
    MPI_Gather(rdtsc_times, n_wait_steps, MPI_DOUBLE, all_rdtsc_times,
            n_wait_steps, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    MPI_Gather(wtime_times, n_wait_steps, MPI_DOUBLE, all_wtime_times,
            n_wait_steps, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (my_rank == master_rank) {
        for (p = 0; p < nprocs; p++) {
            for (step = 0; step < n_wait_steps; step++) {
                printf("%14.9f %3d %14.9f %14.9f\n", step * wait_time_s, p,
                        all_wtime_times[p * n_wait_steps + step],
                        all_rdtsc_times[p * n_wait_steps + step]);
            }
        }
    }

    free(wtime_times);
    free(rdtsc_times);
    if (my_rank == master_rank) {
        free(all_rdtsc_times);
        free(all_wtime_times);

    }

    MPI_Finalize();

    return 0;
}
