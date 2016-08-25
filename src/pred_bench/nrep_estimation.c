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


#include <stdio.h>
#include <stdlib.h>

#include "prediction_methods/prediction_data.h"
#include "nrep_estimation.h"


static char * const pred_methods_opts[] = {
        [RSE] = "rse",
        [COV_MEAN] = "cov_mean",
        [COV_MEDIAN] = "cov_median"
};


double (*compute_condition_functions[])(long current_nreps, double* runtimes_sec,
        pred_method_info_t prediction_info)
        = {
                [RSE] = &compute_rse,
                [COV_MEAN] = &compute_cov_mean,
                [COV_MEDIAN] = &compute_cov_median
};

int (*check_condition_functions[])(pred_method_info_t prediction_info, double value)
        = {
                [RSE] = &check_rse,
                [COV_MEAN] = &check_cov_mean,
                [COV_MEDIAN] = &check_cov_median
};



char* const* get_prediction_methods_list(void) {

    return &(pred_methods_opts[0]);
}

void set_prediction_conditions(long current_nreps, double* runtimes_sec,
        nrep_pred_params_t prediction_params, pred_conditions_t* conds) {

    int i;
    for (i=0; i<prediction_params.n_methods; i++) {
        conds->conditions[i] = compute_condition_functions[prediction_params.info[i].method](
                current_nreps, runtimes_sec, prediction_params.info[i]);
    }
    conds->n_methods = prediction_params.n_methods;

}

int check_prediction_conditions(nrep_pred_params_t prediction_params, pred_conditions_t conds) {

    int i;
    int measurement_done = 1;

    for (i=0; i<prediction_params.n_methods; i++) {
            measurement_done &= check_condition_functions[prediction_params.info[i].method](
                    prediction_params.info[i], conds.conditions[i]);
        }
    return measurement_done;

}





