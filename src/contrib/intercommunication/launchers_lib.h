
/* Inter-Communicator MPI Benchmarking
 * Copyright (c) 2021 Stefan Christians
 * SPDX-License-Identifier: MIT
 */

#ifndef ICMB_LAUNCHERS_LIB_H
#define ICMB_LAUNCHERS_LIB_H

/*
 * When used as a library, it is the applications responsibility to start
 * processes, organize them in groups, and create communicators.
 * This interface provides the means for the application to hand over
 * communicators for the intiating and responding groups to the benchmark
 */

/*
 * For benchmarking with intra-communicators, sets the intra-communicator
 * to use.
 *
 * If no communicator is set, the benchmark will default to using
 * MPI_COMM_WORLD
 */
void icmb_set_communicator(MPI_Comm intracommunicator);

/*
 * For benchmarking with inter-communicators, sets the communicators to use for
 * the initiating and responding group, respectively.
 *
 * The inter-communicator will be constructed from these two
 * intra-communicators.
 */
void icmb_set_communicators(MPI_Comm initiator, MPI_Comm responder);

#endif /* ICMB_LAUNCHERS_LIB_H */
