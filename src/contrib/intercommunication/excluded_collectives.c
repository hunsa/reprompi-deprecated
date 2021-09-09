/* Inter-Communicator MPI Benchmarking
 * Copyright (c) 2021 Stefan Christians
 * SPDX-License-Identifier: MIT
 */

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "error_output.h"
#include "excluded_collectives.h"

/*
 * scan operations are not defined for inter-communicators
 * and must be excluded from benchmarking
 */

static char* excluded_operations[] = {
    "mpi_scan", "mpi_iscan", "mpi_scan_init",
    "mpi_scan_c", "mpi_iscan_c", "mpi_scan_init_c",
    "mpi_exscan", "mpi_iexscan", "mpi_exscan_init",
    "mpi_exscan_c", "mpi_iexscan_c", "mpi_exscan_init_c",
    NULL
};

/*
 * helper function to convert string to lower case
 */
static char* to_lower(char* s) {
  for(char *p=s; *p; p++) *p=tolower(*p);
  return s;
}

/*
 * returns true if the operation must be excluded because
 * this process uses an inter-communicator
 */
int icmb_is_excluded_operation(char* op_name)
{
    // all operations are allowed on intra-communicators
    if (!icmb_is_intercommunicator())
    {
        return 0;
    }

    // see if lower case of op_name is in lsit of excluded operations
    char* lower = (char *) malloc((strlen(op_name)+1) * sizeof(char));
    strcpy(lower, to_lower(op_name));

    int index = -1;
    int i;
    for (i=0; excluded_operations[i]!=NULL; ++i)
    {
        if (0 == strcmp(lower, excluded_operations[i]))
        {
            index = i;
            break;
        }
    }
    free(lower);

    if (-1 == index) {
        return 0;
    }

    return 1;
}

/*
 * prints a warning and returns true if the operation
 * must be excluded because this process uses
 * an inter-communicator
 */
int icmb_warn_on_excluded_operation(char* op_name)
{
    if (icmb_is_excluded_operation(op_name))
    {
        icmb_warning("%s is not defined for inter-communication, skipping", op_name);
        return 1;
    }

    return 0;
}

/*
 * prints an error and aborts execution if the operation
 * must be excluded because this process uses
 * an inter-communicator
 */
void icmb_abort_on_excluded_operation(char* op_name)
{
    if (icmb_is_excluded_operation(op_name))
    {
        icmb_error("%s is not defined for inter-communication, terminating execution", op_name);
        icmb_exit(ICMB_ERROR_UNDEFINED_OPERATION);
    }
}
