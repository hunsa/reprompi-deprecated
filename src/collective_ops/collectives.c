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

// allow strdup with c99
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include "mpi.h"
#include "reprompi_bench/option_parser/parse_common_options.h"
#include "reprompi_bench/sync/benchmark_barrier_sync/bbarrier_sync.h"
#include "buf_manager/mem_allocation.h"
#include "collectives.h"

#include "contrib/intercommunication/intercommunication.h"

const collective_ops_t collective_calls[] = {
        [MPI_ALLGATHER] = {
                &execute_Allgather,
                &initialize_data_Allgather,
                &cleanup_data_Allgather
        },
        [MPI_ALLREDUCE] = {
                &execute_Allreduce,
                &initialize_data_default,
                &cleanup_data_default
        },
        [MPI_ALLTOALL] = {
                &execute_Alltoall,
                &initialize_data_Alltoall,
                &cleanup_data_Alltoall
        },
        [MPI_BARRIER] = {
                &execute_Barrier,
                &initialize_data_default,
                &cleanup_data_default
        },
        [MPI_BCAST] =  {
                &execute_Bcast,
                &initialize_data_default,
                &cleanup_data_default
        },
        [MPI_EXSCAN] = {
                &execute_Exscan,
                &initialize_data_default,
                &cleanup_data_default
        },
        [MPI_GATHER] = {
                &execute_Gather,
                &initialize_data_Gather,
                &cleanup_data_Gather
        },
        [MPI_REDUCE] = {
                &execute_Reduce,
                &initialize_data_default,
                &cleanup_data_default
        },
        [MPI_REDUCE_SCATTER] = {
                &execute_Reduce_scatter,
                &initialize_data_Reduce_scatter,
                &cleanup_data_Reduce_scatter
        },
        [MPI_REDUCE_SCATTER_BLOCK] = {
                &execute_Reduce_scatter_block,
                &initialize_data_Reduce_scatter_block,
                &cleanup_data_Reduce_scatter_block
        },
        [MPI_SCAN] = {
                &execute_Scan,
                &initialize_data_default,
                &cleanup_data_default
        },
        [MPI_SCATTER] = {
                &execute_Scatter,
                &initialize_data_Scatter,
                &cleanup_data_Scatter
        },
        [GL_ALLGATHER_AS_ALLREDUCE] = {
                &execute_GL_Allgather_as_Allreduce,
                &initialize_data_GL_Allgather_as_Allreduce,
                &cleanup_data_GL_Allgather_as_Allreduce
        },
        [GL_ALLGATHER_AS_ALLTOALL] = {
                &execute_GL_Allgather_as_Alltoall,
                &initialize_data_GL_Allgather_as_Alltoall,
                &cleanup_data_GL_Allgather_as_Alltoall
        },
        [GL_ALLGATHER_AS_GATHERBCAST] = {
                &execute_GL_Allgather_as_GatherBcast,
                &initialize_data_GL_Allgather_as_GatherBcast,
                &cleanup_data_GL_Allgather_as_GatherBcast
        },
        [GL_ALLREDUCE_AS_REDUCEBCAST] = {
                &execute_GL_Allreduce_as_ReduceBcast,
                &initialize_data_GL_Allreduce_as_ReduceBcast,
                &cleanup_data_GL_Allreduce_as_ReduceBcast
        },
//        [GL_ALLREDUCE_AS_REDUCESCATTERALLGATHER] = {
//                &execute_GL_Allreduce_as_ReducescatterAllgather,
//                &initialize_data_GL_Allreduce_as_ReducescatterAllgather,
//                &cleanup_data_GL_Allreduce_as_ReducescatterAllgather
//        },
        [GL_ALLREDUCE_AS_REDUCESCATTERALLGATHERV] = {
                &execute_GL_Allreduce_as_ReducescatterAllgatherv,
                &initialize_data_GL_Allreduce_as_ReducescatterAllgatherv,
                &cleanup_data_GL_Allreduce_as_ReducescatterAllgatherv
        },
        [GL_ALLREDUCE_AS_REDUCESCATTERBLOCKALLGATHER] = {
                &execute_GL_Allreduce_as_ReducescatterblockAllgather,
                &initialize_data_GL_Allreduce_as_ReducescatterblockAllgather,
                &cleanup_data_GL_Allreduce_as_ReducescatterblockAllgather
        },
        [GL_BCAST_AS_SCATTERALLGATHER] = {
                &execute_GL_Bcast_as_ScatterAllgather,
                &initialize_data_GL_Bcast_as_ScatterAllgather,
                &cleanup_data_GL_Bcast_as_ScatterAllgather
        },
        [GL_GATHER_AS_ALLGATHER] = {
                &execute_GL_Gather_as_Allgather,
                &initialize_data_GL_Gather_as_Allgather,
                &cleanup_data_GL_Gather_as_Allgather
        },
        [GL_GATHER_AS_REDUCE] = {
                &execute_GL_Gather_as_Reduce,
                &initialize_data_GL_Gather_as_Reduce,
                &cleanup_data_GL_Gather_as_Reduce
        },
        [GL_REDUCE_AS_ALLREDUCE] = {
                &execute_GL_Reduce_as_Allreduce,
                &initialize_data_GL_Reduce_as_Allreduce,
                &cleanup_data_GL_Reduce_as_Allreduce
        },
//        [GL_REDUCE_AS_REDUCESCATTERGATHER] = {
//                &execute_GL_Reduce_as_ReducescatterGather,
//                &initialize_data_GL_Reduce_as_ReducescatterGather,
//                &cleanup_data_GL_Reduce_as_ReducescatterGather
//        },
        [GL_REDUCE_AS_REDUCESCATTERGATHERV] = {
                &execute_GL_Reduce_as_ReducescatterGatherv,
                &initialize_data_GL_Reduce_as_ReducescatterGatherv,
                &cleanup_data_GL_Reduce_as_ReducescatterGatherv
        },
        [GL_REDUCE_AS_REDUCESCATTERBLOCKGATHER] = {
                &execute_GL_Reduce_as_ReducescatterblockGather,
                &initialize_data_GL_Reduce_as_ReducescatterblockGather,
                &cleanup_data_GL_Reduce_as_ReducescatterblockGather
        },
        [GL_REDUCESCATTER_AS_ALLREDUCE] = {
                &execute_GL_Reduce_scatter_as_Allreduce,
                &initialize_data_GL_Reduce_scatter_as_Allreduce,
                &cleanup_data_GL_Reduce_scatter_as_Allreduce
        },
        [GL_REDUCESCATTER_AS_REDUCESCATTERV] = {
                &execute_GL_Reduce_scatter_as_ReduceScatterv,
                &initialize_data_GL_Reduce_scatter_as_ReduceScatterv,
                &cleanup_data_GL_Reduce_scatter_as_ReduceScatterv
        },
        [GL_REDUCESCATTERBLOCK_AS_REDUCESCATTER] = {
                &execute_GL_Reduce_scatter_block_as_ReduceScatter,
                &initialize_data_GL_Reduce_scatter_block_as_ReduceScatter,
                &cleanup_data_GL_Reduce_scatter_block_as_ReduceScatter
        },
        [GL_SCAN_AS_EXSCANREDUCELOCAL] = {
                &execute_GL_Scan_as_ExscanReducelocal,
                &initialize_data_GL_Scan_as_ExscanReducelocal,
                &cleanup_data_GL_Scan_as_ExscanReducelocal
        },
        [GL_SCATTER_AS_BCAST] = {
                &execute_GL_Scatter_as_Bcast,
                &initialize_data_GL_Scatter_as_Bcast,
                &cleanup_data_GL_Scatter_as_Bcast
        },
        [PINGPONG_SEND_RECV] = {
                &execute_pingpong_Send_Recv,
                &initialize_data_pingpong,
                &cleanup_data_pingpong
        },
        [PINGPONG_SENDRECV] = {
                &execute_pingpong_Sendrecv,
                &initialize_data_pingpong,
                &cleanup_data_pingpong
        },
        [PINGPONG_ISEND_RECV] = {
                &execute_pingpong_Isend_Recv,
                &initialize_data_pingpong,
                &cleanup_data_pingpong
        },
        [PINGPONG_ISEND_IRECV] = {
                &execute_pingpong_Isend_Irecv,
                &initialize_data_pingpong,
                &cleanup_data_pingpong
        },
        [PINGPONG_SEND_IRECV] = {
                &execute_pingpong_Send_Irecv,
                &initialize_data_pingpong,
                &cleanup_data_pingpong
        },
        [BBARRIER] = {
                &execute_BBarrier,
                &initialize_data_default,
                &cleanup_data_default
        },
        [EMPTY] = {
                &execute_Empty,
                &initialize_data_default,
                &cleanup_data_default
        }
};


