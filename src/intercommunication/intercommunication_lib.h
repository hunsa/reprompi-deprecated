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

#ifndef ICMB_INTERCOMMUNICATION_LIB_H
#define ICMB_INTERCOMMUNICATION_LIB_H

#include <mpi.h>

/*
 * For benchmarking with intra-communicators, sets the intra-communicator
 * to use.
 *
 * If no communicator is set, the benchmark will default to using
 * MPI_COMM_WORLD
 */
void set_communicator(MPI_Comm intracommunicator);

/*
 * For benchmarking with inter-communicators, sets the communicators to use for
 * the initiating and responding group, respectively.
 *
 * The inter-communicator will be constructed from these two
 * intra-communicators.
 */
void set_communicators(MPI_Comm initiator, MPI_Comm responder);

#endif /* ICMB_INTERCOMMUNICATION_LIB_H */

