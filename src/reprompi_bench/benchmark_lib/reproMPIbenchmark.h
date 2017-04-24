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

#ifndef REPROMPI_BENCHMARK_H_
#define REPROMPI_BENCHMARK_H_

#if defined (ENABLE_WINDOWSYNC_SK)
#define ENABLE_WINDOWSYNC
#undef ENABLE_WINDOWSYNC_NG
#undef ENABLE_WINDOWSYNC_JK
#undef ENABLE_WINDOWSYNC_HCA
#undef ENABLE_GLOBAL_TIMES

#elif defined (ENABLE_WINDOWSYNC_NG)
#define ENABLE_WINDOWSYNC
#undef ENABLE_WINDOWSYNC_SK
#undef ENABLE_WINDOWSYNC_JK
#undef ENABLE_WINDOWSYNC_HCA
#undef ENABLE_GLOBAL_TIMES

#elif defined (ENABLE_WINDOWSYNC_JK)
#define ENABLE_WINDOWSYNC
#undef ENABLE_WINDOWSYNC_NG
#undef ENABLE_WINDOWSYNC_SK
#undef ENABLE_WINDOWSYNC_HCA
#undef ENABLE_GLOBAL_TIMES

#elif defined (ENABLE_WINDOWSYNC_HCA)
#define ENABLE_WINDOWSYNC
#undef ENABLE_WINDOWSYNC_NG
#undef ENABLE_WINDOWSYNC_JK
#undef ENABLE_WINDOWSYNC_SK
#undef ENABLE_GLOBAL_TIMES

#else
#undef ENABLE_WINDOWSYNC
#endif

#ifdef ENABLE_GLOBAL_TIMES
#ifndef ENABLE_WINDOWSYNC
#define ENABLE_WINDOWSYNC
#endif
#endif

#include "reprompi_bench/option_parser/option_parser_data.h"

typedef struct {
    char* testname;
    long n_rep;

    char** user_svars;
    char** user_svar_names;
    int n_user_svars;

    int* user_ivars;
    char** user_ivar_names;
    int n_user_ivars;

} reprompib_job_t;


void reprompib_initialize_benchmark(int argc, char* argv[], reprompib_sync_functions_t* sync_f_p, reprompib_options_t* opts_p);
void reprompib_cleanup_benchmark(reprompib_options_t opts);

void reprompib_print_bench_output(reprompib_job_t job, double* tstart_sec, double* tend_sec,
        reprompib_sync_functions_t sync_f,
        reprompib_options_t opts, char* op, char* timername, char* timertype);


void reprompib_initialize_job(int nrep, reprompib_job_t* job);
void reprompib_cleanup_job(reprompib_job_t job);
void reprompib_add_svar_to_job(char* name, char* s, reprompib_job_t* job_p);
void reprompib_add_ivar_to_job(char* name, int v, reprompib_job_t* job_p);


#endif /* REPROMPI_BENCHMARK_H_ */
