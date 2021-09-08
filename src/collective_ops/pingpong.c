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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <assert.h>
#include <limits.h>
#include "mpi.h"
#include "buf_manager/mem_allocation.h"
#include "collectives.h"

#include "contrib/intercommunication/intercommunication.h"

static const int TAG = 1;

/***************************************/
// MPI_Send + MPI_Recv
inline void execute_pingpong_Send_Recv(collective_params_t* params) {
  int src_rank, dest_rank;
  MPI_Status stat;

  src_rank = params->pingpong_ranks[0];
  dest_rank = params->pingpong_ranks[1];
  if (!params->is_intercommunicator)
  {
    assert(src_rank != dest_rank);
  }

  if (params->is_initiator && params->rank == src_rank) {
    MPI_Send(params->sbuf, params->count, params->datatype, dest_rank, TAG, params->communicator);
    MPI_Recv(params->rbuf, params->count, params->datatype, dest_rank, TAG, params->communicator, &stat);

  } else if (params->is_responder && params->rank == dest_rank) {
    MPI_Recv(params->rbuf, params->count, params->datatype, src_rank, TAG, params->communicator, &stat);
    MPI_Send(params->sbuf, params->count, params->datatype, src_rank, TAG, params->communicator);
  }
}

/***************************************/
// MPI_Isend + MPI_Recv
inline void execute_pingpong_Isend_Recv(collective_params_t* params) {
  int src_rank, dst_rank;
  int recv_rank;
  int flag = 0;
  MPI_Status stat;
  MPI_Request req;

  src_rank  = params->pingpong_ranks[0];
  dst_rank = params->pingpong_ranks[1];
  if (!params->is_intercommunicator)
  {
    assert(src_rank != dst_rank);
  }

  if (params->is_initiator && params->rank == src_rank) {
    recv_rank = dst_rank;
    flag = 1;
  } else if(params->is_responder && params->rank == dst_rank ) {
    recv_rank = src_rank;
    flag = 1;
  }

  if( flag == 1 ) {
    MPI_Isend(params->sbuf, params->count, params->datatype, recv_rank, TAG, params->communicator, &req);
    MPI_Recv(params->rbuf, params->count, params->datatype, recv_rank, TAG, params->communicator, &stat);
    MPI_Wait(&req, &stat);
  }
}

/***************************************/
// MPI_Isend + MPI_Irecv
inline void execute_pingpong_Isend_Irecv(collective_params_t* params) {
  int src_rank, dst_rank;
  const int nreqs = 2;
  int recv_rank;
  int flag = 0;
  MPI_Request reqs[nreqs];
  MPI_Status stats[nreqs];

  src_rank = params->pingpong_ranks[0];
  dst_rank = params->pingpong_ranks[1];
  if (!params->is_intercommunicator)
  {
    assert(src_rank != dst_rank);
  }

  if (params->is_initiator && params->rank == src_rank) {
    recv_rank = dst_rank;
    flag = 1;
  } else if(params->is_responder && params->rank == dst_rank ) {
    recv_rank = src_rank;
    flag = 1;
  }

  if( flag == 1 ) {
    MPI_Isend(params->sbuf, params->count, params->datatype, recv_rank, TAG, params->communicator, &reqs[0]);
    MPI_Irecv(params->rbuf, params->count, params->datatype, recv_rank, TAG, params->communicator, &reqs[1]);
    MPI_Waitall(nreqs, reqs, stats);
  }
}

/***************************************/
// MPI_Send + MPI_Irecv
inline void execute_pingpong_Send_Irecv(collective_params_t* params) {
  int src_rank, dst_rank;
  int recv_rank;
  int flag = 0;
  MPI_Request req;
  MPI_Status stat;

  src_rank  = params->pingpong_ranks[0];
  dst_rank  = params->pingpong_ranks[1];
  if (!params->is_intercommunicator)
  {
    assert(src_rank != dst_rank);
  }

  if (params->is_initiator && params->rank == src_rank) {
    recv_rank = dst_rank;
    flag = 1;
  } else if(params->is_responder && params->rank == dst_rank ) {
    recv_rank = src_rank;
    flag = 1;
  }

  if( flag == 1 ) {
    MPI_Irecv(params->rbuf, params->count, params->datatype, recv_rank, TAG, params->communicator, &req);
    MPI_Send(params->sbuf, params->count, params->datatype, recv_rank, TAG, params->communicator);
    MPI_Wait(&req, &stat);
  }
}

