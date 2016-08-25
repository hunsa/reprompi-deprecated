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

#ifndef MISC_H__
#define MISC_H__
#include <stdint.h>

#define SUCCESS 0ULL
#define ERROR	1ULL

enum {
    ERROR_MPI_CALL_LIST = 1ULL << 0,
    ERROR_MPI_CALL_LIST_EMPTY = 1ULL << 1,
    ERROR_MSIZE_LIST = 1ULL << 2,
    ERROR_MSIZE_LIST_EMPTY = 1ULL << 3,
    ERROR_NREP_NULL = 1ULL << 4,
    ERROR_MSIZE_MIN = 1ULL << 5,
    ERROR_MSIZE_MAX = 1ULL << 6,
    ERROR_MSIZE_STEP = 1ULL << 7,
    ERROR_MSIZE_INTERVAL_UNKNOWN = 1ULL << 8,
    ERROR_MSIZE_INTERVAL_MINMAX = 1ULL << 9,
    ERROR_UNKNOWN_OPTION = 1ULL << 10,
    ERROR_WIN = 1ULL << 11,
    ERROR_FITPOINTS = 1ULL << 12,
    ERROR_EXCHANGES = 1ULL << 13,
    ERROR_BATCH = 1ULL << 14,
    ERROR_NREPS_MIN = 1ULL << 15,
    ERROR_NREPS_MAX = 1ULL << 16,
    ERROR_NREPS_STEP = 1ULL << 17,
    ERROR_NREPS_INTERVAL_UNKNOWN = 1ULL << 18,
    ERROR_NREPS_INTERVAL_MINMAX = 1ULL << 19,
    ERROR_PRED_METHODS_LIST = 1ULL << 20,
    ERROR_PRED_METHODS_LIST_EMPTY = 1ULL <<21,
    ERROR_THRES_LIST = 1ULL << 22,
    ERROR_THRES_LIST_EMPTY = 1ULL << 23,
    ERROR_PRED_WIN_LIST = 1ULL << 24,
    ERROR_PRED_WIN_LIST_EMPTY = 1ULL << 25,
    ERROR_SUMMARY_METHOD = 1ULL << 26,
    ERROR_SUMMARY_METHODS_EMPTY = 1ULL << 27,
    ERROR_KEY_VAL_PARAM = 1ULL << 28,
    ERROR_OUTPUT_FILE_UNAVAILABLE = 1ULL <<29,
    ERROR_DATATYPE = 1ULL << 30,
    ERROR_MPI_OP = 1ULL << 31,
    ERROR_ROOT_PROC = 1ULL << 32
} bench_error_t;

enum {
    FLAG_START_TIME_HAS_PASSED = 0x1,
    FLAG_SYNC_WIN_EXPIRED = 0x2
};


typedef uint64_t reprompib_error_t;

char* get_error_message(reprompib_error_t error);

double repro_min(double a, double b);
double repro_max(double a, double b);
void shuffle(int *array, size_t n);

#endif /* MISC_H__ */
