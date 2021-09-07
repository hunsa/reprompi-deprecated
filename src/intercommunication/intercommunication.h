/*  ReproMPI Benchmark
 *
 *  Copyright (c) 2021 Stefan Christians
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

#ifndef ICMB_INTERCOMMUNICATION_H
#define ICMB_INTERCOMMUNICATION_H

#include <mpi.h>

// command line parser
void icmb_copy_commandline(int argc, char**argv);
void icmb_parse_intercommunication_options (int argc, char** argv);
void icmb_print_intercommunication_help();

// utilities
MPI_Comm icmb_benchmark_communicator();
int icmb_benchmark_rank();
int icmb_collective_root(int root);
MPI_Comm icmb_global_communicator();
int icmb_global_rank();
int icmb_global_size();
int icmb_has_initiator_rank(int rank);
int icmb_initiator_size();
int icmb_is_initiator();
int icmb_is_intercommunicator();
int icmb_is_responder();
int icmb_larger_size();
int icmb_local_size();
int icmb_lookup_global_rank(int initiator_rank);
int icmb_remote_size();
int icmb_responder_size();

// excluded operations
void icmb_abort_on_excluded_operation(char* op_name);
int icmb_is_excluded_operation(char* op_name);
int icmb_warn_on_excluded_operation(char* op_name);

// errors
enum icmb_error_codes
{
    ICMB_ERROR_AAA = 100,
    ICMB_ERROR_PING_EQUALS_PONG,
    ICMB_ERROR_PING_MISSING,
    ICMB_ERROR_PING_TOO_HIGH,
    ICMB_ERROR_PONG_MISSING,
    ICMB_ERROR_PONG_TOO_HIGH,
    ICMB_ERROR_ROOT_TOO_HIGH,
    ICMB_ERROR_SAME_CLIENT_SERVER,
    ICMB_ERROR_SPAWN_FROM_CLIENT_SERVER,
    ICMB_ERROR_SPLIT_FROM_CLIENT_SERVER,
    ICMB_ERROR_UNDEFINED_OPERATION,
};

void icmb_abort(int exit_code);
void icmb_error_and_exit(const char* message, int exit_code);
void icmb_error(const char* message, ...);
void icmb_exit(int exit_code);
void icmb_message(const char* message, ...);
void icmb_warning(const char* message, ...);


#endif /* ICMB_INTERCOMMUNICATION_H */
