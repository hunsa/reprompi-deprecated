/* Inter-Communicator MPI Benchmarking
 * Copyright (c) 2021 Stefan Christians
 * SPDX-License-Identifier: MIT
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "attributes.h"
#include "launchers.h"

static const int MSG_TAG_SPLIT = 2;
static const int MSG_TAG_PORT = 4;

enum icmb_spawn_types
{
    ICMB_SPAWN_TYPE_WORKER = 128,
    ICMB_SPAWN_TYPE_SERVER,
};

/*****************************************************************************/

/*
 * The standard launcher does not create any inter-communicator.
 * Benchmarking is done over a conventional intra-communicator.
 * All processes were started statically.
 * MPI_COMM_WORLD is used for both benchmarking and synchronization.
 */

void icmb_launch_standard()
{
    MPI_Comm benchmark_communicator;
    MPI_Comm_dup(MPI_COMM_WORLD, &benchmark_communicator);
    icmb_set_initiator_attribute(benchmark_communicator);
    icmb_set_benchmarkcommunicator_attribute(benchmark_communicator);
    icmb_set_intercommunicatortype_attribute(benchmark_communicator, ICMB_METHOD_NONE);

    MPI_Comm global_communicator;
    MPI_Comm_dup(MPI_COMM_WORLD, &global_communicator);
    icmb_set_globalcommunicator_attribute(global_communicator);

    MPI_Comm partial_communicator;
    MPI_Comm_dup(MPI_COMM_WORLD, &partial_communicator);
    icmb_set_partialcommunicator_attribute(partial_communicator);
}

/*****************************************************************************/

/*
 * The split launcher creates an inter-communicator by splitting all statically
 * started processes into two groups.
 * The newly created inter-communicator is used for benchmarking, and
 * MPI_COMM_WORLD is used for synchronization.
 */

/*
 * split processes into two groups
 *
 * current group is initiator
 *
 * move num_workers processes into new group
 * new group is responder
 *
 * if num_workers is 0, split the processes
 * by even (initiator) and odd (responder) ranks
 */
void icmb_launch_split(int num_workers)
{
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // if number of workers is given, move requested number of processes into remote group
    // otherwise split by even and odd
    int color = num_workers ? (rank>=(size - num_workers)) : rank % 2;

    // split MPI_COMM_WORLD into two partial communicators
    MPI_Comm partial_communicator;
    MPI_Comm_split(MPI_COMM_WORLD, color, rank, &partial_communicator );
    icmb_set_partialcommunicator_attribute(partial_communicator);

    // create inter-communicator spanning both partial communicators
    MPI_Comm benchmark_communicator;
    if(0 == color)
    {
        int remote_leader = num_workers ? (size - num_workers) : 1;
        MPI_Intercomm_create( partial_communicator, 0, MPI_COMM_WORLD, remote_leader, MSG_TAG_SPLIT, &benchmark_communicator);
        icmb_set_initiator_attribute(benchmark_communicator);
    }
    else
    {
        MPI_Intercomm_create( partial_communicator, 0, MPI_COMM_WORLD, 0, MSG_TAG_SPLIT, &benchmark_communicator);
    }
    icmb_set_benchmarkcommunicator_attribute(benchmark_communicator);
    icmb_set_intercommunicatortype_attribute(benchmark_communicator, ICMB_METHOD_SPLIT);

    // use MPI_COMM_WORLD for global communication
    MPI_Comm global_communicator;
    MPI_Comm_dup(MPI_COMM_WORLD, &global_communicator);
    icmb_set_globalcommunicator_attribute(global_communicator);
}

/*****************************************************************************/

/*
 * The master launcher dynamically spawns worker processes and creates an
 * inter-communicator to communicate with them.
 * The master processes were statically started and are in MPI_COMM_WORLD.
 * The dynamically spawned worker processes will be in their own
 * MPI_COMM_WORLD.
 * The inter-communicator is used for benchmarking.
 * A new global intra-communicator spanning both groups is created for
 * synchronization.
 */

/*
 * spawns workers by running command with argv arguments
 *
 * argv does not include the command itself (usually argv[0])
 * and must end with a NULL element
 */
