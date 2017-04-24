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
#include <assert.h>
#include <assert.h>
#include <limits.h>
#include "mpi.h"
#include "buf_manager/mem_allocation.h"
#include "collectives.h"

static const int TAG = 1;

/***************************************/
// MPI_Send + MPI_Recv
inline void execute_pingpong_Send_Recv(collective_params_t* params) {
  int src_rank, dest_rank;
  MPI_Status stat;

  src_rank = params->pingpong_ranks[0];
  dest_rank = params->pingpong_ranks[1];
  assert(src_rank != dest_rank);

  if (params->rank == src_rank) {
    MPI_Send(params->sbuf, params->count, params->datatype, dest_rank, TAG, MPI_COMM_WORLD);
    MPI_Recv(params->rbuf, params->count, params->datatype, dest_rank, TAG, MPI_COMM_WORLD, &stat);

  } else if (params->rank == dest_rank) {
    MPI_Recv(params->rbuf, params->count, params->datatype, src_rank, TAG, MPI_COMM_WORLD, &stat);
    MPI_Send(params->sbuf, params->count, params->datatype, src_rank, TAG, MPI_COMM_WORLD);
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
  assert(src_rank != dst_rank);

  if (params->rank == src_rank) {
    recv_rank = dst_rank;
    flag = 1;
  } else if( params->rank == dst_rank ) {
    recv_rank = src_rank;
    flag = 1;
  }

  if( flag == 1 ) {
    MPI_Isend(params->sbuf, params->count, params->datatype, recv_rank, TAG, MPI_COMM_WORLD, &req);
    MPI_Recv(params->rbuf, params->count, params->datatype, recv_rank, TAG, MPI_COMM_WORLD, &stat);
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
  assert(src_rank != dst_rank);

  if (params->rank == src_rank) {
    recv_rank = dst_rank;
    flag = 1;
  } else if( params->rank == dst_rank ) {
    recv_rank = src_rank;
    flag = 1;
  }

  if( flag == 1 ) {
    MPI_Isend(params->sbuf, params->count, params->datatype, recv_rank, TAG, MPI_COMM_WORLD, &reqs[0]);
    MPI_Irecv(params->rbuf, params->count, params->datatype, recv_rank, TAG, MPI_COMM_WORLD, &reqs[1]);
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
  assert(src_rank != dst_rank);

  if (params->rank == src_rank) {
    recv_rank = dst_rank;
    flag = 1;
  } else if( params->rank == dst_rank ) {
    recv_rank = src_rank;
    flag = 1;
  }

  if( flag == 1 ) {
    MPI_Irecv(params->rbuf, params->count, params->datatype, recv_rank, TAG, MPI_COMM_WORLD, &req);
    MPI_Send(params->sbuf, params->count, params->datatype, recv_rank, TAG, MPI_COMM_WORLD);
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
  assert(src_rank != dst_rank);

  if (params->rank == src_rank) {
    recv_rank = dst_rank;
    flag = 1;
  } else if (params->rank == dst_rank) {
    recv_rank = src_rank;
    flag = 1;
  }

  if( flag == 1 ) {
    MPI_Sendrecv(params->sbuf, params->count, params->datatype, recv_rank, TAG, params->rbuf, params->count,
        params->datatype, recv_rank, TAG,
        MPI_COMM_WORLD, &stat);
  }

}

void initialize_data_pingpong(const basic_collective_params_t info, const long count, collective_params_t* params) {
  int i, invalid_ranks = 0;
  int other_rank;
  int nranks = 2;

  initialize_common_data(info, params);

  if (params->nprocs < nranks) { // abort pingpong if not enough processes
    fprintf(stderr, "ERROR: Cannot perform pingpong with only one process\n");
    MPI_Finalize();
    exit(0);
  }
  for (i = 0; i < nranks; i++) {
    if (params->pingpong_ranks[i] < 0 || params->pingpong_ranks[i] >= params->nprocs) {
      invalid_ranks = 1;
    }
  }
  if (params->pingpong_ranks[0] == params->pingpong_ranks[1]) {
    invalid_ranks = 1;
  }

  if (invalid_ranks) { // abort benchmark if the ranks are not correctly specified
    if (params->rank == 0) {
      fprintf(stderr, "ERROR: Invalid ping-pong ranks (%d,%d). Specify them using the \"--pingpong-ranks\" command-line option.\n",
          params->pingpong_ranks[0], params->pingpong_ranks[1]);
    }
    MPI_Finalize();
    exit(0);
  }

  // check whether the two ranks belong to different machines
  if (params->rank == params->pingpong_ranks[0] || params->rank == params->pingpong_ranks[1]) {
    char *local_proc_name, *other_proc_name;
    int local_proc_name_len, other_proc_name_len;
    MPI_Status stat;

    local_proc_name = (char*)calloc(MPI_MAX_PROCESSOR_NAME, sizeof(char));
    other_proc_name = (char*)calloc(MPI_MAX_PROCESSOR_NAME, sizeof(char));

    if (params->rank == params->pingpong_ranks[0]) {
      other_rank = params->pingpong_ranks[1];
    } else {
      other_rank = params->pingpong_ranks[0];
    }

    MPI_Get_processor_name(local_proc_name, &local_proc_name_len);

    MPI_Sendrecv(&local_proc_name_len, 1, MPI_INT, other_rank, TAG,
            &other_proc_name_len, 1, MPI_INT, other_rank, TAG,
            MPI_COMM_WORLD, &stat);

    MPI_Sendrecv(local_proc_name, local_proc_name_len, MPI_CHAR, other_rank, TAG,
        other_proc_name, other_proc_name_len, MPI_CHAR, other_rank, TAG,
        MPI_COMM_WORLD, &stat);

   if (local_proc_name_len == other_proc_name_len &&    // print warning if the two ranks are on the same node
       strcmp(local_proc_name, other_proc_name) == 0) {
         if (params->rank == params->pingpong_ranks[0]) {
           fprintf(stderr, "WARNING: The two ping-pong ranks (%d, %d) are running on the same node (%s)\n",
               params->pingpong_ranks[0], params->pingpong_ranks[1], local_proc_name);
         }
   }

   free(local_proc_name);
   free(other_proc_name);
  }


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

