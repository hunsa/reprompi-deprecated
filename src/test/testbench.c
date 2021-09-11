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
#include "mpi.h"

//#include "parse_options.h"
#include "../collective_ops/collectives.h"
#include "testbench.h"

typedef double test_type;


void set_buffer_sequentially(const test_type start_val, const int n_elems, char* buffer) {
    int i;
    test_type val = start_val;
    test_type *buff;

    buff = (test_type*)buffer;
    for (i=0; i< n_elems; i++) {
        buff[i] = val++;
    }
}


void set_buffer_random(const int n_elems, char* buffer) {
    int i;
    test_type *buff;

    buff = (test_type*)buffer;
    for (i=0; i< n_elems; i++) {
        buff[i] = rand();
    }
}


void set_buffer_sequentially_char(const char start_val, const int n_elems, char* buffer) {
    int i;
    char val = start_val;
    //int *buff;

    //buff = (int*)buffer;
    for (i=0; i< n_elems; i++) {
        buffer[i] = val++;
    }
}

void set_buffer_const(const test_type val, const int n_elems, char* buffer) {
    int i;
    test_type *buff;

    buff = (test_type*)buffer;
    for (i=0; i< n_elems; i++) {
        buff[i] = val;
    }
}


void collect_buffers(const collective_params_t params, char* send_buffer, char* recv_buffer) {

    // gather measurement results
    MPI_Gather(params.sbuf,  params.scount * params.datatype_extent, MPI_CHAR,
            send_buffer,  params.scount * params.datatype_extent, MPI_CHAR,
            0, MPI_COMM_WORLD);
    MPI_Gather(params.rbuf,  params.rcount * params.datatype_extent, MPI_CHAR,
            recv_buffer,  params.rcount * params.datatype_extent, MPI_CHAR,
            0, MPI_COMM_WORLD);
}


void print_buffers(char coll1[], char coll2[], test_type* buffer1, test_type* buffer2, int count) {
/*
    int my_rank;
    int i;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    if (my_rank == 0) {
        printf("%s\n", coll1);
        for (i=0; i< count; i++) {
            printf ("%lf ", (double)buffer1[i] );
        }
        printf("\n");

        printf("%s\n", coll2);
        for (i=0; i< count; i++) {
            printf ("%lf ", (double)buffer2[i] );
        }
        printf("\n");
    }*/

}


int identical(test_type* buffer1, test_type* buffer2, int count) {
    int i;
    int identical = 1;

    for (i=0; i< count; i++) {
        if (buffer1[i] != buffer2[i]) {
            identical = 0;
        }
    }

    return identical;
}


void check_results(char coll1[], char coll2[],
        collective_params_t coll_params,
        collective_params_t mockup_params,
        int check_only_at_root) {

    int my_rank, p;
    int error = 0;
    test_type *send_buffer, *mockup_send_buffer;
    test_type *recv_buffer, *mockup_recv_buffer;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    // gather send and receive buffers from all processes
    send_buffer = (test_type*)malloc(coll_params.nprocs* coll_params.scount * coll_params.datatype_extent);
    mockup_send_buffer = (test_type*)malloc(mockup_params.nprocs* mockup_params.scount * mockup_params.datatype_extent);

    recv_buffer = (test_type*)malloc(coll_params.nprocs* coll_params.rcount * coll_params.datatype_extent);
    mockup_recv_buffer = (test_type*)malloc(mockup_params.nprocs* mockup_params.rcount * mockup_params.datatype_extent);

    collect_buffers(coll_params, (char*)send_buffer, (char*)recv_buffer);
    collect_buffers(mockup_params, (char*)mockup_send_buffer, (char*)mockup_recv_buffer);

    if (my_rank == 0) {
        printf ("----------------------------------------\n");
        printf ("---------------- Comparing functions %s and %s\n", coll1, coll2);

        if (check_only_at_root) {
            p = coll_params.root;
            print_buffers(coll1, coll2,
                    recv_buffer + p*coll_params.rcount,
                    mockup_recv_buffer + p*coll_params.rcount,
                    coll_params.rcount);
            error = (!identical(recv_buffer + p*coll_params.rcount,
                    mockup_recv_buffer + p*coll_params.rcount,
                    coll_params.rcount));
        }
        else {
            if (strcmp(coll1, "MPI_Bcast")){

                for (p=0; p<coll_params.nprocs; p++) {
                    printf ("=========== Process %d\n", p);

                    print_buffers(coll1, coll2,
                            recv_buffer + p*coll_params.rcount,
                            mockup_recv_buffer + p*coll_params.rcount,
                            coll_params.rcount);
                }
                error = (!identical(recv_buffer, mockup_recv_buffer, coll_params.nprocs * coll_params.rcount));
            }
            else {
                // for Bcast check source buffers
                for (p=0; p<coll_params.nprocs; p++) {
                    printf ("=========== Process %d\n", p);

                    print_buffers(coll1, coll2,
                            send_buffer + p*coll_params.scount,
                            mockup_send_buffer + p*coll_params.scount,
                            coll_params.scount);
                }
                error = (!identical(send_buffer, mockup_send_buffer, coll_params.nprocs * coll_params.scount));

            }

        }
        if (error) {
            printf ("****************\n**************** TEST FAILED for %s and %s\n", coll1, coll2);
            printf("****************\n****************\n\n");
        }
        else {
            printf ("---- Test passed.\n\n");
        }
    }
}


