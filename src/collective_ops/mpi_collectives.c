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
#include <limits.h>
#include "mpi.h"
#include "buf_manager/mem_allocation.h"
#include "collectives.h"


inline void execute_Scan(collective_params_t* params) {
    MPI_Scan(params->sbuf, params->rbuf, params->count, params->datatype,
            params->op, MPI_COMM_WORLD);
}

inline void execute_Allreduce(collective_params_t* params) {
    MPI_Allreduce(params->sbuf, params->rbuf, params->count, params->datatype,
            params->op, MPI_COMM_WORLD);
}

inline void execute_Exscan(collective_params_t* params) {
    MPI_Exscan(params->sbuf, params->rbuf, params->count, params->datatype,
            params->op, MPI_COMM_WORLD);
}

inline void execute_Reduce(collective_params_t* params) {
    MPI_Reduce(params->sbuf, params->rbuf, params->count, params->datatype,
            params->op, params->root, MPI_COMM_WORLD);
}

inline void execute_Barrier(collective_params_t* params) {
    MPI_Barrier(MPI_COMM_WORLD);
}



/***************************************/
// Bcast

inline void execute_Bcast(collective_params_t* params) {
    MPI_Bcast(params->sbuf, params->count, params->datatype,
            params->root, MPI_COMM_WORLD);
}


void cleanup_Bcast_data(collective_params_t* params) {
    cleanup_data_default(params);
}




/***************************************/
// Scatter

inline void execute_Scatter(collective_params_t* params) {
    MPI_Scatter(params->sbuf, params->count, params->datatype,
            params->rbuf, params->count, params->datatype,
            params->root, MPI_COMM_WORLD);
}

void initialize_data_Scatter(const basic_collective_params_t info, const long count, collective_params_t* params) {
    initialize_common_data(info, params);

    params->count = count; // size of the Scatter per-process buffer

    params->scount = count * params->nprocs;
    params->rcount = count;

    assert (params->scount < INT_MAX);
    assert (params->rcount < INT_MAX);
    params->sbuf = (char*)reprompi_calloc(params->scount, params->datatype_extent);
    params->rbuf = (char*)reprompi_calloc(params->rcount, params->datatype_extent);
}


void cleanup_data_Scatter(collective_params_t* params) {
    free(params->sbuf);
    free(params->rbuf);
    params->sbuf = NULL;
    params->rbuf = NULL;
}


/***************************************/



/***************************************/
// MPI_Gather

inline void execute_Gather(collective_params_t* params) {
    MPI_Gather(params->sbuf, params->count, params->datatype,
            params->rbuf, params->count, params->datatype,
            params->root, MPI_COMM_WORLD);
}


void initialize_data_Gather(const basic_collective_params_t info, const long count, collective_params_t* params) {
    initialize_common_data(info, params);

    params->count = count; // size of the buffer sent by each process

    params->scount = count;
    params->rcount = count * params->nprocs;

    assert (params->scount < INT_MAX);
    assert (params->rcount < INT_MAX);
    params->sbuf = (char*)reprompi_calloc(params->scount, params->datatype_extent);
    params->rbuf = (char*)reprompi_calloc(params->rcount, params->datatype_extent);

}


void cleanup_data_Gather(collective_params_t* params) {
    free(params->sbuf);
    free(params->rbuf);
    params->sbuf = NULL;
    params->rbuf = NULL;
}
/***************************************/


/***************************************/
// MPI_Allgather

inline void execute_Allgather(collective_params_t* params) {
    MPI_Allgather(params->sbuf, params->count, params->datatype,
            params->rbuf, params->count, params->datatype,
            MPI_COMM_WORLD);
}



