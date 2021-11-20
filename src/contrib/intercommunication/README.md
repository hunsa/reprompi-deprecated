# Inter-Communicator MPI Benchmarking

Building blocks for extending MPI benchmark suites to use inter-communicators

***

## Fundamentals

### Inter-Commmunicator Construction

To start a benchmark using inter-communicators, following command line options are used, which are mutually exclusive:


| option | description |
| :--- | :--- |
| (none) | The benchmark works as before using `MPI_COMM_WORLD`. |
| `--split[=<n>]` | The benchmark processes are started and then the highest n processes are split into a new group. Both groups are connected by an inter-communicator. If n is not given, processes are split by even and odd rank. |
| `--spawn=<n>` | The benchmark processes are started as master and then spawn n worker processes. Master and worker processes are connected by an inter-communicator. |
| `--connect=<n>` | The benchmark processes are started as client, then n server processes are spawned. The server opens a port and informs it to the client, after which the server is disconnected so that client and server are two independent process groups. The client then connects to the waiting server's port. On successful connection, client and server processes are connected by an inter-communicator. |

example:
```
mpiexec -n 6 mybenchmark --split=4 --option1 --option2 ...
```

The command line options are parsed in `options_parser.c`, and the inter-communicator is constructed by splitting, connecting, or spawning processes in `launchers.c`.

`icmb_parse_intercommunication_options()` should be called as early as possible after `MPI_Init()` so that spawned or connected processes are available to execute subsequent actions in parallel to the original processes.

### Communicators

Benchmarking with inter-communicators involves four different communicators:

 1. Before construction of the inter-communicator, only `MPI_COMM_WORLD` exists.
 2. For operations which are benchmarked, `MPI_COMM_WORLD` must be replaced with the new inter-communicator.
 3. Communication within a group is done using a new local intra-communicator.
 4. Synchronization of clocks and processes as well as gathering of measurement results involves all processes and is therefore done with a new global intra-communicator which spans the processes of both groups.

The communicators are cached as attributes of `MPI_COMM_SELF` and thereby readily available in all functions without the need to change any function signatures.
Attribute management is handled in `attributes.c`.

### Initiators and Responders

Intercommunication differentiates between the group from which a communication is initiated and the group whose processes respond to the communication.

The **initiating** group is what the MPI-standard calls local group for rooted operations, which is:

  * if no inter-communicators are used, the group belonging to `MPI_COMM_WORLD`
  * if processes were split, the group containing the process whose rank is 0 in `MPI_COMM_WORLD`
  * if processes were connected, the client group
  * if processes were spawned, the master group

The **responding** group is what the MPI-standard calls remote group for rooted operations, which is:

  * if no inter-communicators are used, the group belonging to `MPI_COMM_WORLD`
  * if processes were split, the group which does *not* contain the process whose rank is 0 in `MPI_COMM_WORLD`
  * if processes were connected, the server group
  * if processes were spawned, the worker group

Input and output should only be handled by the initiator group.

A process's group membership is inidcated by a flag cached as atribute of the inter-communicator.

### Utility Functions

Following utility functions are provided in `utilities.c` to conveniently access and interpret the communicator attributes:


| Communicators | |
| :--- | :--- |
| `icmb_partial_communicator()` | returns the (intra-)communicator used for communication within the local group |
| `icmb_benchmark_communicator()` | returns the (inter-)communicator used for benchmarking |
| `icmb_is_intercommunicator()` | returns true if the benchmark communicator is an inter-communicator |
| `icmb_intercommunicator_type()` | returns a string indicating how the benchmark communicator was constructed (none, split, spawn, connect) |
| `icmb_global_communicator()` | returns the (intra-)communicator used for synchronization and collecting results |

