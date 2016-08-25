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

#ifndef REPROMPIB_SYNCHRONIZATION_H_
#define REPROMPIB_SYNCHRONIZATION_H_

typedef int (*init_sync_module_t)(int argc, char* argv[], long nrep);
typedef void (*sync_clocks_t)(void);
typedef void (*init_sync_t)(void);
typedef void (*start_sync_t)(void);
typedef void (*stop_sync_t)(void);
typedef void (*cleanup_sync_t)(void);
typedef int* (*sync_errorcodes_t)(void);
typedef double (*sync_normtime_t)(double local_time);
typedef void (*print_sync_info_t)(FILE* f);
typedef double (*sync_time_t)(void);

typedef struct {
    init_sync_module_t init_sync_module;
    sync_clocks_t sync_clocks;
    init_sync_t init_sync;
    start_sync_t start_sync;
    stop_sync_t stop_sync;
    cleanup_sync_t clean_sync_module;
    sync_normtime_t get_normalized_time;
    sync_errorcodes_t get_errorcodes;
    print_sync_info_t print_sync_info;
    sync_time_t get_time;
} reprompib_sync_functions_t;

void initialize_sync_implementation(reprompib_sync_functions_t *sync_f);

#endif /* REPROMPIB_SYNCHRONIZATION_H_ */
