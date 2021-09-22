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

#ifndef JK_SYNC_H_
#define JK_SYNC_H_

#include "reprompi_bench/sync/sync_info.h"

typedef struct {
    long n_rep; /* --repetitions */
    double window_size_sec; /* --window-size */

    int n_fitpoints; /* --fitpoints */
    int n_exchanges; /* --exchanges */

    double wait_time_sec; /* --wait-time */
} reprompi_jk_options_t;

void jk_init_synchronization_module(const reprompib_sync_options_t parsed_opts, const long nrep);
void jk_init_synchronization(void);
void jk_sync_clocks(void);
void jk_start_synchronization(void);
void jk_stop_synchronization(void);
void jk_cleanup_synchronization_module(void);

int* jk_get_local_sync_errorcodes(void);

double jk_get_normalized_time(double local_time);

void jk_print_sync_parameters(FILE* f);

#endif /* JK_SYNC_H_ */
