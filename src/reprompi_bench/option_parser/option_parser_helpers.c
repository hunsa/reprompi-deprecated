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
#include <mpi.h>

static const int OUTPUT_ROOT_PROC = 0;

void reprompib_print_common_help(void) {
    int my_rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    if (my_rank == OUTPUT_ROOT_PROC) {
        printf("%-40s %-40s\n", "-h", "print this help");
        printf("%-40s %-40s\n", "-v",
                "increase verbosity level (print times measured for each process)");
        printf("%-40s %-40s\n", "-f | --input-file=<path>",
                "input file containing the list of benchmarking jobs");
        printf("%-40s %-40s\n", "--output-file=<path>",
                        "results file");
        printf("%-40s %-40s\n %50s%s\n", "--msizes-list=<values>",
                "list of comma-separated message sizes in Bytes,", "",
                "e.g., --msizes-list=10,1024");
        printf("%-40s %-40s\n %50s%s\n %50s%s\n",
                "--msize-interval=min=<min>,max=<max>,step=<step>",
                "list of power of 2 message sizes as an interval between 2^<min> and 2^<max>,",
                "", "with 2^<step> distance between values, ", "",
                "e.g., --msize-interval=min=1,max=4,step=1");
        printf("%-40s %-40s\n", "--pingpong-ranks=<rank1,rank2>",
                "two comma-separated ranks to be used for the ping-pong operations");
        printf("%-40s %-40s\n", "--root-proc=<process_id>",
                "root node for collective operations");
        printf("%-40s %-40s\n %50s%s\n", "--operation=<mpi_op>",
                "MPI operation applied by collective operations (where applicable)", "",
                "e.g., --operation=MPI_BOR ");
        printf("%40s Supported operations:\n", "");
        printf("%50s%s\n", "","MPI_BOR, MPI_BAND, MPI_LOR, MPI_LAND, MPI_MIN, MPI_MAX, MPI_SUM, MPI_PROD");
        printf("%-40s %-40s\n %50s%s\n", "--datatype=<mpi_type>",
                "MPI datatype used by collective operations", "",
                "e.g., --datatype=MPI_BYTE ");
        printf("%40s Supported datatypes:\n", "");
        printf("%50s%s\n", "","MPI_BYTE, MPI_CHAR, MPI_INT, MPI_FLOAT, MPI_DOUBLE");

        printf("%-40s %-40s\n", "--shuffle-jobs",
                "shuffle experiments before running the benchmark");
        printf("%-40s %-40s\n", "--params=<k1>:<v1>,<k2>:<v2>",
                "list of comma-separated <key>:<value> pairs to be printed in the benchmark output");
        printf("\n");
        printf("%-40s %-40s\n %50s%s\n", "--calls-list=<args>",
                "list of comma-separated MPI calls to be benchmarked,", "",
                "e.g., --calls-list=MPI_Bcast,MPI_Allgather");
        printf("%40s Supported MPI calls (and ping-pong operations):\n", "");
        printf("%50s%s\n%50s%s\n%50s%s", "",
                "MPI_Bcast, MPI_Alltoall, MPI_Allgather, MPI_Scan, MPI_Gather,",
                "", "MPI_Scatter, MPI_Reduce, MPI_Allreduce, MPI_Barrier, Send_Recv,",
                "", "Isend_Recv, Isend_Irecv, Sendrecv\n");

        printf("\nWindow-based synchronization options:\n");
        printf("%-40s %-40s\n", "--window-size=<win>",
                "window size in microseconds for window-based synchronization (default: 1 ms)");
        printf("%-40s %-40s\n", "--wait-time=<wait>",
                "wait time in microseconds before the start of the first window (default: 1 ms)");

        printf("\nSpecific options for the linear model of the clock skew:\n");
        printf("%-40s %-40s\n", "--fitpoints=<nfit>",
                "number of fitpoints (default: 20)");
        printf("%-40s %-40s\n", "--exchanges=<nexc>",
                "number of exchanges (default: 10)\n");

    }
}


