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

#ifndef REPROMPIB__OPTION_PARSER_DATA_H_
#define REPROMPIB__OPTION_PARSER_DATA_H_



typedef struct reprompib_common_opt {
    int verbose; /* -v */
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


typedef struct reprompib_opt {
    reprompib_common_options_t common_opt;

    long n_rep; /* --repetitions */

    int* print_summary_methods; /* --summary */
    int n_print_summary_selected;

} reprompib_options_t;

#endif /* REPROMPIB__OPTION_PARSER_DATA_H_ */
