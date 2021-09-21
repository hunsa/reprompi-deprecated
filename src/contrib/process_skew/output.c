/* ReproMPI Process Skew Measurement
 *
 * Copyright (c) 2021 Stefan Christians
 *
 * based on
 * ReproMPI Benchmark results_output.c
   Copyright 2015 Alexandra Carpen-Amarie, Sascha Hunold
   Research Group for Parallel Computing
   Faculty of Informatics
   Vienna University of Technology, Austria
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
   This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <stdio.h>

#include "reprompi_bench/option_parser/parse_extra_key_value_options.h"
#include "reprompi_bench/option_parser/parse_options.h"
#include "reprompi_bench/sync/joneskoenig_sync/jk_sync.h"
#include "reprompi_bench/sync/time_measurement.h"
#include "version.h"

#include "contrib/intercommunication/intercommunication.h"

#include "options_parser.h"

static const int OUTPUT_ROOT_PROC = 0;

void print_header(const skew_options_t* skew_options, const reprompib_options_t* benchmark_options)
{
}

static void print_settings_to_file(FILE* f, const skew_options_t* skew_options, const reprompib_dictionary_t* params_dict, const reprompib_options_t* benchmark_options)
{
    // print_benchmark_common_settings_to_file
    fprintf(f, "#MPI calls:\n");
    fprintf(f, "#\t%s\n", "process_skew");

    // print_common_settings_to_file
    reprompib_print_dictionary(params_dict, f);
    fprintf(f, "#@reproMPIcommitSHA1=%s\n", git_commit);
    if (icmb_is_intercommunicator())
    {
        fprintf(f, "#@localprocs=%d\n", icmb_initiator_size());
        fprintf(f, "#@remoteprocs=%d\n", icmb_responder_size());
    }
    else{
        fprintf(f, "#@nprocs=%d\n", icmb_local_size());
    }
    fprintf(f, "#@clocktype=global\n");
    print_time_parameters(f);
    jk_print_sync_parameters(f);

    // print_initial_settings
    fprintf(f, "#@nrep=%ld\n", benchmark_options->n_rep);

    // process skew options
    if (skew_options->use_window)
    {
        fprintf(f, "#@process_sync=window\n");
    }
    else if (skew_options->use_mpi_barrier)
    {
        fprintf(f, "#@sync=MPI_Barrier\n");
    }
    else if (skew_options->use_dissemination_barrier)
    {
        fprintf(f, "#@sync=BBarrier\n");
    }
    if (skew_options->use_double_barrier)
    {
        fprintf(f, "#@doublebarrier=true\n");
    }
}

void print_settings(const skew_options_t* skew_options, const reprompib_dictionary_t* params_dict, const reprompib_options_t* benchmark_options)
{
    if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC))
    {
        FILE* f = stdout;
        print_settings_to_file(f, skew_options, params_dict, benchmark_options);
        if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC)) {
            if (skew_options->output_file != NULL) {
                f = fopen(skew_options->output_file, "a");
                print_settings_to_file(f, skew_options, params_dict, benchmark_options);
                fflush(f);
                fclose(f);
            }
        }
    }
}