void initialize_data_Allgather(const basic_collective_params_t info, const long count, collective_params_t* params) {
    initialize_common_data(info, params);

    params->count = count; // size of the buffer sent by each process

    params->scount = count;
    params->rcount = count * params->nprocs;

    assert (params->scount < INT_MAX);
    assert (params->rcount < INT_MAX);
    params->sbuf = (char*)reprompi_calloc(params->scount, params->datatype_extent);
    params->rbuf = (char*)reprompi_calloc(params->rcount, params->datatype_extent);

}


void cleanup_data_Allgather(collective_params_t* params) {
    free(params->sbuf);
    free(params->rbuf);
    params->sbuf = NULL;
    params->rbuf = NULL;
}
/***************************************/



/***************************************/
// MPI_Reduce_scatter

inline void execute_Reduce_scatter(collective_params_t* params) {
    MPI_Reduce_scatter(params->sbuf, params->rbuf, params->counts_array,
            params->datatype, params->op, MPI_COMM_WORLD);
}



void initialize_data_Reduce_scatter(const basic_collective_params_t info, const long count, collective_params_t* params) {
    int i;

    initialize_common_data(info, params);

    params->count = count; // size of the block received by each process

    params->scount = count * params->nprocs;
    params->rcount = count;

    assert (params->scount < INT_MAX);
    assert (params->rcount < INT_MAX);

    assert (count < INT_MAX);
    // we send the same number of elements to all processes
    params->counts_array = (int*)reprompi_calloc(params->nprocs, sizeof(int));
    for (i=0; i< params->nprocs; i++) {
        params->counts_array[i] = count;
    }

    params->sbuf = (char*)reprompi_calloc(params->scount, params->datatype_extent);
    params->rbuf = (char*)reprompi_calloc(params->rcount, params->datatype_extent);

}


void cleanup_data_Reduce_scatter(collective_params_t* params) {
    free(params->sbuf);
    free(params->rbuf);

    free(params->counts_array);

    params->sbuf = NULL;
    params->rbuf = NULL;
    params->counts_array = NULL;
}
/***************************************/


/***************************************/
// MPI_Reduce_scatter_block

inline void execute_Reduce_scatter_block(collective_params_t* params) {
    MPI_Reduce_scatter_block(params->sbuf, params->rbuf, params->rcount,
            params->datatype, params->op, MPI_COMM_WORLD);
}



void initialize_data_Reduce_scatter_block(const basic_collective_params_t info, const long count, collective_params_t* params) {
    initialize_common_data(info, params);

    params->count = count; // size of the block received by each process

    params->scount = count * params->nprocs;
    params->rcount = count;

    assert (params->scount < INT_MAX);
    assert (params->rcount < INT_MAX);
    params->sbuf = (char*)reprompi_calloc(params->scount, params->datatype_extent);
    params->rbuf = (char*)reprompi_calloc(params->rcount, params->datatype_extent);
}


void cleanup_data_Reduce_scatter_block(collective_params_t* params) {
    free(params->sbuf);
    free(params->rbuf);
    params->sbuf = NULL;
    params->rbuf = NULL;
}
/***************************************/



/***************************************/
// MPI_Alltoall

inline void execute_Alltoall(collective_params_t* params) {
    MPI_Alltoall(params->sbuf, params->count, params->datatype,
            params->rbuf, params->count, params->datatype,
            MPI_COMM_WORLD);
}

void initialize_data_Alltoall(const basic_collective_params_t info, const long count, collective_params_t* params) {
    initialize_common_data(info, params);

    params->count = count;

    params->scount = count * params->nprocs;
    params->rcount = count * params->nprocs;

    assert (params->scount < INT_MAX);
    assert (params->rcount < INT_MAX);
    params->sbuf = (char*)reprompi_calloc(params->scount, params->datatype_extent);
    params->rbuf = (char*)reprompi_calloc(params->rcount, params->datatype_extent);

}


void cleanup_data_Alltoall(collective_params_t* params) {
    free(params->sbuf);
    free(params->rbuf);
    params->sbuf = NULL;
    params->rbuf = NULL;
}
/***************************************/

