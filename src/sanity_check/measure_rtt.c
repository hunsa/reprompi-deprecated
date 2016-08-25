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

#include "reprompi_bench/misc.h"
#include "reprompi_bench/sync/time_measurement.h"

const int WARMUP_ROUNDS = 10;

int main(int argc, char* argv[]) {
    int my_rank, nprocs, p;
    int i;
    MPI_Status stat;
    double t_start, t_remote, *rtt_list;

    /* start up MPI */
    MPI_Init(&argc, &argv);

    if (argc < 3) {
        printf("USAGE: mpirun -np 4 ./src/sanity_check/measure_rtt n_repetitions root_proc\n");
        exit(1);
    }

    int nrep = atol(argv[1]);
    int root = atoi(argv[2]);

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    if (my_rank == root)
    {
        printf("root process nrep rtt\n");

    }

    if (my_rank == root)
    {
        /* warm up          */
        for (p = 0; p < nprocs; p++)
        {
            if (p!=root) {
                for (i = 0; i < WARMUP_ROUNDS; i++)
                {
                    double tmp;

                    tmp = get_time();
                    MPI_Send(&tmp, 1, MPI_DOUBLE, p, 0, MPI_COMM_WORLD);
                    MPI_Recv(&tmp, 1, MPI_DOUBLE, p, 0, MPI_COMM_WORLD, &stat);
                }
            }
        }

        rtt_list = (double*)malloc(nrep * sizeof(double));

        //MPI_Barrier(MPI_COMM_WORLD);
        for (p = 0; p < nprocs; p++)
        {
            if (p!=root) {
                for (i = 0; i < nrep; i++)
                {
                    t_start = get_time();

                    MPI_Send(&t_start, 1, MPI_DOUBLE, p, 0, MPI_COMM_WORLD);
                    MPI_Recv(&t_remote, 1, MPI_DOUBLE, p, 0, MPI_COMM_WORLD, &stat);

                    rtt_list[i] = get_time() - t_start;
                }

                for (i = 0; i < nrep; i++)
                {
                    printf("%d %d %d %3.10f\n", root, p, i, rtt_list[i]);
                }
            }
        }

        free(rtt_list);

    }
    else
    {
        /* warm up */
        for (i = 0; i < WARMUP_ROUNDS; i++)
        {
            double tmp;
            MPI_Recv(&tmp, 1, MPI_DOUBLE, root, 0, MPI_COMM_WORLD, &stat);
            tmp = get_time();
            MPI_Send(&tmp, 1, MPI_DOUBLE, root, 0, MPI_COMM_WORLD);
        }

        //MPI_Barrier(MPI_COMM_WORLD);
        for (i = 0; i < nrep; i++)
        {
            MPI_Recv(&t_start, 1, MPI_DOUBLE, root, 0, MPI_COMM_WORLD, &stat);
            MPI_Send(&t_start, 1, MPI_DOUBLE, root, 0, MPI_COMM_WORLD);
        }
    }

    MPI_Finalize();

    return 0;
}
