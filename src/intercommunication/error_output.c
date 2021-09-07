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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "intercommunication.h"

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
