/* Inter-Communicator MPI Benchmarking
 * Copyright (c) 2021 Stefan Christians
 * SPDX-License-Identifier: MIT
 */

#ifndef ICMB_ATTRIBUTES_H
#define ICMB_ATTRIBUTES_H

#include <mpi.h>

// attribute types
typedef struct
{
    int ref_count;
    char* service_name;
    char* port_name;
} icmb_attribute_port_t;

typedef struct
{
    int ref_count;
    MPI_Comm communicator;
} icmb_attribute_communicator_t;

// attribute keys
extern int icmb_key_benchmarkcommunicator;
extern int icmb_key_globalcommunicator;
extern int icmb_key_partialcommunicator;
extern int icmb_key_initiator;
extern int icmb_key_port;

// callback functions
int icmb_attribute_communicator_copier (MPI_Comm comm, int key, void* state, void* value_in, void* value_out, int* flag);
int icmb_attribute_communicator_destructor (MPI_Comm comm, int key, void* value, void* state);
int icmb_attribute_port_copier (MPI_Comm comm, int key, void* state, void* value_in, void* value_out, int* flag);
int icmb_attribute_port_destructor (MPI_Comm comm, int key, void* value, void* state);

// accessors
int icmb_get_initiator_attribute(MPI_Comm comm);
MPI_Comm icmb_get_benchmarkcommunicator_attribute();
MPI_Comm icmb_get_globalcommunicator_attribute();
MPI_Comm icmb_get_partialcommunicator_attribute();
int icmb_set_benchmarkcommunicator_attribute(MPI_Comm benchmark_communicator);
int icmb_set_globalcommunicator_attribute(MPI_Comm global_communicator);
int icmb_set_partialcommunicator_attribute(MPI_Comm partial_communicator);
int icmb_set_initiator_attribute(MPI_Comm comm);
int icmb_set_port_attribute(MPI_Comm comm, const char* service_name, const char* port_name);

#endif /* ICMB_ATTRIBUTES_H */
