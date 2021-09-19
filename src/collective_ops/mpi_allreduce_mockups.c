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
#include <limits.h>
#include "mpi.h"

#include "buf_manager/mem_allocation.h"
#include "collectives.h"


/***************************************/
// MPI_Allreduce with Reduce + Bcast

inline void execute_GL_Allreduce_as_ReduceBcast(collective_params_t* params)
{
    if (!params->is_intercommunicator)
    {
        MPI_Reduce(params->sbuf, params->rbuf, params->scount, params->datatype, params->op, 0, params->communicator);
    }
    else
    {
        // for inter-communication, need to reduce in both directions
        MPI_Reduce(params->sbuf, params->rbuf, params->scount, params->datatype, params->op, params->troot_i2r, params->communicator);
        MPI_Reduce(params->sbuf, params->rbuf, params->scount, params->datatype, params->op, params->troot_r2i, params->communicator);
    }

    // broadcast within local group only
    MPI_Bcast(params->rbuf, params->rcount, params->datatype, 0, params->partial_communicator);
}



void initialize_data_GL_Allreduce_as_ReduceBcast(const basic_collective_params_t info, const long count, collective_params_t* params)
{
    initialize_common_data(info, params);

    params->count = count;

    params->scount = count;
    params->rcount = count;

    assert (params->scount < INT_MAX);
    assert (params->rcount < INT_MAX);

    params->sbuf = (char*)reprompi_calloc(params->scount, params->datatype_extent);
    params->rbuf = (char*)reprompi_calloc(params->rcount, params->datatype_extent);

}


void cleanup_data_GL_Allreduce_as_ReduceBcast(collective_params_t* params)
{
    free(params->sbuf);
    free(params->rbuf);
    params->sbuf = NULL;
    params->rbuf = NULL;

}
/***************************************/



/***************************************/
// MPI_Allreduce with Reduce_scatterblock and Allgather
inline void execute_GL_Allreduce_as_ReducescatterblockAllgather(collective_params_t* params)
{

    MPI_Reduce_scatter_block(params->sbuf, params->tmp_buf, params->trcount, params->datatype, params->op, params->communicator);
    MPI_Allgather(params->tmp_buf, params->rcount, params->datatype, params->rbuf, params->rcount, params->datatype, params->partial_communicator);

}


void initialize_data_GL_Allreduce_as_ReducescatterblockAllgather(const basic_collective_params_t info, const long count, collective_params_t* params)
{
    initialize_common_data(info, params);

    params->count = count; // size of the block received by each process

    params->scount = count * params->remote_size;
    params->tscount = count * params->local_size;
    params->rcount = count;
    params->trcount = count;
    if (params->is_intercommunicator)
    {
        long equalizer = params->combined_size / params->local_size;
        params->tscount *= equalizer; // send buffers must have same size in both groups
        params->trcount *= equalizer; // (local_size * trcount) must be same in both groups
    }

    assert (params->scount < INT_MAX);
    assert (params->rcount < INT_MAX);
    assert (params->tscount < INT_MAX);
    assert (params->trcount < INT_MAX);

    params->sbuf = (char *) reprompi_calloc(params->tscount, params->datatype_extent);
    params->tmp_buf = (char *) reprompi_calloc(params->trcount, params->datatype_extent);
    params->rbuf = (char *) reprompi_calloc(params->tscount, params->datatype_extent);
}


void cleanup_data_GL_Allreduce_as_ReducescatterblockAllgather(collective_params_t* params)
{
    free(params->sbuf);
    free(params->rbuf);
    free(params->tmp_buf);
    params->sbuf = NULL;
    params->rbuf = NULL;
    params->tmp_buf = NULL;

}
/***************************************/


/***************************************/
// MPI_Allreduce with Reduce_scatter and Allgatherv
inline void execute_GL_Allreduce_as_ReducescatterAllgatherv(collective_params_t* params)
{

    MPI_Reduce_scatter(params->sbuf, params->tmp_buf, params->scounts_array, params->datatype, params->op, params->communicator);

    MPI_Allgatherv(params->tmp_buf, params->count, params->datatype, params->rbuf, params->counts_array, params->displ_array, params->datatype, params->partial_communicator);

}


void initialize_data_GL_Allreduce_as_ReducescatterAllgatherv(const basic_collective_params_t info, const long count, collective_params_t* params)
{

    int i;

    initialize_common_data(info, params);

    params->count = count; // size of the block received by each process

    params->scount = count * params->remote_size;
    params->tscount = count * params->local_size;
    params->rcount = count;
    params->trcount = count;
    if (params->is_intercommunicator)
    {
        long equalizer = params->combined_size / params->local_size;
        params->tscount *= equalizer; // send buffers must have same size in both groups
        params->trcount *= equalizer; // (local_size * trcount) must be same in both groups
    }

    assert (params->scount < INT_MAX);
    assert (params->rcount < INT_MAX);
    assert (params->tscount < INT_MAX);
    assert (params->trcount < INT_MAX);

    params->sbuf = (char*)reprompi_calloc(params->tscount, params->datatype_extent);
    params->tmp_buf = (char*)reprompi_calloc(params->trcount, params->datatype_extent);
    params->rbuf = (char*)reprompi_calloc(params->tscount, params->datatype_extent);

    // calculate inter-communicator count array for reducescatter
    params->scounts_array = (int*)reprompi_calloc(params->local_size, sizeof(int));
    for (i=0; i< params->local_size; i++) {
        params->scounts_array[i] = params->trcount;
    }

    // calculate intra-communicator count and displacement arrays for allgatherv
    params->counts_array = (int*)reprompi_calloc(params->local_size, sizeof(int));
    params->displ_array = (int*)reprompi_calloc(params->local_size, sizeof(int));
    for (i=0; i< params->local_size; i++) {
        params->counts_array[i] = params->rcount;
        if (i==0)
        {
            params->displ_array[0] = 0;
        }
        else{
            params->displ_array[i] = params->displ_array[i-1] + params->counts_array[i-1];
        }
    }
}


void cleanup_data_GL_Allreduce_as_ReducescatterAllgatherv(collective_params_t* params)
{
    free(params->sbuf);
    free(params->rbuf);
    free(params->tmp_buf);
    free(params->counts_array);
    free(params->displ_array);
    free(params->scounts_array);
    params->sbuf = NULL;
    params->rbuf = NULL;
    params->tmp_buf = NULL;
    params->counts_array = NULL;
    params->displ_array = NULL;
    params->scounts_array = NULL;

}
/***************************************/
