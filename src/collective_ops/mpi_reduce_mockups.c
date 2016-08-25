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
// MPI_Reduce with Allreduce
inline void execute_GL_Reduce_as_Allreduce(collective_params_t* params) {

    MPI_Allreduce(params->sbuf, params->rbuf, params->msize, params->datatype,
            params->op, MPI_COMM_WORLD);

}


void initialize_data_GL_Reduce_as_Allreduce(const basic_collective_params_t info, const long msize, collective_params_t* params) {
    initialize_common_data(info, params);

    params->msize = msize;

    params->scount = msize;
    params->rcount = msize;

    params->sbuf = (char*) malloc(params->scount * params->datatypesize);
    params->rbuf = (char*) malloc(params->rcount * params->datatypesize);

}


void cleanup_data_GL_Reduce_as_Allreduce(collective_params_t* params) {
    free(params->sbuf);
    free(params->rbuf);
    params->sbuf = NULL;
    params->rbuf = NULL;

}
/***************************************/


/***************************************/
// MPI_Reduce with Reduce_scatter and Gather
inline void execute_GL_Reduce_as_ReducescatterGather(collective_params_t* params) {

    MPI_Reduce_scatter(params->sbuf, params->tmp_buf, params->counts_array,
            params->datatype, params->op, MPI_COMM_WORLD);

    MPI_Gather(params->tmp_buf, params->msize, params->datatype,
                params->rbuf, params->msize, params->datatype,
                params->root, MPI_COMM_WORLD);

}


void initialize_data_GL_Reduce_as_ReducescatterGather(const basic_collective_params_t info, const long msize, collective_params_t* params) {
    int i;

    initialize_common_data(info, params);

    params->msize = msize / params->nprocs; // block size per process

    params->scount = msize;
    params->rcount = msize;

    // we send the same number of elements to all processes
    params->counts_array = (int*) malloc(params->nprocs * sizeof(int));
    for (i=0; i< params->nprocs; i++) {
        params->counts_array[i] = params->msize;
    }

    params->sbuf = (char*) malloc(params->scount * params->datatypesize);
    params->rbuf = (char*) malloc(params->rcount * params->datatypesize);
    params->tmp_buf = (char*) malloc(params->scount * params->datatypesize);

}


void cleanup_data_GL_Reduce_as_ReducescatterGather(collective_params_t* params) {
    free(params->sbuf);
    free(params->rbuf);
    free(params->tmp_buf);
    free(params->counts_array);
    params->sbuf = NULL;
    params->rbuf = NULL;
    params->tmp_buf = NULL;
    params->counts_array = NULL;

}
/***************************************/



/***************************************/
// MPI_Reduce with Reduce_scatter_block and Gather
inline void execute_GL_Reduce_as_ReducescatterblockGather(collective_params_t* params) {

    MPI_Reduce_scatter_block(params->sbuf, params->tmp_buf, params->msize,
            params->datatype, params->op, MPI_COMM_WORLD);

    MPI_Gather(params->tmp_buf, params->msize, params->datatype,
                params->rbuf, params->msize, params->datatype,
                params->root, MPI_COMM_WORLD);

}


void initialize_data_GL_Reduce_as_ReducescatterblockGather(const basic_collective_params_t info, const long msize, collective_params_t* params) {
    initialize_common_data(info, params);

    params->msize = msize / params->nprocs; // block size per process

    params->scount = msize;
    params->rcount = msize;

    params->sbuf = (char*) malloc(params->scount * params->datatypesize);
    params->rbuf = (char*) malloc(params->rcount * params->datatypesize);
    params->tmp_buf = (char*) malloc(params->scount * params->datatypesize);

}


void cleanup_data_GL_Reduce_as_ReducescatterblockGather(collective_params_t* params) {
    free(params->sbuf);
    free(params->rbuf);
    free(params->tmp_buf);
    params->sbuf = NULL;
    params->rbuf = NULL;
    params->tmp_buf = NULL;

}
/***************************************/