void icmb_launch_master(int num_workers, char* command, char** argv)
{
    // use our own limited MPI_COMM_WORLD as partial communicator
    MPI_Comm partial_communicator;
    MPI_Comm_dup(MPI_COMM_WORLD, &partial_communicator);
    icmb_set_partialcommunicator_attribute(partial_communicator);

    // spawn workers and create inter-communicator
    MPI_Comm benchmark_communicator;
    MPI_Comm_spawn(command, argv, num_workers, MPI_INFO_NULL, 0, MPI_COMM_WORLD, &benchmark_communicator, MPI_ERRCODES_IGNORE);
    icmb_set_initiator_attribute(benchmark_communicator);
    icmb_set_benchmarkcommunicator_attribute(benchmark_communicator);
    icmb_set_intercommunicatortype_attribute(benchmark_communicator, ICMB_METHOD_SPAWN);

    // communicate type of launch to spawned worker processes
    int rank;
    int root = MPI_PROC_NULL;
    int type = ICMB_SPAWN_TYPE_WORKER;

    MPI_Comm_rank(benchmark_communicator, &rank);
    if (0 == rank)
    {
        root = MPI_ROOT;
    }
    MPI_Bcast(&type, 1, MPI_INT, root, benchmark_communicator);

    // create global intra-communicator collectively with spawned workers
    MPI_Comm global_communicator;
    MPI_Intercomm_merge(benchmark_communicator, 0, &global_communicator);
    icmb_set_globalcommunicator_attribute(global_communicator);
}

/*****************************************************************************/

/*
 * requests communicator to parent
 * if no parent exists, this is not a spawned process
 */
void icmb_launch_spawned()
{
    // try to create inter-communicator to parent
    MPI_Comm parent_communicator;
    MPI_Comm_get_parent(&parent_communicator);

    if (MPI_COMM_NULL != parent_communicator)
    {
        // this is a spawned process
        // ask parent how to launch
        int type;
        MPI_Bcast(&type, 1, MPI_INT, 0, parent_communicator);

        switch (type)
        {
            case ICMB_SPAWN_TYPE_SERVER:
                icmb_launch_server();
                break;

            default:
                icmb_launch_worker();
                break;
        }
    }

}


/*****************************************************************************/

/*
 * The worker launcher requests an inter-communicator to its parent processes
 * (the master).
 * Worker processes have their own MPI_COMM_WORLD which is separate from that
 * of the master processes and can be used for message-passing amongst
 * themselves.
 * But for benchmarking, only the inter-communicator is used.
 * A new global intra-communicator spanning both master and worker groups is
 * created for synchronization.
 */

/*
 * requests communicator to parent
 * if no parent exists, this is not a worker process
 */
void icmb_launch_worker()
{
    // try to create inter-communicator to master
    MPI_Comm benchmark_communicator;
    MPI_Comm_get_parent(&benchmark_communicator);


    if (MPI_COMM_NULL != benchmark_communicator)
    {
        // this is a worker process
        icmb_set_benchmarkcommunicator_attribute(benchmark_communicator);
        icmb_set_intercommunicatortype_attribute(benchmark_communicator, ICMB_METHOD_SPAWN);

        // create global intra-communicator collectively with master
        MPI_Comm global_communicator;
        MPI_Intercomm_merge(benchmark_communicator, 1, &global_communicator);
        icmb_set_globalcommunicator_attribute(global_communicator);

        // use our own limited MPI_COMM_WORLD as partial communicator
        MPI_Comm partial_communicator;
        MPI_Comm_dup(MPI_COMM_WORLD, &partial_communicator);
        icmb_set_partialcommunicator_attribute(partial_communicator);
    }
}

/*****************************************************************************/

/*
 * The client launcher spawns server processes and receives the server's port
 * name, after which the communicator to the server is dissolved.
 * Client and server are therefore independent of eachother.
 * The client then creates an inter-communicator by connecting to the server.
 * The inter-communicator is used for benchmarking.
 * A new global intra-communicator spanning both client and server groups is
 * created for synchronization.
 */

/*
 * spawns servers by running command with argv arguments
 *
 * argv does not include the command itself (usually argv[0])
 * and must end with a NULL element
 */
