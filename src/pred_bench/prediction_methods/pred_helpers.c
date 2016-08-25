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
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_sort.h>

#include "pred_helpers.h"

const double COEF_ERROR_VALUE = -1;
const double OUTLIER_FILTER_THRES = 1.5;
const double OUTLIER_FILTER_MIN_MEAS = 10;

void filter_outliers_from_sorted(const double* runtimes, const long nrep, const double q1, const double q3,
        const double outlier_filter_thres,
        long* start_index, long* end_index) {

    long i;
    double lvalue, uvalue;
    lvalue = q1 - (q3-q1) * outlier_filter_thres;
    uvalue = q3 + (q3-q1) * outlier_filter_thres;

    *start_index = 0;
    for (i=0; i<nrep; i++) {
        if (runtimes[i] >= lvalue) {
            *start_index = i;
            break;
        }
    }

    *end_index = 0;
    for (i=nrep-1; i>=0; i--) {
        if (runtimes[i] <= uvalue) {
            *end_index = i;
            break;
        }
    }

}

