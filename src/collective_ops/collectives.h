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

#ifndef COLLECTIVES_H_
#define COLLECTIVES_H_

#include "reprompi_bench/option_parser/parse_common_options.h"

enum {
    MPI_ALLGATHER = 0,
    MPI_ALLREDUCE,
    MPI_ALLTOALL,
    MPI_BARRIER,
    MPI_BCAST,
    MPI_EXSCAN,
    MPI_GATHER,
    MPI_REDUCE,
    MPI_REDUCE_SCATTER,
    MPI_REDUCE_SCATTER_BLOCK,
    MPI_SCAN,
    MPI_SCATTER,
    GL_ALLGATHER_AS_ALLREDUCE,
    GL_ALLGATHER_AS_ALLTOALL,
    GL_ALLGATHER_AS_GATHERBCAST,
    GL_ALLREDUCE_AS_REDUCEBCAST,
//    GL_ALLREDUCE_AS_REDUCESCATTERALLGATHER,
    GL_ALLREDUCE_AS_REDUCESCATTERALLGATHERV,
    GL_ALLREDUCE_AS_REDUCESCATTERBLOCKALLGATHER,
    GL_BCAST_AS_SCATTERALLGATHER,
    GL_GATHER_AS_ALLGATHER,
    GL_GATHER_AS_REDUCE,
    GL_REDUCE_AS_ALLREDUCE,
//    GL_REDUCE_AS_REDUCESCATTERGATHER,
    GL_REDUCE_AS_REDUCESCATTERGATHERV,
    GL_REDUCE_AS_REDUCESCATTERBLOCKGATHER,
    GL_REDUCESCATTER_AS_ALLREDUCE,
    GL_REDUCESCATTER_AS_REDUCESCATTERV,
    GL_REDUCESCATTERBLOCK_AS_REDUCESCATTER,
    GL_SCAN_AS_EXSCANREDUCELOCAL,
    GL_SCATTER_AS_BCAST,
    PINGPONG_SEND_RECV,
    PINGPONG_SENDRECV,
    PINGPONG_ISEND_RECV,
    PINGPONG_ISEND_IRECV,
    PINGPONG_SEND_IRECV,
    BBARRIER,
    EMPTY,
    N_MPI_CALLS         // number of calls
};


typedef struct collparams {
    size_t count;
    char* sbuf;
    char* rbuf;
    char* tmp_buf;
    int root;
    MPI_Datatype datatype;
    MPI_Aint datatype_extent;
    MPI_Op op;
    int is_intercommunicator;
    int is_initiator;
    int is_responder;
    int initiator_size;
    int local_size;
    int remote_size;
    int responder_size;
    int larger_size;
    int global_size;
    MPI_Comm communicator;
    int rank;
    size_t scount;
    size_t rcount;
    size_t tcount;
    int* counts_array;
    int* displ_array;

    // parameters relevant for ping-pong operations
    int pingpong_ranks[2];
} collective_params_t;


typedef struct basic_collparams {
    int root;
    MPI_Datatype datatype;
    MPI_Op op;

    // parameters relevant for ping-pong operations
    int pingpong_ranks[2];
} basic_collective_params_t;



typedef void (*collective_call_t)(collective_params_t* params);
typedef void (*initialize_data_t)(const basic_collective_params_t info, const long count, collective_params_t* params);
typedef void (*cleanup_data_t)(collective_params_t* params);

typedef struct coll_op {
    collective_call_t collective_call;
    initialize_data_t initialize_data;
    cleanup_data_t cleanup_data;
} collective_ops_t;

int get_call_index(char* name);
char* get_call_from_index(int index);
char* const* get_mpi_calls_list(void);

extern const collective_ops_t collective_calls[];

/*
 * procs argument is not used with inter-communicators because it is ambiguous
 *
 * instead, the implementation must either use the number of processes in the
 * local group or the number of processes in the remote group
 */
void init_collective_basic_info(reprompib_common_options_t opts, int procs, basic_collective_params_t* coll_basic_info);