void icmb_launch_client(int num_servers, char* command, char** argv)
{

    // spawn servers
    MPI_Comm child_communicator;
    MPI_Comm_spawn(command, argv, num_servers, MPI_INFO_NULL, 0, MPI_COMM_WORLD, &child_communicator, MPI_ERRCODES_IGNORE);

    // communicate type of launch to spawned server processes
    int rank;
    int root = MPI_PROC_NULL;
    int type = ICMB_SPAWN_TYPE_SERVER;
    MPI_Comm_rank(child_communicator, &rank);
    if (0 == rank)
    {
        root = MPI_ROOT;
    }
    MPI_Bcast(&type, 1, MPI_INT, root, child_communicator);

    // receive port information from child root
    char port_name[MPI_MAX_PORT_NAME] = "";
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if(0 == rank)
    {
        MPI_Recv(port_name, MPI_MAX_PORT_NAME, MPI_CHAR, 0, MSG_TAG_PORT, child_communicator, MPI_STATUS_IGNORE);
    }

    // disconnect from spawned processes
    MPI_Comm_disconnect(&child_communicator);

    // connect to server and create benchmark communicator
    MPI_Comm benchmark_communicator;
    MPI_Comm_connect(port_name, MPI_INFO_NULL, 0, MPI_COMM_WORLD, &benchmark_communicator);
    icmb_set_initiator_attribute(benchmark_communicator);
    icmb_set_benchmarkcommunicator_attribute(benchmark_communicator);
    icmb_set_intercommunicatortype_attribute(benchmark_communicator, ICMB_METHOD_CONNECT);

    // create global intra-communicator collectively with server
    MPI_Comm global_communicator;
    MPI_Intercomm_merge(benchmark_communicator, 0, &global_communicator);
    icmb_set_globalcommunicator_attribute(global_communicator);

    // use our own limited MPI_COMM_WORLD as partial communicator
    MPI_Comm partial_communicator;
    MPI_Comm_dup(MPI_COMM_WORLD, &partial_communicator);
    icmb_set_partialcommunicator_attribute(partial_communicator);
}

/*****************************************************************************/

/*
 * The server launcher opens a port and communicates it to the parent by whom
 * it was spawned.
 * The communicator to the parent is then dissolved.
 * The now disconnected (independent) server waits for the client to
 * connect.
 * An inter-communicator is created by accepting connections from client
 * processes to the server's port.
 * Client and server processes were therefore independent of eachother before
 * connecting.
 * The inter-communicator is used for benchmarking.
 * A new global intra-communicator spanning both client and server groups is
 * created for synchronization.
 */

void icmb_launch_server()
{
    // try to create inter-communicator to parent
    MPI_Comm parent_communicator;
    MPI_Comm_get_parent(&parent_communicator);


    if (MPI_COMM_NULL != parent_communicator)
    {
        // open port and communicate it to parent
        char port_name[MPI_MAX_PORT_NAME] = "";
        int rank;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        int root = MPI_PROC_NULL;
        if(0 == rank)
        {
            root = MPI_ROOT;
            MPI_Open_port(MPI_INFO_NULL, port_name);
            MPI_Send(port_name, strlen(port_name)+1, MPI_CHAR, 0, MSG_TAG_PORT, parent_communicator);
        }

        // disconnect from parent processes
        MPI_Comm_disconnect(&parent_communicator);

        // accept client connection and create inter-communicator
        MPI_Comm benchmark_communicator;
        MPI_Comm_accept(port_name, MPI_INFO_NULL, 0, MPI_COMM_WORLD, &benchmark_communicator);
        if (0 == rank)
        {
            icmb_set_port_attribute(benchmark_communicator, port_name);
        }
        icmb_set_benchmarkcommunicator_attribute(benchmark_communicator);
        icmb_set_intercommunicatortype_attribute(benchmark_communicator, ICMB_METHOD_CONNECT);

        // create global intra-communicator collectively with clients
        MPI_Comm global_communicator;
        MPI_Intercomm_merge(benchmark_communicator, 0, &global_communicator);
        icmb_set_globalcommunicator_attribute(global_communicator);

        // use our own limited MPI_COMM_WORLD as partial communicator
        MPI_Comm partial_communicator;
        MPI_Comm_dup(MPI_COMM_WORLD, &partial_communicator);
        icmb_set_partialcommunicator_attribute(partial_communicator);
    }
}