static char* const mpi_calls_opts[] = {
        [MPI_ALLGATHER] = "MPI_Allgather",
        [MPI_ALLREDUCE] = "MPI_Allreduce",
        [MPI_ALLTOALL] = "MPI_Alltoall",
        [MPI_BARRIER] = "MPI_Barrier",
        [MPI_BCAST] =  "MPI_Bcast",
        [MPI_EXSCAN] = "MPI_Exscan",
        [MPI_GATHER] = "MPI_Gather",
        [MPI_REDUCE] = "MPI_Reduce",
        [MPI_REDUCE_SCATTER] = "MPI_Reduce_scatter",
        [MPI_REDUCE_SCATTER_BLOCK] = "MPI_Reduce_scatter_block",
        [MPI_SCAN] = "MPI_Scan",
        [MPI_SCATTER] = "MPI_Scatter",
        [GL_ALLGATHER_AS_ALLREDUCE] = "GL_Allgather_as_Allreduce",
        [GL_ALLGATHER_AS_ALLTOALL] = "GL_Allgather_as_Alltoall",
        [GL_ALLGATHER_AS_GATHERBCAST] = "GL_Allgather_as_GatherBcast",
        [GL_ALLREDUCE_AS_REDUCEBCAST] = "GL_Allreduce_as_ReduceBcast",
//        [GL_ALLREDUCE_AS_REDUCESCATTERALLGATHER] = "GL_Allreduce_as_ReducescatterAllgather",
        [GL_ALLREDUCE_AS_REDUCESCATTERALLGATHERV] = "GL_Allreduce_as_ReducescatterAllgatherv",
        [GL_ALLREDUCE_AS_REDUCESCATTERBLOCKALLGATHER] = "GL_Allreduce_as_ReducescatterblockAllgather",
        [GL_BCAST_AS_SCATTERALLGATHER] = "GL_Bcast_as_ScatterAllgather",
        [GL_GATHER_AS_ALLGATHER] = "GL_Gather_as_Allgather",
        [GL_GATHER_AS_REDUCE] = "GL_Gather_as_Reduce",
        [GL_REDUCE_AS_ALLREDUCE] = "GL_Reduce_as_Allreduce",
//        [GL_REDUCE_AS_REDUCESCATTERGATHER] = "GL_Reduce_as_ReducescatterGather",
        [GL_REDUCE_AS_REDUCESCATTERGATHERV] = "GL_Reduce_as_ReducescatterGatherv",
        [GL_REDUCE_AS_REDUCESCATTERBLOCKGATHER] = "GL_Reduce_as_ReducescatterblockGather",
        [GL_REDUCESCATTER_AS_ALLREDUCE] = "GL_Reduce_scatter_as_Allreduce",
        [GL_REDUCESCATTER_AS_REDUCESCATTERV] = "GL_Reduce_scatter_as_ReduceScatterv",
        [GL_REDUCESCATTERBLOCK_AS_REDUCESCATTER] = "GL_Reduce_scatter_block_as_ReduceScatter",
        [GL_SCAN_AS_EXSCANREDUCELOCAL] = "GL_Scan_as_ExscanReducelocal",
        [GL_SCATTER_AS_BCAST] = "GL_Scatter_as_Bcast",
        [PINGPONG_SEND_RECV] = "Send_Recv",
        [PINGPONG_SENDRECV] = "Sendrecv",
        [PINGPONG_ISEND_RECV] = "Isend_Recv",
        [PINGPONG_ISEND_IRECV] = "Isend_Irecv",
        [PINGPONG_SEND_IRECV] = "Send_Irecv",
        [BBARRIER] = "BBarrier",
        [EMPTY] = "Empty",
        NULL
};

