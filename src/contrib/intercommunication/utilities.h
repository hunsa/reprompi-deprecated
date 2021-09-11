/* Inter-Communicator MPI Benchmarking
 * Copyright (c) 2021 Stefan Christians
 * SPDX-License-Identifier: MIT
 */

#ifndef ICMB_UTILITIES_H
#define ICMB_UTILITIES_H

#include <mpi.h>

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
int icmb_lookup_benchmark_rank(int global_rank);
int icmb_lookup_global_rank(int initiator_rank);
int icmb_lookup_is_initiator (int global_rank);
int icmb_lookup_is_responder(int global_rank);
int icmb_lookup_local_size(int global_rank);
int icmb_lookup_remote_size(int global_rank);
MPI_Comm icmb_partial_communicator();
int icmb_remote_size();
int icmb_responder_size();

#endif /* ICMB_UTILITIES_H */

