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
#include <time.h>

#include "reprompi_bench/sync/time_measurement.h"
#include "reprompi_bench/output_management/bench_info_output.h"
#include "reprompi_bench/option_parser/parse_common_options.h"
#include "reprompi_bench/option_parser/parse_extra_key_value_options.h"
#include "reprompi_bench/option_parser/parse_options.h"

#include "contrib/intercommunication/intercommunication.h"

#include "options_parser.h"

static const int HASHTABLE_SIZE=100;

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
    print_command_line_args(argc, argv);

    // parse process skew options
    skew_options_t skew_options;
    parse_process_skew_options(&skew_options, argc, argv);

    // we only use the output file parameter from common options,
    // so instead of parsing all common options, we get the
    // output file from above process skew options
    // and set the parameter in common options manually
    reprompib_common_options_t common_opts;
    common_opts.output_file = skew_options.output_file;

    // parse extra parameters into the global dictionary
    reprompib_dictionary_t params_dict;
    reprompib_init_dictionary(&params_dict, HASHTABLE_SIZE);
    reprompib_parse_extra_key_value_options(&params_dict, argc, argv);

    // parse reprompi benchmark arguments (nreps, summary)
    reprompib_options_t benchmark_opts;
    reprompib_parse_options(&benchmark_opts, argc, argv);


    // shutdown time measurement
    time_t end_time = time(NULL);
    print_final_info(&common_opts, start_time, end_time);

    // free memory allocations
    free_process_skew_options(&skew_options);

    // shut down MPI
    MPI_Finalize();

    return 0;
}
