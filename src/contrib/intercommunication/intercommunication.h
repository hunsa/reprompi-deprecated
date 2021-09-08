/* Inter-Communicator MPI Benchmarking
 * Copyright (c) 2021 Stefan Christians
 * SPDX-License-Identifier: MIT
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
    ICMB_ERROR_FILE_INPUT,
    ICMB_ERROR_MESSAGE_SIZE,
    ICMB_ERROR_NUM_PROCS,
    ICMB_ERROR_PING_EQUALS_PONG,
    ICMB_ERROR_PING_MISSING,
    ICMB_ERROR_PINGPONG_INVALID,
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
