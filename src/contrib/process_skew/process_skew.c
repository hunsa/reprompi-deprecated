/* ReproMPI Process Skew Measurement
 *
 * Copyright (c) 2021 Stefan Christians
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
   This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/*
 * benchmark for measuring process skew
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "reprompi_bench/option_parser/parse_common_options.h"
#include "reprompi_bench/option_parser/parse_extra_key_value_options.h"
#include "reprompi_bench/option_parser/parse_options.h"
#include "reprompi_bench/sync/benchmark_barrier_sync/bbarrier_sync.h"
#include "reprompi_bench/sync/joneskoenig_sync/jk_parse_options.h"
#include "reprompi_bench/sync/mpibarrier_sync/barrier_sync.h"
#include "reprompi_bench/sync/sync_info.h"
#include "reprompi_bench/sync/synchronization.h"
#include "reprompi_bench/sync/time_measurement.h"
#include "reprompi_bench/sync/joneskoenig_sync/jk_sync.h"

#include "contrib/intercommunication/intercommunication.h"

#include "options_parser.h"
#include "output.h"

static const int HASHTABLE_SIZE=100;


static inline void mpibarrier_start_single()
{
    MPI_Barrier(icmb_global_communicator());
}

static inline void mpibarrier_start_double()
{
    MPI_Barrier(icmb_global_communicator());
    MPI_Barrier(icmb_global_communicator());
}

static inline void bbarrier_start_single()
{
    dissemination_barrier();
}

static inline void bbarrier_start_double()
{
    dissemination_barrier();
    dissemination_barrier();
}

int main(int argc, char* argv[])
{
    // start up MPI
    MPI_Init(&argc, &argv);

    // parse command line options to launch inter-communicators
    icmb_parse_intercommunication_options(argc, argv);

    // initialize time measurement
    init_timer();
    time_t start_time = time(NULL);

    // log command line arguments
    print_command_line(argc, argv);

    // parse process skew options
    skew_options_t skew_options;
    parse_process_skew_options(&skew_options, argc, argv);

    // parse extra parameters into the global dictionary
    reprompib_dictionary_t params_dict;
    reprompib_init_dictionary(&params_dict, HASHTABLE_SIZE);
    reprompib_parse_extra_key_value_options(&params_dict, argc, argv);

    // parse reprompi benchmark arguments (nreps, summary)
    reprompib_options_t benchmark_opts;
    reprompib_parse_options(&benchmark_opts, argc, argv);

    if (!benchmark_opts.n_rep)
    {
        // default is one repetition
        benchmark_opts.n_rep = 1;
    }

    // parse jones-koenig sync options
    reprompib_sync_options_t sync_opts;
    jk_parse_options(argc, argv, &sync_opts);

    // optionally parse mpi barrier sync options
    if (skew_options.use_mpi_barrier)
    {
        mpibarrier_parse_options(argc, argv, &sync_opts);
    }

    // optionally parse dissemination barrier options
    {
        bbarrier_parse_options(argc, argv, &sync_opts);
    }

    // initialize joneskoenig module
    jk_init_synchronization_module(sync_opts, benchmark_opts.n_rep);
    double* tstart_sec = (double*) malloc(benchmark_opts.n_rep * sizeof(double));

    // log settings
    print_settings(&skew_options, &params_dict, &benchmark_opts);
    print_header(&skew_options, &benchmark_opts);

    // select synchronization start function
    start_sync_t sync_func;
    if (skew_options.use_mpi_barrier)
    {
        if (skew_options.use_double_barrier)
        {
            sync_func = mpibarrier_start_double;
        }
        else
        {
            sync_func = mpibarrier_start_single;
        }
    }
    else if (skew_options.use_dissemination_barrier) {
        if (skew_options.use_double_barrier)
        {
            sync_func = bbarrier_start_double;
        }
        else
        {
            sync_func = bbarrier_start_single;
        }
    }
    else
    {
        sync_func = jk_start_synchronization;
    }

    // synchronize clocks
    jk_sync_clocks();
    jk_init_synchronization();


    // run benchmark
    for (int i = 0; i < benchmark_opts.n_rep; ++i)
    {
        // synchronize processes
        sync_func();

        // we are only interested in the start time here,
        // as that shows the process skew coming out of the barrier
        tstart_sec[i] = get_time();

        if (skew_options.use_window)
        {
            jk_stop_synchronization();
        }
    }

    // print benchmark result
    print_result(&skew_options, &benchmark_opts, tstart_sec);

    // shutdown joneskoenig module
    free(tstart_sec);
    jk_cleanup_synchronization_module();

    // shutdown time measurement
    time_t end_time = time(NULL);
    print_final(&skew_options, start_time, end_time);

    // free memory allocations
    reprompib_free_parameters(&benchmark_opts);
    reprompib_cleanup_dictionary(&params_dict);
    free_process_skew_options(&skew_options);

    // shut down MPI
    MPI_Finalize();

    return 0;
}
