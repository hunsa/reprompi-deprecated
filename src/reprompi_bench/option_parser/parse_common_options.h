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

#ifndef REPROMPIB_PARSE_COMMON_OPTIONS_H_
#define REPROMPIB_PARSE_COMMON_OPTIONS_H_

#include "reprompi_bench/utils/keyvalue_store.h"

typedef struct reprompib_common_opt {
    int n_calls; /* number of MPI calls */
    int* list_mpi_calls;

    int n_msize; /* number of message sizes to measure */
    size_t* msize_list; /* --msizes-list / --msize-interval */

    int enable_job_shuffling;

    MPI_Datatype datatype;
    MPI_Op operation;
    int root_proc;

    char* input_file;
    char* output_file;

    // parameters relevant for ping-pong operations
    int pingpong_ranks[2];
} reprompib_common_options_t;


void reprompib_free_common_parameters(const reprompib_common_options_t* opts_p);
void reprompib_parse_common_options(reprompib_common_options_t* opts_p, int argc, char** argv, reprompib_dictionary_t* dict);

#endif /* REPROMPIB_PARSE_COMMON_OPTIONS_H_ */
