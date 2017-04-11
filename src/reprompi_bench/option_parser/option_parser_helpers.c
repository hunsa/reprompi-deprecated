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
                "--msize-interval min=<min>,max=<max>,step=<step>",
                "list of power of 2 message sizes as an interval between 2^<min> and 2^<max>,",
                "", "with <step> distance between values, ", "",
                "e.g., --msize-interval min=1,max=4,step=1");
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
                "window size in microseconds for window-based synchronization");
        printf("%-40s %-40s\n", "--wait-time=<wait>",
                "wait time in microseconds before the start of the first window (default: 1 ms)");

        printf("\nSpecific options for the linear model of the clock skew:\n");
        printf("%-40s %-40s\n", "--fitpoints=<nfit>",
                "number of fitpoints (default: 20)");
        printf("%-40s %-40s\n", "--exchanges=<nexc>",
                "number of exchanges (default: 10)\n");

    }
}


void reprompib_print_benchmark_help(void) {
    int my_rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    if (my_rank == OUTPUT_ROOT_PROC) {
        printf("\nUSAGE: mpibenchmark [options]\n");
        printf("options:\n");
    }

    reprompib_print_common_help();

    if (my_rank == OUTPUT_ROOT_PROC) {
        printf("\nSpecific options for the benchmark execution:\n");
        printf("%-40s %-40s\n", "-r | --repetitions=<nrep>",
                "set number of experiment repetitions");
        printf("%-40s %-40s\n %50s%s\n", "--summary=<args>",
                "list of comma-separated data summarizing methods (mean, median, min, max)", "",
                "e.g., --summary=mean,max");

        printf("\nEXAMPLES: mpirun -np 4 ./bin/mpibenchmark --calls-list=MPI_Bcast --msizes-list=8,512,1024 -r 5 --summary=mean,max,min\n");
        printf("\n          mpirun -np 4 ./bin/mpibenchmark --calls-list=MPI_Bcast --msizes-list=8,512,1024 -r 5\n");
        printf("\n          mpirun -np 4 ./bin/mpibenchmark --window-size=100 --calls-list=MPI_Bcast --msize-interval min=1,max=8,step=1 -r 5\n");
        printf("\n          mpirun -np 4 ./bin/mpibenchmark --window-size=100 --calls-list=MPI_Bcast --msizes-list=1024 -r 5 --fitpoints=10 --exchanges=20\n");
        printf("\n          mpirun -np 4 ./bin/mpibenchmark --window-size=100 --calls-list=MPI_Bcast --msizes-list=1024 -r 5 --params=p1:1,p2:aaa,p3:34\n");
        printf("\n          mpirun -np 4 ./bin/mpibenchmark --calls-list=Sendrecv --msizes-list=10 --pingpong-ranks=0,3 -r 5 --summary \n");

        printf("\n\n");
    }
}




void reprompib_print_prediction_help(void) {
    int my_rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    if (my_rank == OUTPUT_ROOT_PROC) {
        printf("\nUSAGE: mpibenchmarkPredNreps [options]\n");
        printf("options:\n");
    }

    reprompib_print_common_help();

    if (my_rank == OUTPUT_ROOT_PROC) {
        printf("\nSpecific options for estimating the number of repetitions:\n");
        printf("%-40s %-40s\n %50s%s\n %50s%s\n %50s%s\n",
                "--rep-prediction min=<min>,max=<max>,step=<step>",
                "set the total number of repetitions to be estimated between <min> and <max>,",
                "", "so that at each iteration i, the number of measurements (nrep) is either",
                "", "nrep(0) = <min>, or nrep(i) = nrep(i-1) + <step> * 2^(i-1),  ", "",
                "e.g., --rep-prediction min=1,max=4,step=1");
        printf("%-40s %-40s\n", "--pred-method=m1,m2",
                " comma-separated list of prediction methods, i.e., rse, cov_mean, cov_median (default: rse)");
        printf("%-40s %-40s\n %50s%s\n", "--var-thres=thres1,thres2",
                " comma-separated list of thresholds corresponding to the specified prediction methods ",
                "", "(default: 0.01)");
        printf("%-40s %-40s\n %50s%s\n %50s%s\n %50s%s\n", "--var-win=win1,win2",
                " comma-separated list of (non-zero) windows corresponding to the specified prediction methods;",
                "", "rse does not rely on a measurement window, however a dummy window value is required ",
                "", "in this list when multiple methods are used ",
                "", "(default: 10)");

        printf("\nEXAMPLES: mpirun -np 4 ./bin/mpibenchmarkPredNreps --calls-list=MPI_Bcast --msizes-list=1024 --rep-prediction min=1,max=100,step=1\n");
        printf("\n          mpirun -np 4 ./bin/mpibenchmarkPredNreps --calls-list=MPI_Bcast --msizes-list=8,512,1024 --rep-prediction min=1,max=100,step=1 \n");
        printf("\n          mpirun -np 4 ./bin/mpibenchmarkPredNreps --calls-list=MPI_Bcast --msizes-list=1024 --rep-prediction min=1,max=100,step=1 --pred-method=cov_mean,rse --var-thres=0.01,0.1 --var-win=10,1\n");

        printf("\n\n");
    }
}



void reprompib_print_error_and_exit(const char* error_str) {
  int my_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  if (my_rank == OUTPUT_ROOT_PROC) {
    fprintf(stderr, "\nERROR: %s\n\n", error_str);
  }
  MPI_Finalize();
  exit(0);
}
