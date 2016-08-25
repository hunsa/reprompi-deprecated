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
#include "mpi.h"
#include "collectives.h"


/***************************************/
// MPI_Bcast with Scatter + Allgather

inline void execute_GL_Bcast_as_ScatterAllgather(collective_params_t* params) {
    MPI_Scatter(params->sbuf, params->msize, params->datatype,
                params->rbuf, params->msize, params->datatype,
                params->root, MPI_COMM_WORLD);
    MPI_Allgather(params->rbuf, params->msize, params->datatype,
                params->tmp_buf, params->msize, params->datatype,
                MPI_COMM_WORLD);

#ifdef COMPILE_BENCH_TESTS
    memcpy(params->sbuf, params->tmp_buf, params->scount);
#endif
}



void initialize_data_GL_Bcast_as_ScatterAllgather(const basic_collective_params_t info, const long msize, collective_params_t* params) {
    initialize_common_data(info, params);

    params->msize = msize/params->nprocs + (params->msize%params->nprocs != 0);

    params->scount = msize;
    params->rcount = msize + params->nprocs; // at most one extra byte per process

    params->sbuf = (char*) malloc((params->scount + params->nprocs) * params->datatypesize);
    params->rbuf = (char*) malloc(params->rcount * params->datatypesize);
    params->tmp_buf = (char*) malloc(params->rcount * params->datatypesize);

}


void cleanup_data_GL_Bcast_as_ScatterAllgather(collective_params_t* params) {
    free(params->sbuf);
    free(params->rbuf);
    free(params->tmp_buf);
    params->sbuf = NULL;
    params->rbuf = NULL;
    params->tmp_buf = NULL;
}
/***************************************/







