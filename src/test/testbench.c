/*  ReproMPI Benchmark
 *
 *  Copyright 2015 Alexandra Carpen-Amarie, Sascha Hunold
    Research Group for Parallel Computing
    Faculty of Informatics
    Vienna University of Technology, Austria
 *
 * Copyright (c) 2021 Stefan Christians
 *
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

#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mpi.h"

#include "../collective_ops/collectives.h"
#include "testbench.h"

#include "contrib/intercommunication/intercommunication.h"

#if SIZE_MAX == UCHAR_MAX
#define MPI_SIZE_T MPI_UNSIGNED_CHAR
#elif SIZE_MAX == USHRT_MAX
#define MPI_SIZE_T MPI_UNSIGNED_SHORT
#elif SIZE_MAX == UINT_MAX
#define MPI_SIZE_T MPI_UNSIGNED
#elif SIZE_MAX == ULONG_MAX
#define MPI_SIZE_T MPI_UNSIGNED_LONG
#elif SIZE_MAX == ULLONG_MAX
#define MPI_SIZE_T MPI_UNSIGNED_LONG_LONG
#else
#define MPI_SIZE_T MPI_UNSIGNED_LONG
#endif

typedef double test_type;

enum {
    MSG_TAG_COLL_SCOUNT = 3,
    MSG_TAG_MOCK_SCOUNT,
    MSG_TAG_COLL_RCOUNT,
    MSG_TAG_MOCK_RCOUNT,
};

static const int OUTPUT_ROOT_PROC = 0;

static const struct option test_long_options[] = {
        { "count", required_argument, 0, 'c' },
        { "help", no_argument, 0, 'c' },
        { NULL, 0, NULL, 0 }
};

static const char test_short_options[] = "c:h";

static void print_help (char* command)
{
    if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC))
    {
        printf("Usage: mpiexec -n <numprocs> %s [options]\n", command);

        printf("\ntest options:\n");
        printf("%-25s %-.54s\n", "--count=<nummsgs>", "communicates <nummsgs> number of messages");

        icmb_print_intercommunication_help();
    }
}

static void parse_test_options (int argc, char** argv, long* count)
{
    // we expect to be started after MPI_Init was called
    int is_initialized;
    MPI_Initialized(&is_initialized);
    assert(is_initialized);

    int c;
    opterr = 0;

	while(1) {
		c = getopt_long(argc, argv, test_short_options, test_long_options, NULL);
		if(c == -1) {
			break;
		}

		switch(c) {

			case 'c':
                *count = atol(optarg);
				break;

            case 'h':
                print_help(argv[0]);
                icmb_exit(0);
                break;
		}
	}
    optind = 1; // reset optind to enable option re-parsing
    opterr = 1; // reset opterr to catch invalid options
}

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
        // TODO: set back to random when done, now using rank for easy identification
        //buff[i] = rand();
        buff[i] = (!icmb_is_initiator())*10 + icmb_benchmark_rank();
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


void print_buffers(char coll1[], char coll2[], test_type* buffer1, test_type* buffer2, int count) {

    int i;

    if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC)) {

        printf("%s\n", coll1);
        for (i=0; i< count; i++) {
            // TODO: remove 2.1 format
            //printf ("%lf ", ((double*)buffer1)[i] );
            printf ("%2.1lf ", ((double*)buffer1)[i] );
        }
        printf("\n");

        printf("%s\n", coll2);
        for (i=0; i< count; i++) {
            // TODO: remove 2.1 format
            //printf ("%lf ", ((double*)buffer2)[i] );
            printf ("%2.1lf ", ((double*)buffer2)[i] );
        }
        printf("\n");
    }

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

static int check_results_for_intracommunicator (char coll1[], char coll2[], collective_params_t coll_params, collective_params_t mockup_params, int check_only_at_root)
{
    int error = 0;

    int p;
    test_type *send_buffer;
    test_type *recv_buffer, *mockup_recv_buffer;

    // gather send and receive buffers from all processes
    send_buffer = (test_type*)malloc(coll_params.local_size* coll_params.scount * coll_params.datatype_extent);
    MPI_Gather(coll_params.sbuf,  coll_params.scount * coll_params.datatype_extent, MPI_CHAR, send_buffer,  coll_params.scount * coll_params.datatype_extent, MPI_CHAR, OUTPUT_ROOT_PROC, coll_params.communicator);

    recv_buffer = (test_type*)malloc(coll_params.local_size* coll_params.rcount * coll_params.datatype_extent);
    MPI_Gather(coll_params.rbuf,  coll_params.rcount * coll_params.datatype_extent, MPI_CHAR, recv_buffer,  coll_params.rcount * coll_params.datatype_extent, MPI_CHAR, OUTPUT_ROOT_PROC, coll_params.communicator);

    mockup_recv_buffer = (test_type*)malloc(mockup_params.local_size* mockup_params.rcount * mockup_params.datatype_extent);
    MPI_Gather(mockup_params.rbuf,  mockup_params.rcount * mockup_params.datatype_extent, MPI_CHAR, mockup_recv_buffer,  mockup_params.rcount * mockup_params.datatype_extent, MPI_CHAR, OUTPUT_ROOT_PROC, coll_params.communicator);

    if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC)) {

        if (check_only_at_root) {
            p = coll_params.root;
            printf("checking only at root :-)\n");
            print_buffers(coll1, coll2, recv_buffer + p*coll_params.rcount, mockup_recv_buffer + p*coll_params.rcount, coll_params.rcount);
            error = (!identical(recv_buffer + p*coll_params.rcount, mockup_recv_buffer + p*coll_params.rcount, coll_params.rcount));
        }
        else {
            if (strcmp(coll1, "MPI_Bcast")){
                for (p=0; p<coll_params.local_size; p++) {
                    printf ("=========== Process %d\n", p);
                    print_buffers(coll1, coll2, recv_buffer + p*coll_params.rcount, mockup_recv_buffer + p*coll_params.rcount, coll_params.rcount);
                }
                error = (!identical(recv_buffer, mockup_recv_buffer, coll_params.local_size * coll_params.rcount));
            }
            else {
               // for Bcast check source buffers
                for (p=0; p<coll_params.local_size; p++) {
                    printf ("=========== Process %d\n", p);
                    print_buffers(coll1, coll2, send_buffer + p*coll_params.scount, mockup_recv_buffer + p*coll_params.scount, coll_params.scount);
                }
                error = (!identical(send_buffer, mockup_recv_buffer, coll_params.local_size * coll_params.scount));

           }

        }
    }

    free(send_buffer);
    free(recv_buffer);
    free(mockup_recv_buffer);

    return error;
}

static int check_results_for_initiators (char coll1[], char coll2[], collective_params_t coll_params, collective_params_t mockup_params, int check_only_at_root)
{
    // initiators are not involved in one-to-all operations
    if (!strcmp(coll1, "MPI_Bcast")) // strcmp returns 0 (false) if strings match
    {
        return 0;
    }

    int error = 0;

    int p = 0;
    test_type *recv_buffer, *mockup_recv_buffer;

    // gather receive buffers from all processes
    recv_buffer = (test_type*)malloc(coll_params.local_size* coll_params.rcount * coll_params.datatype_extent);
    MPI_Gather(coll_params.rbuf,  coll_params.rcount * coll_params.datatype_extent, MPI_CHAR, recv_buffer,  coll_params.rcount * coll_params.datatype_extent, MPI_CHAR, OUTPUT_ROOT_PROC, icmb_partial_communicator());

    mockup_recv_buffer = (test_type*)malloc(mockup_params.local_size* mockup_params.rcount * mockup_params.datatype_extent);
    MPI_Gather(mockup_params.rbuf,  mockup_params.rcount * mockup_params.datatype_extent, MPI_CHAR, mockup_recv_buffer,  mockup_params.rcount * mockup_params.datatype_extent, MPI_CHAR, OUTPUT_ROOT_PROC, icmb_partial_communicator());

    if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC)) {

        if (check_only_at_root)
        {
            p = OUTPUT_ROOT_PROC;
            print_buffers(coll1, coll2, recv_buffer + p*coll_params.rcount, mockup_recv_buffer + p*coll_params.rcount, coll_params.rcount);
            error = (!identical(recv_buffer + p*coll_params.rcount, mockup_recv_buffer + p*coll_params.rcount, coll_params.rcount));
        }
        else
        {
                for (p=0; p<coll_params.local_size; p++) {
                    printf ("=========== Process Initiator %d\n", p);
                    print_buffers(coll1, coll2, recv_buffer + p*coll_params.rcount, mockup_recv_buffer + p*coll_params.rcount, coll_params.rcount);
                }
                error = (!identical(recv_buffer, mockup_recv_buffer, coll_params.local_size * coll_params.rcount));
        }
    }

    free(recv_buffer);
    free(mockup_recv_buffer);

    return error;
}

static int check_results_for_responders (char coll1[], char coll2[], collective_params_t coll_params, collective_params_t mockup_params, int check_only_at_root)
{
    // responders are not involved in all-to-one operations
    if (check_only_at_root)
    {
        return 0;
    }

    int error = 0;

    int p;

    if (icmb_is_responder() && 0 == icmb_benchmark_rank())
    {
        // inform unknown sizes to initiator root
        MPI_Send(&coll_params.scount, 1, MPI_SIZE_T, OUTPUT_ROOT_PROC, MSG_TAG_COLL_SCOUNT, icmb_benchmark_communicator());
        MPI_Send(&coll_params.rcount, 1, MPI_SIZE_T, OUTPUT_ROOT_PROC, MSG_TAG_COLL_RCOUNT, icmb_benchmark_communicator());
        MPI_Send(&mockup_params.rcount, 1, MPI_SIZE_T, OUTPUT_ROOT_PROC, MSG_TAG_MOCK_RCOUNT, icmb_benchmark_communicator());
    }

    if (icmb_is_responder())
    {
        // let initiator root gather our data
        MPI_Gather(coll_params.sbuf, coll_params.scount * coll_params.datatype_extent, MPI_CHAR, NULL, 0, MPI_CHAR, OUTPUT_ROOT_PROC, icmb_benchmark_communicator());
        MPI_Gather(coll_params.rbuf, coll_params.rcount * coll_params.datatype_extent, MPI_CHAR, NULL, 0, MPI_CHAR, OUTPUT_ROOT_PROC, icmb_benchmark_communicator());
        MPI_Gather(mockup_params.rbuf, mockup_params.rcount * mockup_params.datatype_extent, MPI_CHAR, NULL, 0, MPI_CHAR, OUTPUT_ROOT_PROC, icmb_benchmark_communicator());
    }

    if (icmb_is_initiator() && !icmb_has_initiator_rank(OUTPUT_ROOT_PROC))
    {
        // exclude other initiator processes from collective communication
        MPI_Gather(NULL, 0, MPI_CHAR, NULL, 0, MPI_CHAR, MPI_PROC_NULL, icmb_benchmark_communicator()); // send_buffer
        MPI_Gather(NULL, 0, MPI_CHAR, NULL, 0, MPI_CHAR, MPI_PROC_NULL, icmb_benchmark_communicator()); // recv_buffer
        MPI_Gather(NULL, 0, MPI_CHAR, NULL, 0, MPI_CHAR, MPI_PROC_NULL, icmb_benchmark_communicator()); // mockup_recv_buffer
    }

    if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC))
    {
        // receive remote sizes from responder root
        size_t coll_scount;
        MPI_Recv(&coll_scount, 1, MPI_SIZE_T, 0, MSG_TAG_COLL_SCOUNT, icmb_benchmark_communicator(), MPI_STATUS_IGNORE);
        size_t coll_rcount;
        MPI_Recv(&coll_rcount, 1, MPI_SIZE_T, 0, MSG_TAG_COLL_RCOUNT, icmb_benchmark_communicator(), MPI_STATUS_IGNORE);
        size_t mock_rcount;
        MPI_Recv(&mock_rcount, 1, MPI_SIZE_T, 0, MSG_TAG_MOCK_RCOUNT, icmb_benchmark_communicator(), MPI_STATUS_IGNORE);

        // gather send and receive buffers from responder processes
        test_type* send_buffer = (test_type*)malloc(coll_params.remote_size * coll_scount * coll_params.datatype_extent);
        MPI_Gather(NULL, 0, 0, send_buffer,  coll_scount * coll_params.datatype_extent, MPI_CHAR, MPI_ROOT, icmb_benchmark_communicator());

        test_type* recv_buffer = (test_type*)malloc(coll_params.remote_size * coll_rcount * coll_params.datatype_extent);
        MPI_Gather(NULL, 0, 0, recv_buffer, coll_rcount * coll_params.datatype_extent, MPI_CHAR, MPI_ROOT, icmb_benchmark_communicator());

        test_type* mockup_recv_buffer = (test_type*)malloc(mockup_params.remote_size * mock_rcount * mockup_params.datatype_extent);
        MPI_Gather(NULL, 0, 0, mockup_recv_buffer, mock_rcount * mockup_params.datatype_extent, MPI_CHAR, MPI_ROOT, icmb_benchmark_communicator());

        if (strcmp(coll1, "MPI_Bcast"))
        {
            // this is NOT bcast (strcmp returns 0=false if strings match)
            for (p=0; p<coll_params.remote_size; p++)
            {
                printf ("=========== Process Responder %d\n", p);
                print_buffers(coll1, coll2, recv_buffer + p * coll_rcount, mockup_recv_buffer + p * coll_rcount, coll_rcount);
            }
            error = (!identical(recv_buffer, mockup_recv_buffer, coll_params.remote_size * coll_rcount));
        }
        else
        {
            // for Bcast check source buffers
            for (p=0; p<coll_params.remote_size; p++) {
                printf ("=========== Process Responder %d\n", p);
                print_buffers(coll1, coll2, send_buffer + p * coll_scount, mockup_recv_buffer + p * coll_scount, coll_scount);
            }
            error = (!identical(send_buffer, mockup_recv_buffer, coll_params.remote_size * coll_scount));
        }

        free(send_buffer);
        free(recv_buffer);
        free(mockup_recv_buffer);
    }

    return error;
}

void check_results(char coll1[], char coll2[], collective_params_t coll_params, collective_params_t mockup_params, int check_only_at_root)
{
    int error = 0;

    if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC))
    {
        printf ("----------------------------------------\n");
        printf ("---------------- Comparing functions %s and %s\n", coll1, coll2);
    }

    if (!icmb_is_intercommunicator())
    {
        error = check_results_for_intracommunicator(coll1, coll2, coll_params, mockup_params, check_only_at_root);
    }
    else{
        if (icmb_is_initiator())
        {
            error = check_results_for_initiators(coll1, coll2, coll_params, mockup_params, check_only_at_root);
        }
        error |= check_results_for_responders(coll1, coll2, coll_params, mockup_params, check_only_at_root);
    }

    if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC))
    {
        if (error) {
            printf ("****************\n**************** TEST FAILED for %s and %s\n", coll1, coll2);
            printf("****************\n****************\n\n");
        }
        else {
            printf ("---- Test passed.\n\n");
        }
    }
}


void test_collective(basic_collective_params_t basic_coll_info, long count, int coll_index, int mockup_index)
{

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
    check_results(get_call_from_index(coll_index), get_call_from_index(mockup_index), coll_params, mockup_params, check_only_at_root);

    // cleanup data
    collective_calls[coll_index].cleanup_data(&coll_params);
    collective_calls[mockup_index].cleanup_data(&mockup_params);
}




int main(int argc, char* argv[])
{
    basic_collective_params_t basic_coll_info;
    long count = 100;

    srand(1000);

    /* start up MPI
     *
     * */
    MPI_Init(&argc, &argv);

    // parse command line options to launch inter-communicators
    icmb_parse_intercommunication_options(argc, argv);

    parse_test_options(argc, argv, &count);

    basic_coll_info.datatype = MPI_DOUBLE;
    basic_coll_info.op = MPI_SUM;
    basic_coll_info.root = icmb_collective_root(OUTPUT_ROOT_PROC);

    //test_collective(basic_coll_info, count, MPI_ALLGATHER, GL_ALLGATHER_AS_ALLREDUCE);
    //test_collective(basic_coll_info, count, MPI_ALLGATHER, GL_ALLGATHER_AS_ALLTOALL);
    //test_collective(basic_coll_info, count, MPI_ALLGATHER, GL_ALLGATHER_AS_GATHERBCAST);

    //test_collective(basic_coll_info, count, MPI_ALLREDUCE, GL_ALLREDUCE_AS_REDUCEBCAST);
    //test_collective(basic_coll_info, count, MPI_ALLREDUCE, GL_ALLREDUCE_AS_REDUCESCATTERALLGATHERV);
    //test_collective(basic_coll_info, count, MPI_ALLREDUCE, GL_ALLREDUCE_AS_REDUCESCATTERBLOCKALLGATHER);

    //test_collective(basic_coll_info, count, MPI_BCAST, GL_BCAST_AS_SCATTERALLGATHER);

    //test_collective(basic_coll_info, count, MPI_GATHER, GL_GATHER_AS_ALLGATHER);
    test_collective(basic_coll_info, count, MPI_GATHER, GL_GATHER_AS_REDUCE);

