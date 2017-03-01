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

#include "sync/time_measurement.h"
#include "parse_test_options.h"

int main(int argc, char* argv[]) {
    int my_rank, nprocs, p;
    int i;
    reprompib_st_opts_t opts;
    int master_rank;
    MPI_Status stat;

    double *all_frequencies;
    double *all_wtime_times;
    double *frequencies;
    double *wtime_times;
    double time_msg[2];

    int step;
    int n_wait_steps = 11;
    double wait_time_s = 1;

    /* start up MPI */
    MPI_Init(&argc, &argv);
    init_globals();
    master_rank = get_master_rank();

    parse_test_options(&opts, argc, argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    n_wait_steps = opts.n_rep + 1;

//	wtime_times = (double*)calloc(n_wait_steps, sizeof(double));
    frequencies = (double*) calloc(n_wait_steps, sizeof(double));

    if (my_rank == master_rank) {
//		all_wtime_times = (double*)calloc(nprocs*n_wait_steps, sizeof(double));
        all_frequencies = (double*) calloc(nprocs * n_wait_steps,
                sizeof(double));
    }

    for (step = 0; step < n_wait_steps; step++) {
        uint64_t timerfreq = 0;
        HRT_INIT(0 /* do not print */, timerfreq);

        frequencies[step] = (double) timerfreq;
//       wtime_times[step] = MPI_Wtime();

// wait 1 second
        struct timespec sleep_time;
        sleep_time.tv_sec = 0;
        sleep_time.tv_nsec = wait_time_s * 1e9;

        nanosleep(&sleep_time, NULL);
    }

    if (my_rank == master_rank) {
        printf("wait_time_s p freq\n");
    }

    // gather measurement results
    MPI_Gather(frequencies, n_wait_steps, MPI_DOUBLE, all_frequencies,
            n_wait_steps, MPI_DOUBLE, 0, MPI_COMM_WORLD);

//	MPI_Gather(wtime_times, n_wait_steps, MPI_DOUBLE,
    //		all_wtime_times, n_wait_steps, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (my_rank == master_rank) {
        for (p = 0; p < nprocs; p++) {
            for (step = 0; step < n_wait_steps; step++) {
                printf("%14.9f %3d %14.9f\n", step * wait_time_s, p,
                //all_wtime_times[p*n_wait_steps + step]
                        all_frequencies[p * n_wait_steps + step]);
            }
        }
    }

//    free(wtime_times);
    free(frequencies);
    if (my_rank == master_rank) {
        free(all_frequencies);
//		free(all_wtime_times);

    }

    MPI_Finalize();
    reprompib_free_parameters(&opts);

    return 0;
}
