/*  ReproMPI Benchmark
 *
 *  Copyright 2015 Alexandra Carpen-Amarie, Sascha Hunold
    Research Group for Parallel Computing
    Faculty of Informatics
    Vienna University of Technology, Austria
 *
 * Copyright (c) 2021 Stefan Christians
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

#include "mpi.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "reprompi_bench/sync/sync_info.h"
#include "bbarrier_sync.h"

#include "contrib/intercommunication/intercommunication.h"

inline double bbarrier_get_normalized_time(double local_time) {
    return local_time;
}

void bbarrier_init_synchronization_module(const reprompib_sync_options_t parsed_opts, const long nrep) {
}

void bbarrier_parse_options(int argc, char **argv, reprompib_sync_options_t* opts_p) {
}

void bbarrier_init_synchronization(void) {
    MPI_Barrier(icmb_global_communicator());
}


void dissemination_barrier(void) {
    int my_rank = icmb_global_rank();
    int np = icmb_global_size();
    int send_rank, recv_rank;
    int i, nrounds;
    MPI_Status status;
    int send_value = 1;
    int recv_value = 1;

    nrounds = ceil(log2((double) np));

    for (i = 0; i < nrounds; i++) {
        send_rank = (my_rank + (1<<i)) % np;
        recv_rank = (my_rank - (1 << i) + np) % np;

        //printf("[%d] Sending from %d to %d; receive from %d\n", i, my_rank, send_rank, recv_rank);
        MPI_Sendrecv(&send_value, 1, MPI_INT, send_rank, 0,
                &recv_value, 1, MPI_INT, recv_rank, 0,
                icmb_global_communicator(), &status);
    }

}


void bbarrier_start_synchronization(void) {
    dissemination_barrier();
#ifdef ENABLE_DOUBLE_BARRIER
    dissemination_barrier();
#endif
}

void bbarrier_stop_synchronization(void) {
}

void bbarrier_cleanup_synchronization_module(void) {
}

void bbarrier_print_sync_parameters(FILE* f) {
    fprintf(f, "#@sync=BBarrier\n");
#ifdef ENABLE_DOUBLE_BARRIER
    fprintf(f, "#@doublebarrier=true\n");
#endif
}

