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

#ifndef PREDICTION_DATA_H_
#define PREDICTION_DATA_H_

enum { N_PRED_METHODS = 3};

typedef struct pred_method_info {
    int method;
    double method_thres;
    int method_win;
} pred_method_info_t;


typedef struct nrep_pred_params {
    long n_rep_min, n_rep_max, n_rep_stride;

    pred_method_info_t info[N_PRED_METHODS];
    int n_methods;
} nrep_pred_params_t;


typedef struct pred_conditions {
    int n_methods;
    double conditions[N_PRED_METHODS];
} pred_conditions_t;




#endif /* PREDICTION_DATA_H_ */
