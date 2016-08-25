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
#include <math.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_sort.h>
#include "mpi.h"

#include "prediction_data.h"
#include "pred_helpers.h"
#include "rse.h"

static const int OUTPUT_ROOT_PROC = 0;

double compute_rse(long nreps, double* runtimes_sec,
        pred_method_info_t prediction_info) {
    double rse = COEF_ERROR_VALUE;
    int my_rank, i;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    if (my_rank == OUTPUT_ROOT_PROC) {

        double mean = 0, q1,q3, sd;
        long start_index, end_index, current_nreps;
        double* runtimes;
        double* tmp_runtimes;

        if (nreps <= 0) {
            return COEF_ERROR_VALUE;
        }
        if (nreps == 1) {   // cannot compute rse of one value
            return COEF_ERROR_VALUE;
        }

        tmp_runtimes = (double*)malloc(nreps * sizeof(double));
        for (i=0; i < nreps; i++) {
                  tmp_runtimes[i] = runtimes_sec[i];
        }


        gsl_sort(tmp_runtimes, 1, nreps);

        q1 =  gsl_stats_quantile_from_sorted_data (tmp_runtimes, 1, nreps, 0.25);
        q3 =  gsl_stats_quantile_from_sorted_data (tmp_runtimes, 1, nreps, 0.75);

        runtimes = tmp_runtimes;
        current_nreps = nreps;
        if (nreps > OUTLIER_FILTER_MIN_MEAS) {
            filter_outliers_from_sorted(tmp_runtimes, nreps,
                    q1, q3, OUTLIER_FILTER_THRES, &start_index, &end_index);
            runtimes = runtimes + start_index;
            current_nreps =  end_index - start_index + 1;
        }

        mean = gsl_stats_mean(runtimes, 1, current_nreps);
        sd =  gsl_stats_sd(runtimes, 1, current_nreps);
        rse = sd/(sqrt(current_nreps) * mean);

        //printf("rse=%lf, nreps = %ld, thres=%lf mean=%.10f\n", rse, nreps, prediction_info.method_thres, mean);

        free(tmp_runtimes);
    }

    return rse;
}

int check_rse(pred_method_info_t prediction_info, double value) {

    return (value != COEF_ERROR_VALUE) && (value < prediction_info.method_thres);

}

