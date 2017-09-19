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

#ifndef BENCH_INFO_OUTPUT_H_
#define BENCH_INFO_OUTPUT_H_

#include <time.h>
#include "reprompi_bench/utils/keyvalue_store.h"
#include "reprompi_bench/option_parser/parse_common_options.h"
#include "reprompi_bench/sync/synchronization.h"


void print_command_line_args(int argc, char* argv[]);
void print_common_settings(const reprompib_common_options_t* opts, const print_sync_info_t print_sync_info, const reprompib_dictionary_t* dict);
void print_common_settings_to_file(FILE* f, print_sync_info_t print_sync_info, const reprompib_dictionary_t* dict);
void print_benchmark_common_settings_to_file(FILE* f, const reprompib_common_options_t* opts, const print_sync_info_t print_sync_info,
                                             const reprompib_dictionary_t* dict);
void print_final_info(const reprompib_common_options_t* opts, const time_t start_time, const time_t end_time);

#endif /* BENCH_INFO_OUTPUT_H_ */
