/*  ReproMPI Benchmark
 *
 *  Copyright 2015 Alexandra Carpen-Amarie, Sascha Hunold
    Research Group for Parallel Computing
    Faculty of Informatics
    Vienna University of Technology, Austria

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



#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#include "synchronization.h"
#include "time_measurement.h"

#ifdef ENABLE_WINDOWSYNC_SK
#include "skampi_sync/sk_sync.h"

#elif  ENABLE_WINDOWSYNC_NG
#include "netgauge_sync/ng_sync.h"

#elif  ENABLE_WINDOWSYNC_JK
#include "joneskoenig_sync/jk_sync.h"

#elif ENABLE_WINDOWSYNC_HCA
#include "hca_sync/hca_sync.h"

#elif ENABLE_GLOBAL_TIMES
#include "hca_sync/hca_sync.h"
#endif

#ifdef ENABLE_BENCHMARK_BARRIER
#include "benchmark_barrier_sync/bbarrier_sync.h"
#endif

#ifndef ENABLE_WINDOWSYNC
#include "mpibarrier_sync/barrier_sync.h"
#endif

void no_op(void) {

}


#ifdef ENABLE_WINDOWSYNC_SK
void initialize_sync_implementation(reprompib_sync_functions_t *sync_f)
{
    sync_f->init_sync_module = sk_init_synchronization_module;
    sync_f->init_sync = sk_init_synchronization;
    sync_f->sync_clocks = sk_sync_clocks;
    sync_f->start_sync = sk_start_synchronization;
    sync_f->stop_sync = sk_stop_synchronization;
    sync_f->clean_sync_module = sk_cleanup_synchronization_module;
    sync_f->get_normalized_time = sk_get_normalized_time;
    sync_f->get_errorcodes = sk_get_local_sync_errorcodes;
    sync_f->print_sync_info = sk_print_sync_parameters;
    sync_f->get_time = get_time;
}

#elif ENABLE_WINDOWSYNC_NG
void initialize_sync_implementation(reprompib_sync_functions_t *sync_f)
{
    sync_f->init_sync_module = ng_init_synchronization_module;
    sync_f->init_sync = ng_init_synchronization;
    sync_f->sync_clocks = ng_sync_clocks;
    sync_f->start_sync = ng_start_synchronization;
    sync_f->stop_sync = ng_stop_synchronization;
    sync_f->clean_sync_module = ng_cleanup_synchronization_module;
    sync_f->get_normalized_time = ng_get_normalized_time;
    sync_f->get_errorcodes = ng_get_local_sync_errorcodes;
    sync_f->print_sync_info = ng_print_sync_parameters;
    sync_f->get_time = get_time;
}

#elif ENABLE_WINDOWSYNC_JK
void initialize_sync_implementation(reprompib_sync_functions_t *sync_f)
{
    sync_f->init_sync_module = jk_init_synchronization_module;
    sync_f->init_sync = jk_init_synchronization;
    sync_f->sync_clocks = jk_sync_clocks;
    sync_f->start_sync = jk_start_synchronization;
    sync_f->stop_sync = jk_stop_synchronization;
    sync_f->clean_sync_module = jk_cleanup_synchronization_module;
    sync_f->get_normalized_time = jk_get_normalized_time;
    sync_f->get_errorcodes = jk_get_local_sync_errorcodes;
    sync_f->print_sync_info = jk_print_sync_parameters;
    sync_f->get_time = get_time;
}

#elif ENABLE_WINDOWSYNC_HCA
void initialize_sync_implementation(reprompib_sync_functions_t *sync_f)
{
    sync_f->init_sync_module = hca_init_synchronization_module;
    sync_f->sync_clocks = hca_synchronize_clocks;
    sync_f->init_sync = hca_init_synchronization;
    sync_f->clean_sync_module = hca_cleanup_synchronization_module;
    sync_f->get_normalized_time = hca_get_normalized_time;
    sync_f->get_errorcodes = hca_get_local_sync_errorcodes;

    sync_f->print_sync_info = hca_print_sync_parameters;
    sync_f->start_sync = hca_start_synchronization;
    sync_f->stop_sync = hca_stop_synchronization;
    sync_f->get_time = hca_get_adjusted_time;
}

#elif ENABLE_GLOBAL_TIMES // barrier sync with HCA-global times
void initialize_sync_implementation(reprompib_sync_functions_t *sync_f)
{
    sync_f->init_sync_module = hca_init_synchronization_module;
    sync_f->sync_clocks = hca_synchronize_clocks;
    sync_f->init_sync = hca_init_synchronization;
    sync_f->clean_sync_module = hca_cleanup_synchronization_module;
    sync_f->get_normalized_time = hca_get_normalized_time;
    sync_f->get_errorcodes = hca_get_local_sync_errorcodes;
    sync_f->get_time = hca_get_adjusted_time;

#if ENABLE_BENCHMARK_BARRIER
    sync_f->print_sync_info = bbarrier_print_sync_parameters;
    sync_f->start_sync = bbarrier_start_synchronization;
    sync_f->stop_sync = bbarrier_stop_synchronization;
#else	// MPI_Barrier sync
    sync_f->print_sync_info = mpibarrier_print_sync_parameters;
    sync_f->start_sync = mpibarrier_start_synchronization;
    sync_f->stop_sync = mpibarrier_stop_synchronization;
#endif
}

#elif ENABLE_BENCHMARK_BARRIER
void initialize_sync_implementation(reprompib_sync_functions_t *sync_f)
{
    sync_f->init_sync_module = bbarrier_init_synchronization_module;
    sync_f->sync_clocks = no_op;
    sync_f->init_sync = bbarrier_init_synchronization;
    sync_f->start_sync = bbarrier_start_synchronization;
    sync_f->stop_sync = bbarrier_stop_synchronization;
    sync_f->clean_sync_module = bbarrier_cleanup_synchronization_module;
    sync_f->get_normalized_time = bbarrier_get_normalized_time;
    sync_f->get_errorcodes = NULL;
    sync_f->print_sync_info = bbarrier_print_sync_parameters;
    sync_f->get_time = get_time;
}

#else	// MPI_Barrier
void initialize_sync_implementation(reprompib_sync_functions_t *sync_f) {
    sync_f->init_sync_module = mpibarrier_init_synchronization_module;
    sync_f->sync_clocks = no_op;
    sync_f->init_sync = mpibarrier_init_synchronization;
    sync_f->start_sync = mpibarrier_start_synchronization;
    sync_f->stop_sync = mpibarrier_stop_synchronization;
    sync_f->clean_sync_module = mpibarrier_cleanup_synchronization_module;
    sync_f->get_normalized_time = mpibarrier_get_normalized_time;
    sync_f->get_errorcodes = NULL;
    sync_f->print_sync_info = mpibarrier_print_sync_parameters;
    sync_f->get_time = get_time;
}

#endif