char* const* get_mpi_calls_list(void) {

    return &(mpi_calls_opts[0]);
}

int get_call_index(char* name) {
    int index = -1;
    int i;

    for (i=0; i< N_MPI_CALLS; i++) {
        if (strcmp(name, mpi_calls_opts[i]) == 0) {
            index = i;
            break;
        }
    }

    return index;
}



char* get_call_from_index(int index) {
    if (index < 0 || index >= N_MPI_CALLS) {
        return "";
    }
    return strdup(mpi_calls_opts[index]);
}



inline void execute_BBarrier(collective_params_t* params) {
    dissemination_barrier();
}


inline void execute_Empty(collective_params_t* params) {
}



void initialize_data_default(const basic_collective_params_t info, const long count,
        collective_params_t* params) {
    initialize_common_data(info, params);

    params->count = count;

    params->scount = count;
    params->rcount = count;
    params->tcount = 0;

    assert (params->scount < INT_MAX);
    assert (params->rcount < INT_MAX);

    params->sbuf = (char*)reprompi_calloc(params->scount, params->datatype_extent);
    params->rbuf = (char*)reprompi_calloc(params->rcount, params->datatype_extent);
    memset(params->sbuf, 0, params->scount * params->datatype_extent);
    memset(params->rbuf, 0, params->rcount * params->datatype_extent);

}


