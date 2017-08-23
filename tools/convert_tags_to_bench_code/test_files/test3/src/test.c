#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include "measurement_functions/comm_patterns.h"

//@ add_includes
//@ declare_variables

int main(int argc, char *argv[])
{
    int n_procs = 0;
    int rank;


    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size( MPI_COMM_WORLD, &n_procs);


    //@ initialize_bench

    alltoall_pattern(n_procs);

    //@cleanup_bench

    MPI_Finalize();

    return 0;
}
