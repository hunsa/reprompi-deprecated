/*  ReproMPI Benchmark
 *
 * Copyright 2003-2008 Werner Augustin, Lars Diesselberg - SKaMPI   MPI-Benchmark
   Lehrstuhl Informatik fuer Naturwissenschaftler und Ingenieure
   Fakultaet fuer Informatik
   University of Karlsruhe
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

#ifndef SK_SYNC_H_
#define SK_SYNC_H_


typedef struct {
    long n_rep; /* --repetitions */
    double window_size_sec; /* --window-size */

    double wait_time_sec; /* --wait-time */
} sk_options_t;


int sk_init_synchronization_module(int argc, char* argv[], long nrep);
void sk_sync_clocks(void);
void sk_init_synchronization(void);
void sk_start_synchronization(void);
void sk_stop_synchronization(void);
void sk_cleanup_synchronization_module(void);

int* sk_get_local_sync_errorcodes(void);

double sk_get_timediff_to_root(void);
double sk_get_normalized_time(double local_time);

void sk_print_sync_parameters(FILE* f);

double should_wait_till(int counter, double interval, double offset);

void* skampi_malloc(int size);
void error_with_abort(int errorcode, char *fmt, ...);
void mpi_abort(int errorcode);

#endif /* SK_SYNC_H_ */