void cleanup_data_default(collective_params_t* params) {
    free(params->sbuf);
    free(params->rbuf);
    params->sbuf = NULL;
    params->rbuf = NULL;
}


void initialize_common_data(const basic_collective_params_t info,
        collective_params_t* params) {

    MPI_Aint lb;

    MPI_Type_get_extent(info.datatype, &lb, &(params->datatype_extent));
    params->datatype = info.datatype;

    params->op = info.op;

    params->is_intercommunicator = icmb_is_intercommunicator();
    params->is_initiator = icmb_is_initiator();
    params->is_responder = icmb_is_responder();
    params->communicator = icmb_benchmark_communicator();
    params->initiator_size = icmb_initiator_size();
    params->local_size = icmb_local_size();
    params->remote_size = icmb_remote_size();
    params->responder_size = icmb_responder_size();
    params->larger_size = icmb_larger_size();

    params->rank = icmb_benchmark_rank();

    params->root = info.root;

    params->pingpong_ranks[0] = info.pingpong_ranks[0];
    params->pingpong_ranks[1] = info.pingpong_ranks[1];

    params->sbuf = NULL;
    params->rbuf = NULL;
    params->tmp_buf = NULL;
    params->counts_array = NULL;
    params->displ_array = NULL;
}


/*
 * procs argument is not used with inter-communicators because it is ambiguous
 *
 * instead, the implementation must either use the number of processes in the
 * local group or the number of processes in the remote group, depending on
 * context
 */
void init_collective_basic_info(reprompib_common_options_t opts, int procs, basic_collective_params_t* coll_basic_info) {
    // initialize common collective calls information
    coll_basic_info->datatype = opts.datatype;
    coll_basic_info->op = opts.operation;
    coll_basic_info->root = 0;

    coll_basic_info->pingpong_ranks[0] = opts.pingpong_ranks[0];
    coll_basic_info->pingpong_ranks[1] = opts.pingpong_ranks[1];

    if (opts.root_proc >= 0 && opts.root_proc < icmb_initiator_size()) {
        coll_basic_info->root = opts.root_proc;
    }

    // adjust root for inter-communicator collectives
    coll_basic_info->root = icmb_collective_root(coll_basic_info->root);
}
