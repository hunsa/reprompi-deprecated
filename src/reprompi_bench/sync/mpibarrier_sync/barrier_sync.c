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
#include <stdlib.h>
#include <stdio.h>

#include "reprompi_bench/sync/sync_info.h"
#include "barrier_sync.h"

#include "contrib/intercommunication/intercommunication.h"

inline double mpibarrier_get_normalized_time(double local_time) {
    return local_time;
}

void mpibarrier_init_synchronization_module(const reprompib_sync_options_t parsed_opts, const long nrep) {
}

void mpibarrier_parse_options(int argc, char **argv, reprompib_sync_options_t* opts_p) {
}

void mpibarrier_init_synchronization(void) {
    MPI_Barrier(icmb_global_communicator());
}

void mpibarrier_start_synchronization(void) {
    MPI_Barrier(icmb_global_communicator());
#ifdef ENABLE_DOUBLE_BARRIER
    MPI_Barrier(icmb_global_communicator());
#endif
}

void mpibarrier_stop_synchronization(void) {
}

void mpibarrier_cleanup_synchronization_module(void) {
}

void mpibarrier_print_sync_parameters(FILE* f) {
    fprintf(f, "#@sync=MPI_Barrier\n");
#ifdef ENABLE_DOUBLE_BARRIER
    fprintf(f, "#@doublebarrier=true\n");
#endif
}

