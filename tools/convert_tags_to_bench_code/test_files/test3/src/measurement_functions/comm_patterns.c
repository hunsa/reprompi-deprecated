#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include <string.h>
//@ add_includes


char basetype[] = "MPI_INT";

void alltoall_pattern(int n_procs) {
    int i, j, msize;
    void *send_buffer, *recv_buffer;
    int rank;
    char* meas_functions[] = {"MPI_Alltoall", "MPI_Bcast"};
    int msize_list[] = {1, 64, 1024};
    int n_msizes = 3;
    int n_calls = 2;
    char* meas_func;

    //@ declare_variables

    //@ initialize_timestamps t1
    //@ initialize_timestamps t2


    //@ global basetype=basetype
    int count = 100;
    int INC=10;
    for (i=0; i<count; i+=INC) {
        for(j=0;j<n_calls; j++) {
            msize = msize_list[0];
            meas_func = meas_functions[j];
            //@ set testtype=meas_functions[j]


            send_buffer = malloc( n_procs * msize);
            recv_buffer = malloc( n_procs * msize);

            //@ start_measurement_loop

            //@ start_sync
            //@ measure_timestamp t1

                MPI_Alltoall( send_buffer, msize, MPI_BYTE, recv_buffer,
                        msize, MPI_BYTE, MPI_COMM_WORLD);

            //@ measure_timestamp t2
            //@ stop_sync
            //@stop_measurement_loop

            //@ print_runtime_array name=runtime end_time=t2 start_time=t1 type=reduce op=max testtype=testtype count=i

            //@ print_runtime_array name=runtime_per_process end_time=t2 start_time=t1 type=all testtype=testtype count=i


            free( send_buffer);
            send_buffer = NULL;
            free( recv_buffer);
            recv_buffer = NULL;
        }
    }

    //@cleanup_variables

}