void execute_Allgather(collective_params_t* params);
void execute_Allreduce(collective_params_t* params);
void execute_Alltoall(collective_params_t* params);
void execute_Barrier(collective_params_t* params);
void execute_Bcast(collective_params_t* params);
void execute_Exscan(collective_params_t* params);
void execute_Gather(collective_params_t* params);
void execute_Reduce(collective_params_t* params);
void execute_Reduce_scatter(collective_params_t* params);
void execute_Reduce_scatter_block(collective_params_t* params);
void execute_Scan(collective_params_t* params);
void execute_Scatter(collective_params_t* params);

void execute_BBarrier(collective_params_t* params);
void execute_Empty(collective_params_t* params);

// Mockup functions to measure for MPI guidelines
void execute_GL_Allgather_as_Allreduce(collective_params_t* params);
void execute_GL_Allgather_as_Alltoall(collective_params_t* params);
void execute_GL_Allgather_as_GatherBcast(collective_params_t* params);
void execute_GL_Allreduce_as_ReduceBcast(collective_params_t* params);
//void execute_GL_Allreduce_as_ReducescatterAllgather(collective_params_t* params);
void execute_GL_Allreduce_as_ReducescatterAllgatherv(collective_params_t* params);
void execute_GL_Allreduce_as_ReducescatterblockAllgather(collective_params_t* params);
void execute_GL_Bcast_as_ScatterAllgather(collective_params_t* params);
void execute_GL_Gather_as_Allgather(collective_params_t* params);
void execute_GL_Gather_as_Reduce(collective_params_t* params);
void execute_GL_Reduce_as_Allreduce(collective_params_t* params);
//void execute_GL_Reduce_as_ReducescatterGather(collective_params_t* params);
void execute_GL_Reduce_as_ReducescatterGatherv(collective_params_t* params);
void execute_GL_Reduce_as_ReducescatterblockGather(collective_params_t* params);
void execute_GL_Reduce_scatter_as_Allreduce(collective_params_t* params);
void execute_GL_Reduce_scatter_as_ReduceScatterv(collective_params_t* params);
void execute_GL_Reduce_scatter_block_as_ReduceScatter(collective_params_t* params);
void execute_GL_Scan_as_ExscanReducelocal(collective_params_t* params);
void execute_GL_Scatter_as_Bcast(collective_params_t* params);


// pingpong operations
void execute_pingpong_Send_Recv(collective_params_t* params);
void execute_pingpong_Isend_Recv(collective_params_t* params);
void execute_pingpong_Isend_Irecv(collective_params_t* params);
void execute_pingpong_Send_Irecv(collective_params_t* params);
void execute_pingpong_Sendrecv(collective_params_t* params);



// buffer initialization functions
void initialize_common_data(const basic_collective_params_t info,
        collective_params_t* params);
void initialize_data_default(const basic_collective_params_t info, const long count, collective_params_t* params);

void initialize_data_Allgather(const basic_collective_params_t info, const long count, collective_params_t* params);
void initialize_data_Alltoall(const basic_collective_params_t info, const long count, collective_params_t* params);
void initialize_data_Bcast(const basic_collective_params_t info, const long count, collective_params_t* params);
void initialize_data_Gather(const basic_collective_params_t info, const long count, collective_params_t* params);
void initialize_data_Reduce_scatter(const basic_collective_params_t info, const long count, collective_params_t* params);
void initialize_data_Reduce_scatter_block(const basic_collective_params_t info, const long count, collective_params_t* params);
void initialize_data_Scatter(const basic_collective_params_t info, const long count, collective_params_t* params);

