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
// MPI_Allgather with Alltoall

inline void execute_GL_Allgather_as_Alltoall(collective_params_t* params) {

#ifdef COMPILE_BENCH_TESTS
    // replicate data in the source buffer nprocs times
    {
        int i;
        for (i=1; i<params->remote_size; i++) {
            memcpy((char*)params->sbuf + i * params->count * params->datatype_extent, (char*)params->sbuf, params->count * params->datatype_extent);
        }
    }
#endif

    MPI_Alltoall(params->sbuf, params->count, params->datatype, params->rbuf, params->count, params->datatype, params->communicator);
}



void initialize_data_GL_Allgather_as_Alltoall(const basic_collective_params_t info, const long count, collective_params_t* params) {
    initialize_common_data(info, params);

    params->count = count; // size of the buffer for each process

    // source buffer must contain count elements repeated nprocs times
    params->scount = count * params->remote_size;
    params->rcount = count * params->remote_size;

    assert (params->scount < INT_MAX);
    assert (params->rcount < INT_MAX);

    params->sbuf = (char*)reprompi_calloc(params->scount, params->datatype_extent);
    params->rbuf = (char*)reprompi_calloc(params->rcount, params->datatype_extent);
}


void cleanup_data_GL_Allgather_as_Alltoall(collective_params_t* params) {
    free(params->sbuf);
    free(params->rbuf);
    params->sbuf = NULL;
    params->rbuf = NULL;
}
/***************************************/



/***************************************/
// MPI_Allgather with Allreduce

inline void execute_GL_Allgather_as_Allreduce(collective_params_t* params) {

#ifdef COMPILE_BENCH_TESTS
    memcpy((char*)params->tmp_buf + params->rank * params->scount * params->datatype_extent, (char*)params->sbuf, params->scount * params->datatype_extent);
#endif

    MPI_Allreduce(params->tmp_buf, params->rbuf, params->trcount, params->datatype, params->op, params->communicator);

}


void initialize_data_GL_Allgather_as_Allreduce(const basic_collective_params_t info, const long count, collective_params_t* params) {
    int num_ints, num_adds, num_dtypes, combiner;
    int i;

    initialize_common_data(info, params);

    params->count = count;

    params->scount = count;
    params->rcount = count * params->remote_size;

    params->tscount = count * params->larger_size;
    params->trcount = count * params->larger_size;

    assert (params->scount < INT_MAX);
    assert (params->rcount < INT_MAX);
    assert (params->tscount < INT_MAX);
    assert (params->trcount < INT_MAX);

    params->sbuf = (char*)reprompi_calloc(params->scount, params->datatype_extent);
    params->rbuf = (char*)reprompi_calloc(params->trcount, params->datatype_extent);
    params->tmp_buf = (char*)reprompi_calloc(params->tscount, params->datatype_extent);

    // set identity operand for different operations
    if (params->op == MPI_BAND || params->op == MPI_PROD) {
        MPI_Type_get_envelope(params->datatype, &num_ints, &num_adds, &num_dtypes, &combiner);

        if (combiner == MPI_COMBINER_NAMED) {
            if (params->datatype == MPI_INT) {
                for (i=0; i<params->tscount; i++) {
                    ((int*)params->tmp_buf)[i] = 1;
                }
            }
            else {
                if (params->datatype == MPI_DOUBLE) {
                    for (i=0; i<params->tscount; i++) {
                        ((double*)params->tmp_buf)[i] = 1;
                    }
                }
                else {
                    memset( params->tmp_buf, 0xFF, params->tscount * params->datatype_extent);
                }
            }
        }
        else {
            memset( params->tmp_buf, 0xFF, params->tscount * params->datatype_extent);
        }
    }
    else {
        memset(params->tmp_buf, 0, params->tscount * params->datatype_extent);
    }
}


void cleanup_data_GL_Allgather_as_Allreduce(collective_params_t* params) {
    free(params->sbuf);
    free(params->rbuf);
    free(params->tmp_buf);
    params->sbuf = NULL;
    params->rbuf = NULL;
    params->tmp_buf = NULL;

}
/***************************************/




/***************************************/
// MPI_Allgather with Gather + Bcast

inline void execute_GL_Allgather_as_GatherBcast(collective_params_t* params) {

    if (!params->is_intercommunicator)
    {
        MPI_Gather(params->sbuf, params->count, params->datatype, params->rbuf, params->count, params->datatype, 0, params->communicator);
    }
    else
    {
        // for inter-communication, need to gather in both directions
        MPI_Gather(params->sbuf, params->count, params->datatype, params->rbuf, params->count, params->datatype, params->troot_i2r, params->communicator);
        MPI_Gather(params->sbuf, params->count, params->datatype, params->rbuf, params->count, params->datatype, params->troot_r2i, params->communicator);
    }

    // broadcast within local group only
    MPI_Bcast(params->rbuf, params->rcount, params->datatype, 0, params->partial_communicator);
}


void initialize_data_GL_Allgather_as_GatherBcast(const basic_collective_params_t info, const long count, collective_params_t* params) {
    initialize_common_data(info, params);

    params->count = count;

    params->scount = count;
    params->rcount = count * params->remote_size;

    assert (params->scount < INT_MAX);
    assert (params->rcount < INT_MAX);

    params->sbuf = (char*)reprompi_calloc(params->scount, params->datatype_extent);
    params->rbuf = (char*)reprompi_calloc(params->rcount, params->datatype_extent);

}


void cleanup_data_GL_Allgather_as_GatherBcast(collective_params_t* params) {
    free(params->sbuf);
    free(params->rbuf);
    params->sbuf = NULL;
    params->rbuf = NULL;

}
/***************************************/