/***************************************/
// MPI_Sendrecv
inline void execute_pingpong_Sendrecv(collective_params_t* params) {
  int src_rank, dst_rank;
  int recv_rank;
  MPI_Status stat;
  int flag = 0;

  src_rank  = params->pingpong_ranks[0];
  dst_rank = params->pingpong_ranks[1];
  if (!params->is_intercommunicator)
  {
    assert(src_rank != dst_rank);
  }

  if (params->is_initiator && params->rank == src_rank) {
    recv_rank = dst_rank;
    flag = 1;
  } else if (params->is_responder && params->rank == dst_rank) {
    recv_rank = src_rank;
    flag = 1;
  }

  if( flag == 1 ) {
    MPI_Sendrecv(params->sbuf, params->count, params->datatype, recv_rank, TAG, params->rbuf, params->count,
        params->datatype, recv_rank, TAG,
        params->communicator, &stat);
  }

}

void initialize_data_pingpong(const basic_collective_params_t info, const long count, collective_params_t* params) {
  int i;
  int other_rank;
  int nranks = 2;

  initialize_common_data(info, params);

    if ((params->is_intercommunicator && (params->local_size < 1 || params->remote_size < 1))
        ||
        (!params->is_intercommunicator && params->local_size < nranks))
    {
        // abort pingpong if not enough processes
        icmb_error_and_exit("Cannot perform pingpong with only one process", ICMB_ERROR_NUM_PROCS);
    }

    if (params->pingpong_ranks[0] < 0 || params->pingpong_ranks[0] >= params->local_size) {
        icmb_error("Invalid ping rank (%d). Highest available rank is %d.", info.pingpong_ranks[0], params->local_size-1);
        icmb_exit(ICMB_ERROR_PING_TOO_HIGH);
    }

    if (params->pingpong_ranks[1] < 0 || params->pingpong_ranks[1] >= params->remote_size) {
        icmb_error("Invalid pong rank (%d). Highest available rank is %d.", info.pingpong_ranks[1], params->remote_size-1);
        icmb_exit(ICMB_ERROR_PONG_TOO_HIGH);
    }

    if (!params->is_intercommunicator && params->pingpong_ranks[0] == params->pingpong_ranks[1]) {
        icmb_error("Invalid pingpong ranks (%d, %d). Process can not pingpong itself.", params->pingpong_ranks[0], params->pingpong_ranks[1]);
        icmb_exit(ICMB_ERROR_PING_EQUALS_PONG);
    }


    // check whether the two ranks belong to different machines
    if ((params->is_initiator && params->rank == params->pingpong_ranks[0]) || (params->is_responder && params->rank == params->pingpong_ranks[1]))
    {
        char *local_proc_name, *other_proc_name;
        int local_proc_name_len, other_proc_name_len;
        MPI_Status stat;

        local_proc_name = (char*)calloc(MPI_MAX_PROCESSOR_NAME, sizeof(char));
        other_proc_name = (char*)calloc(MPI_MAX_PROCESSOR_NAME, sizeof(char));

        if (params->is_initiator && params->rank == params->pingpong_ranks[0]) {
            other_rank = info.pingpong_ranks[1];
        } else {
            other_rank = info.pingpong_ranks[0];
        }

        MPI_Get_processor_name(local_proc_name, &local_proc_name_len);

        MPI_Sendrecv(&local_proc_name_len, 1, MPI_INT, other_rank, TAG, &other_proc_name_len, 1, MPI_INT, other_rank, TAG, params->communicator, &stat);
        MPI_Sendrecv(local_proc_name, local_proc_name_len, MPI_CHAR, other_rank, TAG, other_proc_name, other_proc_name_len, MPI_CHAR, other_rank, TAG, params->communicator, &stat);

        // print warning if the two ranks are on the same node
        if (local_proc_name_len == other_proc_name_len && strcmp(local_proc_name, other_proc_name) == 0) {
            // print output directly, as process (initiator, 0) is not necessarily involved at this point
            if (params->is_initiator && params->rank == params->pingpong_ranks[0]) {
                fprintf(stderr, "Warning: The two ping-pong ranks (%d, %d) are running on the same node (%s)\n", info.pingpong_ranks[0], info.pingpong_ranks[1], local_proc_name);
            }
        }

        free(local_proc_name);
        free(other_proc_name);
    }


    initialize_common_data(info, params);

    params->count = count;
    params->scount = 0;
    params->rcount = 0;

    assert (params->count < INT_MAX);

    params->sbuf = (char*)reprompi_calloc(params->count, params->datatype_extent);
    params->rbuf = (char*)reprompi_calloc(params->count, params->datatype_extent);
}

void cleanup_data_pingpong(collective_params_t* params) {
  free(params->sbuf);
  free(params->rbuf);
  params->sbuf = NULL;
  params->rbuf = NULL;
}
/***************************************/

