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


#ifndef NREP_ESTIMATION_H_
#define NREP_ESTIMATION_H_

#include "prediction_methods/prediction_data.h"
#include "prediction_methods/rse.h"
#include "prediction_methods/cov_mean.h"
#include "prediction_methods/cov_median.h"

enum {
    RSE,
    COV_MEAN,
    COV_MEDIAN
};


extern double (*compute_condition_functions[])(long current_nreps, double* runtimes_sec,
        pred_method_info_t prediction_info);
extern int (*check_condition_functions[])(pred_method_info_t prediction_info, double value);


char* const* get_prediction_methods_list(void);

void set_prediction_conditions(long current_nreps, double* runtimes_sec,
        nrep_pred_params_t prediction_params, pred_conditions_t* conds);
int check_prediction_conditions(nrep_pred_params_t prediction_params, pred_conditions_t conds);

#endif /* NREP_ESTIMATION_H_ */
