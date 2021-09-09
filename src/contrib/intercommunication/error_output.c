/* Inter-Communicator MPI Benchmarking
 * Copyright (c) 2021 Stefan Christians
 * SPDX-License-Identifier: MIT
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "utilities.h"
#include "error_output.h"

static const int MAX_MSG_LEN = 256;
static const int OUTPUT_ROOT_PROC = 0;

/*
 * prints message to stdout
 */
void icmb_message(const char* message, ...)
{
  if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC))
  {
      va_list args;
      va_start(args, message);
      char buffer[MAX_MSG_LEN];
      vsnprintf(buffer, MAX_MSG_LEN, message, args);
      va_end(args);
      fprintf(stdout, "%s\n", buffer);
  }
}

/*
 * prints warning to stderr
 */
void icmb_warning(const char* message, ...)
{
  if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC))
  {
      va_list args;
      va_start(args, message);
      char buffer[MAX_MSG_LEN];
      vsnprintf(buffer, MAX_MSG_LEN, message, args);
      va_end(args);
      fprintf(stderr, "Warning: %s\n", buffer);
  }
}

/*
 * prints error to stderr
 */
void icmb_error(const char* message, ...)
{
  if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC))
  {
      va_list args;
      va_start(args, message);
      char buffer[MAX_MSG_LEN];
      vsnprintf(buffer, MAX_MSG_LEN, message, args);
      va_end(args);
      fprintf(stderr, "ERROR: %s\n", buffer);
  }
}

/*
 * exits benchmark
 */
void icmb_exit(int exit_code)
{
    // finalize automatically frees benchmark and global
    // communicators by callback
    MPI_Finalize();
    exit(exit_code);
}

/*
 * aborts benchmark
 *
 * aborts all processes connected by the global communicator
 */
void icmb_abort(int exit_code)
{
    MPI_Abort(icmb_global_communicator(), exit_code);
}

/*
 * prints error message and exits
 */
void icmb_error_and_exit(const char* message, int exit_code)
{
    icmb_error(message);
    icmb_exit(exit_code);
}