void initialize_data_GL_Allgather_as_Allreduce(const basic_collective_params_t info, const long count, collective_params_t* params);
void initialize_data_GL_Allgather_as_Alltoall(const basic_collective_params_t info, const long count, collective_params_t* params);
void initialize_data_GL_Allgather_as_GatherBcast(const basic_collective_params_t info, const long count, collective_params_t* params);
void initialize_data_GL_Allreduce_as_ReduceBcast(const basic_collective_params_t info, const long count, collective_params_t* params);
//void initialize_data_GL_Allreduce_as_ReducescatterAllgather(const basic_collective_params_t info, const long count, collective_params_t* params);
void initialize_data_GL_Allreduce_as_ReducescatterAllgatherv(const basic_collective_params_t info, const long count, collective_params_t* params);
void initialize_data_GL_Allreduce_as_ReducescatterblockAllgather(const basic_collective_params_t info, const long count, collective_params_t* params);
void initialize_data_GL_Bcast_as_ScatterAllgather(const basic_collective_params_t info, const long count, collective_params_t* params);
void initialize_data_GL_Gather_as_Allgather(const basic_collective_params_t info, const long count, collective_params_t* params);
void initialize_data_GL_Gather_as_Reduce(const basic_collective_params_t info, const long count, collective_params_t* params);
void initialize_data_GL_Reduce_as_Allreduce(const basic_collective_params_t info, const long count, collective_params_t* params);
//void initialize_data_GL_Reduce_as_ReducescatterGather(const basic_collective_params_t info, const long count, collective_params_t* params);
void initialize_data_GL_Reduce_as_ReducescatterGatherv(const basic_collective_params_t info, const long count, collective_params_t* params);
void initialize_data_GL_Reduce_as_ReducescatterblockGather(const basic_collective_params_t info, const long count, collective_params_t* params);
void initialize_data_GL_Reduce_scatter_as_Allreduce(const basic_collective_params_t info, const long count, collective_params_t* params);
void initialize_data_GL_Reduce_scatter_as_ReduceScatterv(const basic_collective_params_t info, const long count, collective_params_t* params);
void initialize_data_GL_Reduce_scatter_block_as_ReduceScatter(const basic_collective_params_t info, const long count, collective_params_t* params);
void initialize_data_GL_Scan_as_ExscanReducelocal(const basic_collective_params_t info, const long count, collective_params_t* params);
void initialize_data_GL_Scatter_as_Bcast(const basic_collective_params_t info, const long count, collective_params_t* params);

// buffer initialization for pingpongs
void initialize_data_pingpong(const basic_collective_params_t info, const long count, collective_params_t* params);


// buffer cleanup functions
void cleanup_data_default(collective_params_t* params);


void cleanup_data_Allgather(collective_params_t* params);
void cleanup_data_Alltoall(collective_params_t* params);
void cleanup_data_Bcast(collective_params_t* params);
void cleanup_data_Gather(collective_params_t* params);
void cleanup_data_Reduce_scatter(collective_params_t* params);
void cleanup_data_Reduce_scatter_block(collective_params_t* params);
void cleanup_data_Scatter(collective_params_t* params);

void cleanup_data_GL_Allgather_as_Allreduce(collective_params_t* params);
void cleanup_data_GL_Allgather_as_Alltoall(collective_params_t* params);
void cleanup_data_GL_Allgather_as_GatherBcast(collective_params_t* params);
void cleanup_data_GL_Allreduce_as_ReduceBcast(collective_params_t* params);
//void cleanup_data_GL_Allreduce_as_ReducescatterAllgather(collective_params_t* params);
void cleanup_data_GL_Allreduce_as_ReducescatterAllgatherv(collective_params_t* params);
void cleanup_data_GL_Allreduce_as_ReducescatterblockAllgather(collective_params_t* params);
void cleanup_data_GL_Bcast_as_ScatterAllgather(collective_params_t* params);
void cleanup_data_GL_Gather_as_Allgather(collective_params_t* params);
void cleanup_data_GL_Gather_as_Reduce(collective_params_t* params);
void cleanup_data_GL_Reduce_as_Allreduce(collective_params_t* params);
//void cleanup_data_GL_Reduce_as_ReducescatterGather(collective_params_t* params);
void cleanup_data_GL_Reduce_as_ReducescatterGatherv(collective_params_t* params);
void cleanup_data_GL_Reduce_as_ReducescatterblockGather(collective_params_t* params);
void cleanup_data_GL_Reduce_scatter_as_Allreduce(collective_params_t* params);
void cleanup_data_GL_Reduce_scatter_as_ReduceScatterv(collective_params_t* params);
void cleanup_data_GL_Reduce_scatter_block_as_ReduceScatter(collective_params_t* params);
void cleanup_data_GL_Scan_as_ExscanReducelocal(collective_params_t* params);
void cleanup_data_GL_Scatter_as_Bcast(collective_params_t* params);


// buffer initialization for pingpongs
void cleanup_data_pingpong(collective_params_t* params);


#endif /* COLLECTIVES_H_ */


