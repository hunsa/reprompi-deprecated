/* Inter-Communicator MPI Benchmarking
 * Copyright (c) 2021 Stefan Christians
 * SPDX-License-Identifier: MIT
 */

/*
 * convenience functions to access and interpret attributes
 */

#include "attributes.h"
#include "utilities.h"

/*
 * returns true if the given communicator is an inter-communicator
 */
static int is_intercommunicator(MPI_Comm comm)
{
    int flag;
    MPI_Comm_test_inter(comm, &flag);
    return flag;
}

/*
 * returns the communicator used for benchmarking
 */
MPI_Comm icmb_benchmark_communicator()
{
    // see if a benchmark communicator was defined
    MPI_Comm comm = icmb_get_benchmarkcommunicator_attribute();

    // fall back on MPI_COMM_WORLD
    if (MPI_COMM_NULL == comm)
    {
        return MPI_COMM_WORLD;
    }

    return comm;
}

/*
 * returns the communicator used for synchronization and collecting results
 */
MPI_Comm icmb_global_communicator()
{
    // see if a global communicator was defined
    MPI_Comm comm = icmb_get_globalcommunicator_attribute();

    // fall back on MPI_COMM_WORLD
    if (MPI_COMM_NULL == comm)
    {
        return MPI_COMM_WORLD;
    }

    return comm;
}

/*
 * returns true if the benchmark communicator is an inter-communicator
 */
int icmb_is_intercommunicator()
{
    return is_intercommunicator(icmb_benchmark_communicator());
}

/*
 * returns true if the process belongs to the initiating group
 */
int icmb_is_initiator()
{
    MPI_Comm comm = icmb_benchmark_communicator();

    // all processes are initiators in intra-communicators
    if (!is_intercommunicator(comm))
    {
        return 1;
    }

    return icmb_get_initiator_attribute(comm);
}

/*
 * returns true if the process belongs to the responding group
 */
int icmb_is_responder()
{
    MPI_Comm comm = icmb_benchmark_communicator();

    // all processes are responders in intra-communicators
    if (!is_intercommunicator(comm))
    {
        return 1;
    }

    return !icmb_get_initiator_attribute(comm);
}

/*
 * returns rank of process in benchmark communicator
 */
int icmb_benchmark_rank()
{
    MPI_Comm comm = icmb_benchmark_communicator();
    int rank;
    MPI_Comm_rank(comm, &rank);
    return rank;
}

/*
 * returns rank of process in global communicator
 */
int icmb_global_rank()
{
    MPI_Comm comm = icmb_global_communicator();
    int rank;
    MPI_Comm_rank(comm, &rank);
    return rank;
}

/*
 * returns true if the process has the given rank in the initiator group
 */
int icmb_has_initiator_rank(int rank)
{
    if (!icmb_is_initiator())
    {
        return 0;
    }

    return (rank == icmb_benchmark_rank());
}

/*
 * returns size of global communicator
 */
int icmb_global_size()
{
    MPI_Comm comm = icmb_global_communicator();
    int size;
    MPI_Comm_size(comm, &size);
    return size;
}

/*
 * returns size of local group in benchmark communicator
 *
 * (or size of intra-communicator, if benchmark communicator is an intra-communicator)
 */
int icmb_local_size()
{
    MPI_Comm comm = icmb_benchmark_communicator();
    int size;
    MPI_Comm_size(comm, &size);
    return size;
}

/*
 * returns size of remote group in benchmark communicator
 *
 * (or size of intra-communicator, if benchmark communicator is an intra-communicator)
 */
int icmb_remote_size()
{
    MPI_Comm comm = icmb_benchmark_communicator();
    int size;
    if (is_intercommunicator(comm))
    {
        MPI_Comm_remote_size(comm, &size);
    }
    else
    {
        MPI_Comm_size(comm, &size);
    }
    return size;
}

/*
 * returns size of initiator group
 *
 * convenience function to return the size of the initiator group,
 * regardless of what group this process is in
 */
int icmb_initiator_size()
{
    return icmb_is_initiator() ? icmb_local_size() : icmb_remote_size();
}

/*
 * returns size of responder group
 *
 * convenience function to return the size of the responder group,
 * regardless of what group this process is in
 */
int icmb_responder_size()
{
    return icmb_is_initiator() ? icmb_remote_size() : icmb_local_size();
}

/*
 * returns the larger of initiator or responder sizes
 */
int icmb_larger_size()
{
    return (icmb_local_size() >= icmb_remote_size()) ? icmb_local_size() : icmb_remote_size();
}


/*
 * returns the root value to use in all-to-one or one-to-all inter-communications
 */
int icmb_collective_root(int root)
{
    int collective_root = root;
    if (icmb_is_intercommunicator()) {
        if (icmb_has_initiator_rank(root)) {
            // this process is the collective root
            collective_root = MPI_ROOT;
        }
        else if (icmb_is_initiator()) {
            // this process is not participating
            collective_root = MPI_PROC_NULL;
        }
    }
    return collective_root;
}

/*
 * returns rank in the global communicator
 * matching rank in the benchmark communicator's initiator group
 */
int icmb_lookup_global_rank(int initiator_rank) {
    if (!icmb_is_intercommunicator())
    {
        return initiator_rank;
    }

    int local_ranks[1];
    local_ranks[0] = initiator_rank;

    MPI_Group initiator_group;
    if (icmb_is_initiator())
    {
        MPI_Comm_group(icmb_benchmark_communicator(), &initiator_group);
    }
    else
    {
        MPI_Comm_remote_group(icmb_benchmark_communicator(), &initiator_group);
    }

    MPI_Group global_group;
    MPI_Comm_group(icmb_global_communicator(), &global_group);

    int global_ranks[1];
    MPI_Group_translate_ranks(initiator_group, 1, local_ranks, global_group, global_ranks);

    MPI_Group_free(&initiator_group);
    MPI_Group_free(&global_group);

    return global_ranks[0];
}
