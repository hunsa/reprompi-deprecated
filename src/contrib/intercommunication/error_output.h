/* Inter-Communicator MPI Benchmarking
 * Copyright (c) 2021 Stefan Christians
 * SPDX-License-Identifier: MIT
 */

#ifndef ICMB_ERROR_OUTPUT_H
#define ICMB_ERROR_OUTPUT_H

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

#endif /* ICMB_ERROR_OUTPUT_H */