| Rank and Identity | |
| :--- | :--- |
| `icmb_is_initiator()` | returns true if the process belongs to the initiating group |
| `icmb_is_responder()` | returns true if the process belongs to the responding group |
| `icmb_benchmark_rank()` | returns rank of process in benchmark communicator |
| `icmb_has_initiator_rank(int rank)` | returns true if process has given rank in the initiator group |
| `icmb_has_responder_rank(int rank)` | returns true if process has given rank in the responder group |
| `icmb_collective_root(int initiator_root)` | returns the root value to use in all-to-one or one-to-all inter-communications |
| `icmb_lookup_benchmark_rank(int global_rank)` | returns rank in the benchmark communicator's initiating or responding group matching rank in the global communicator |
| `icmb_global_rank()` | returns rank of process in global communicator |
| `icmb_lookup_global_rank(int initiator_rank)` | returns rank in the global communicator matching rank in the benchmark communicator's initiator group |
| `icmb_lookup_is_initiator (int global_rank)` | returns true if the process identifed by global_rank belongs to the initiating group |
| `icmb_lookup_is_responder (int global_rank)` | returns true if the process identifed by global_rank belongs to the responding group |

| Number of Processes | |
| :--- | :--- |
| `icmb_initiator_size()` | returns size of initiator group |
| `icmb_local_size()` | returns size of local group in benchmark communicator |
| `icmb_lookup_local_size ( int global_rank )` | returns size of the global process's local group in the benchmark communicator |
| `icmb_remote_size()` | returns size of remote group in benchmark communicator |
| `icmb_lookup_remote_size ( int global_rank )` | returns size of the global process's remote group in the benchmark communicator |
| `icmb_responder_size()` | returns size of responder group |
| `icmb_larger_size()` | returns the larger of initiator or responder sizes |
| `icmb_global_size()` | returns size of global communicator (this is the sum of initiator and responder sizes) |
| `icmb_combined_size()` | returns the least common multiple of initiator and responder sizes |
| `icmb_maximum_size()` | returns the product of initiator and responder sizes |


If no inter-communicator has been constructed, returned values are appropriate for `MPI_COMM_WORLD`.

***

## Implementation

### Replace `MPI_COMM_WORLD`

Search and replace all occurences of `MPI_COMM_WORLD` with the utility function returning the appropriate communicator.

### Remove `rank` and `size` calls

While searching for occurences of `MPI_COMM_WORLD`, many snippets like the following can be found which determine the rank and number of processes:

```
int rank, size;
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
MPI_Comm_size(MPI_COMM_WORLD, &size);
...
if (rank==0) {
  printf("size:%d\n", size);
}
```

These should be deleted including the variable declarations, as they have no meaning when using inter-communicators.
In above example, output would be generated twice, once in the initiating group and once in the responding group.

By deleting the variable declarations, the IDE or latest the compiler will conveniently point out those areas where the source code needs to be adjusted (because of references to undeclared symbols).

The references to `rank` and `size` should be replaced with the appropriate utility function.
Above example could be changed as follows:

```
if (icmb_has_initiator_rank(0)) {
  printf("size:%d\n", icmb_local_size());
}
```

### Element Counts and Buffer Sizes

For determining the number of element counts to send or receive, or the sizes of send/receive buffers, consider whether they depend on the number of processes in the own or other group and use the appropriate utility function.

Note that some complex operations are a bit more involved, for example `MPI_Reduce_scatter_block()` requires send buffers of equal size in both groups, so if the groups contain a different number of processes, the least common multiple of process numbers must be used (`icmb_combined_size()`).

### Avoid Measuring Utiltiy Functions
Utility functions are not part of the operation to be measured, and precautions must be taken to avoid their measurement.

In below example, the time to execute `icmb_benchmark_communicator()` would wrongly be included in the measurement of `MPI_Barrier`.

```
starttime = now();
MPI_Barrier(icmb_benchmark_communicator());
endtime = now();
```

Instead, first resolve the communicator and then pass it to the operation as value argument:

```
MPI_Comm comm = icmb_benchmark_communicator();
starttime = now();
MPI_Barrier(comm);
endtime = now();
```

### Exclude Undefined Operations

The MPI-standard does not define MPI_Scan and MPI_Exscan for inter-communicators and they should be excluded from benchmarking when inter-communicators are used.

***

## Compiling

  * Include `intercommunication.h` wherever access to Inter-Communicator MPI Benchmarking extensions is required.
  * Add all Inter-Communicator MPI Benchmarking source files to the build path:
      * attributes.c
      * error_output.c
      * excluded_collectives.c
      * launchers.c
      * options_parser.c
      * utilities.c