void test_collective(basic_collective_params_t basic_coll_info,
        long count, int coll_index, int mockup_index) {

    long n_elems, i;
    int check_only_at_root = 0;

    collective_params_t coll_params, mockup_params;

    // initialize operations
    collective_calls[coll_index].initialize_data(basic_coll_info, count, &coll_params);
    collective_calls[mockup_index].initialize_data(basic_coll_info, count, &mockup_params);

    // setup buffers
    n_elems = coll_params.scount;
    set_buffer_random(n_elems, coll_params.sbuf);
    for (i=0;i<n_elems;i++) {
        ((test_type*)mockup_params.sbuf)[i] = ((test_type*)coll_params.sbuf)[i];
    }

    // execute collective op
    collective_calls[coll_index].collective_call(&coll_params);
    collective_calls[mockup_index].collective_call(&mockup_params);

    if (coll_index == MPI_GATHER || coll_index == MPI_REDUCE) {
        check_only_at_root = 1;
    }
    check_results(get_call_from_index(coll_index), get_call_from_index(mockup_index),
            coll_params, mockup_params, check_only_at_root);



    // cleanup data
    collective_calls[coll_index].cleanup_data(&coll_params);
    collective_calls[mockup_index].cleanup_data(&mockup_params);

}




int main(int argc, char* argv[]) {
    int nprocs;
    basic_collective_params_t basic_coll_info;
    long count = 100;


    srand(1000);


    if (argc > 1) {
        count = atol(argv[1]);
    }

    /* start up MPI
     *
     * */
    MPI_Init(&argc, &argv);


    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    basic_coll_info.datatype = MPI_DOUBLE;
    basic_coll_info.op = MPI_SUM;
    basic_coll_info.root = 0;
    basic_coll_info.nprocs = nprocs;

    test_collective(basic_coll_info, count, MPI_ALLGATHER, GL_ALLGATHER_AS_ALLREDUCE);
    test_collective(basic_coll_info, count, MPI_ALLGATHER, GL_ALLGATHER_AS_ALLTOALL);
    test_collective(basic_coll_info, count, MPI_ALLGATHER, GL_ALLGATHER_AS_GATHERBCAST);

    test_collective(basic_coll_info, count, MPI_ALLREDUCE, GL_ALLREDUCE_AS_REDUCEBCAST);
    test_collective(basic_coll_info, count, MPI_ALLREDUCE, GL_ALLREDUCE_AS_REDUCESCATTERALLGATHERV);

    test_collective(basic_coll_info, count, MPI_BCAST, GL_BCAST_AS_SCATTERALLGATHER);

    test_collective(basic_coll_info, count, MPI_GATHER, GL_GATHER_AS_ALLGATHER);
    test_collective(basic_coll_info, count, MPI_GATHER, GL_GATHER_AS_REDUCE);

    test_collective(basic_coll_info, count, MPI_REDUCE, GL_REDUCE_AS_ALLREDUCE);

    test_collective(basic_coll_info, count, MPI_REDUCE, GL_REDUCE_AS_REDUCESCATTERGATHERV);

    test_collective(basic_coll_info, count, MPI_REDUCE_SCATTER, GL_REDUCESCATTER_AS_ALLREDUCE);
    test_collective(basic_coll_info, count, MPI_REDUCE_SCATTER, GL_REDUCESCATTER_AS_REDUCESCATTERV);
    test_collective(basic_coll_info, count, MPI_REDUCE_SCATTER_BLOCK, GL_REDUCESCATTERBLOCK_AS_REDUCESCATTER);

    test_collective(basic_coll_info, count, MPI_SCAN, GL_SCAN_AS_EXSCANREDUCELOCAL);

    test_collective(basic_coll_info, count, MPI_SCATTER, GL_SCATTER_AS_BCAST);

    if (count % nprocs == 0) {  // only works if the number of processes is a divisor of count
        test_collective(basic_coll_info, count, MPI_REDUCE, GL_REDUCE_AS_REDUCESCATTERBLOCKGATHER);
        test_collective(basic_coll_info, count, MPI_ALLREDUCE, GL_ALLREDUCE_AS_REDUCESCATTERBLOCKALLGATHER);
    }
    else {

    }

    /* shut down MPI */
    MPI_Finalize();

    return 0;
}
