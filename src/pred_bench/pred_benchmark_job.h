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

#ifndef PRED_BENCHMARK_JOB_H_
#define PRED_BENCHMARK_JOB_H_

#include "parse_options.h"


typedef struct {
    int call_index;
    size_t count;
    size_t msize;
    long n_rep;
} job_t;


typedef struct {

    job_t* jobs; /* list of jobs */
    int n_jobs; /* number of jobs */
    int* job_indices; /* list of shuffled job indices for random-order execution */

    nrep_pred_params_t prediction_params;
} pred_job_list_t;

void generate_pred_job_list(pred_options_t opts, pred_job_list_t* jlist);
void cleanup_pred_job_list(pred_job_list_t jobs);

#endif /* PRED_BENCHMARK_JOB_H_ */
