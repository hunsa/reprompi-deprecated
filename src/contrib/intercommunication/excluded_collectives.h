/* Inter-Communicator MPI Benchmarking
 * Copyright (c) 2021 Stefan Christians
 * SPDX-License-Identifier: MIT
 */

#ifndef ICMB_EXCLUDED_COLLECTIVES_H
#define ICMB_EXCLUDED_COLLECTIVES_H

#include "options_parser.h"
#include "utilities.h"

void icmb_abort_on_excluded_operation(char* op_name);
int icmb_is_excluded_operation(char* op_name);
int icmb_warn_on_excluded_operation(char* op_name);

#endif /* ICMB_EXCLUDED_COLLECTIVES_H */
