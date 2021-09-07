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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "intercommunication.h"
#include "intercommunication_lib.h"
#include "attributes.h"
#include "launchers.h"

static const int MSG_TAG_SPLIT = 2;

static const char* INTERCOMM_SERVICE_NAME = "reprompi_intercomm_benchmark_service";

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

    MPI_Comm global_communicator;
    MPI_Comm_dup(MPI_COMM_WORLD, &global_communicator);
    icmb_set_globalcommunicator_attribute(global_communicator);
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
    MPI_Comm partialcomm;
    MPI_Comm_split(MPI_COMM_WORLD, color, rank, &partialcomm);

    // create inter-communicator spanning both partial communicators
    MPI_Comm benchmark_communicator;
    if(0 == color)
    {
        int remote_leader = num_workers ? (size - num_workers) : 1;
        MPI_Intercomm_create(partialcomm, 0, MPI_COMM_WORLD, remote_leader, MSG_TAG_SPLIT, &benchmark_communicator);
        icmb_set_initiator_attribute(benchmark_communicator);
    }
    else
    {
        MPI_Intercomm_create(partialcomm, 0, MPI_COMM_WORLD, 0, MSG_TAG_SPLIT, &benchmark_communicator);
    }
    icmb_set_benchmarkcommunicator_attribute(benchmark_communicator);

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
 * spawns works by running command with argv arguments
 *
 * argv does not include the command itself (usually argv[0])
 * and must end with a NULL element
 */
void icmb_launch_master(int num_workers, char* command, char** argv)
{

    // spawn workers and create inter-communicator
    MPI_Comm benchmark_communicator;
    MPI_Comm_spawn(command, argv, num_workers, MPI_INFO_NULL, 0, MPI_COMM_WORLD, &benchmark_communicator, MPI_ERRCODES_IGNORE);
    icmb_set_initiator_attribute(benchmark_communicator);
    icmb_set_benchmarkcommunicator_attribute(benchmark_communicator);

    // create global intra-communicator collectively with spawned workers
    MPI_Comm global_communicator;
    MPI_Intercomm_merge(benchmark_communicator, 0, &global_communicator);
    icmb_set_globalcommunicator_attribute(global_communicator);
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

        // create global intra-communicator collectively with master
        MPI_Comm global_communicator;
        MPI_Intercomm_merge(benchmark_communicator, 1, &global_communicator);
        icmb_set_globalcommunicator_attribute(global_communicator);
    }
}

/*****************************************************************************/

/*
 * The client launcher creates an inter-communicator by connecting client
 * processes to a server listening on the given port.
 * Client and server processes were started statically and independently of
 * eachother.
 * The inter-communicator is used for benchmarking.
 * A new global intra-communicator spanning both client and server groups is
 * created for synchronization.
 */

/*
 * connect to given server port
 *
 * if no port name is given, try looking up the benchmark service
 * from a name server
 */
void icmb_launch_client(char* port_name)
{
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    char my_port_name[MPI_MAX_PORT_NAME];
    strcpy(my_port_name, port_name);

    if(0 == rank)
    {
        if(my_port_name[0] == '\0')
        {
            MPI_Lookup_name(INTERCOMM_SERVICE_NAME, MPI_INFO_NULL, my_port_name);
        }
    }
    MPI_Comm benchmark_communicator;
    MPI_Comm_connect(my_port_name, MPI_INFO_NULL, 0, MPI_COMM_WORLD, &benchmark_communicator);
    icmb_set_initiator_attribute(benchmark_communicator);
    icmb_set_benchmarkcommunicator_attribute(benchmark_communicator);

    // create global intra-communicator collectively with server
    MPI_Comm global_communicator;
    MPI_Intercomm_merge(benchmark_communicator, 0, &global_communicator);
    icmb_set_globalcommunicator_attribute(global_communicator);
}

/*****************************************************************************/

/*
 * The server launcher creates an inter-communicator by accepting connections
 * from client processes to its port.
 * Client and server processes were started statically and independently of
 * eachother.
 * The inter-communicator is used for benchmarking.
 * A new global intra-communicator spanning both client and server groups is
 * created for synchronization.
 */

void icmb_launch_server()
{
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    char port_name[MPI_MAX_PORT_NAME] = "";

    // publish service and open port
    if(0 == rank)
    {
        MPI_Open_port(MPI_INFO_NULL, port_name);
        MPI_Publish_name(INTERCOMM_SERVICE_NAME, MPI_INFO_NULL, port_name);
        // output port name so that it can be manually copy-and-pasted if necessary
        printf("port: %s\n", port_name);
    }

    // create inter-communicator
    MPI_Comm benchmark_communicator;
	MPI_Comm_accept(port_name, MPI_INFO_NULL, 0, MPI_COMM_WORLD, &benchmark_communicator);
    if (0 == rank)
    {
        icmb_set_port_attribute(benchmark_communicator, INTERCOMM_SERVICE_NAME, port_name);
    }
    icmb_set_benchmarkcommunicator_attribute(benchmark_communicator);

    // create global intra-communicator collectively with clients
    MPI_Comm global_communicator;
    MPI_Intercomm_merge(benchmark_communicator, 0, &global_communicator);
    icmb_set_globalcommunicator_attribute(global_communicator);
}


/*****************************************************************************/

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
void set_communicator(MPI_Comm intracommunicator) {

    // make sure we receive an intra-communicator
    // we can not receive inter-communicators because
    // then we do not know which group is the intiator
    // and which is the responder
    int is_intercommunicator;
    MPI_Comm_test_inter(intracommunicator, &is_intercommunicator);
    assert(!is_intercommunicator);

    // communicators are ownd by the application, we only work with duplicates
    MPI_Comm benchmark_communicator;
    MPI_Comm_dup(intracommunicator, &benchmark_communicator);
    icmb_set_initiator_attribute(benchmark_communicator);
    icmb_set_benchmarkcommunicator_attribute(benchmark_communicator);

    MPI_Comm global_communicator;
    MPI_Comm_dup(intracommunicator, &global_communicator);
    icmb_set_globalcommunicator_attribute(global_communicator);
}

/*
 * For benchmarking with inter-communicators, sets the communicators to use for
 * the initiating and responding group, respectively.
 *
 * The inter-communicator will be constructed from these two
 * intra-communicators.
 */
void set_communicators(MPI_Comm initiator, MPI_Comm responder)
{
    // make sure we receive only intra-communicators
    int is_intercommunicator;
    MPI_Comm_test_inter(initiator, &is_intercommunicator);
    assert(!is_intercommunicator);
    MPI_Comm_test_inter(responder, &is_intercommunicator);
    assert(!is_intercommunicator);

    // communicators are ownd by the application, we only work with duplicates
    MPI_Comm benchmark_communicator;
    MPI_Comm_dup(initiator, &benchmark_communicator);
    icmb_set_initiator_attribute(benchmark_communicator);
    icmb_set_benchmarkcommunicator_attribute(benchmark_communicator);

    MPI_Comm global_communicator;
    MPI_Comm_dup(responder, &global_communicator);
    icmb_set_globalcommunicator_attribute(global_communicator);
}
