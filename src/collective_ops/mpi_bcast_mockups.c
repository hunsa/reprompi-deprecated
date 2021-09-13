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
// MPI_Bcast with Scatter + Allgather

inline void execute_GL_Bcast_as_ScatterAllgather(collective_params_t* params)
{
    MPI_Scatter(params->sbuf, params->scount, params->datatype, params->tmp_buf, params->rcount, params->datatype, params->root, params->communicator);
    MPI_Allgather(params->tmp_buf, params->rcount, params->datatype, params->rbuf, params->rcount, params->datatype, params->partial_communicator);
}

void initialize_data_GL_Bcast_as_ScatterAllgather(const basic_collective_params_t info, const long count, collective_params_t* params)
{
    initialize_common_data(info, params);

    params->count = count;

    params->scount = count;
    params->rcount = count;

    assert (params->scount < INT_MAX);
    assert (params->rcount < INT_MAX);

    params->sbuf = (char*) reprompi_calloc(params->scount, params->datatype_extent);
    params->tmp_buf = (char*) reprompi_calloc(params->rcount, params->datatype_extent);
    params->rbuf = (char*) reprompi_calloc(params->rcount * params->local_size, params->datatype_extent);
}


void cleanup_data_GL_Bcast_as_ScatterAllgather(collective_params_t* params)
{
    free(params->sbuf);
    free(params->rbuf);
    free(params->tmp_buf);
    params->sbuf = NULL;
    params->rbuf = NULL;
    params->tmp_buf = NULL;
}
/***************************************/