//     test_collective(basic_coll_info, count, MPI_REDUCE, GL_REDUCE_AS_ALLREDUCE);

//     test_collective(basic_coll_info, count, MPI_REDUCE, GL_REDUCE_AS_REDUCESCATTERGATHERV);

//     test_collective(basic_coll_info, count, MPI_REDUCE_SCATTER, GL_REDUCESCATTER_AS_ALLREDUCE);
//     test_collective(basic_coll_info, count, MPI_REDUCE_SCATTER, GL_REDUCESCATTER_AS_REDUCESCATTERV);
//     test_collective(basic_coll_info, count, MPI_REDUCE_SCATTER_BLOCK, GL_REDUCESCATTERBLOCK_AS_REDUCESCATTER);

//     test_collective(basic_coll_info, count, MPI_SCAN, GL_SCAN_AS_EXSCANREDUCELOCAL);

//     test_collective(basic_coll_info, count, MPI_SCATTER, GL_SCATTER_AS_BCAST);


//     int nprocs;
//     MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
//     if (count % nprocs == 0) {  // only works if the number of processes is a divisor of count
//         test_collective(basic_coll_info, count, MPI_REDUCE, GL_REDUCE_AS_REDUCESCATTERBLOCKGATHER);
//         test_collective(basic_coll_info, count, MPI_ALLREDUCE, GL_ALLREDUCE_AS_REDUCESCATTERBLOCKALLGATHER);
//     }
//     else {
//
//     }

    /* shut down MPI */
    MPI_Finalize();

    return 0;
}
