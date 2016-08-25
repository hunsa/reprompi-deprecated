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
#include "cov_median.h"

static const int OUTPUT_ROOT_PROC = 0;

double compute_cov_median(long nreps, double* runtimes_sec,
        pred_method_info_t prediction_info) {
    int my_rank, i,j;
    double* median_list = NULL;
    int nmedians;
    long current_nreps;
    double* tmp_runtimes;
    double cov_median = COEF_ERROR_VALUE;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    if (my_rank == OUTPUT_ROOT_PROC) {
        double mean_of_medians = 0, sd;
        //double q1,q3;
        //long start_index, end_index;
        double* runtimes;

        nmedians = prediction_info.method_win;
        if (nmedians > nreps) {
            return COEF_ERROR_VALUE;
        }

        median_list = (double*)malloc(prediction_info.method_win * sizeof(double));
        tmp_runtimes = (double*)malloc(nreps * sizeof(double));

        for (i = 0; i < nmedians; i++) {
            current_nreps = nreps - i;

            for (j=0; j < current_nreps; j++) {
                tmp_runtimes[j] = runtimes_sec[j];
            }

            runtimes = tmp_runtimes;
            gsl_sort(tmp_runtimes, 1, current_nreps);
            //if (current_nreps > OUTLIER_FILTER_MIN_MEAS) {
            //            q1 =  gsl_stats_quantile_from_sorted_data (tmp_runtimes, 1, current_nreps, 0.25);
            //            q3 =  gsl_stats_quantile_from_sorted_data (tmp_runtimes, 1, current_nreps, 0.75);

            //            filter_outliers_from_sorted(tmp_runtimes, current_nreps, q1, q3, OUTLIER_FILTER_THRES, &start_index, &end_index);
            //            runtimes = runtimes + start_index;
            //            current_nreps =  end_index - start_index + 1;
            //        }
            //median_list[i] = gsl_stats_quantile_from_sorted_data (runtimes, 1, current_nreps, 0.5);
            median_list[i] = gsl_stats_quantile_from_sorted_data (tmp_runtimes, 1, current_nreps, 0.5);
        }

        mean_of_medians = gsl_stats_mean(median_list, 1, nmedians);
        sd =  gsl_stats_sd(median_list, 1, nmedians);
        cov_median = sd/(mean_of_medians);

        //printf("cov_median=%lf, nreps = %ld, thres=%lf (mean_of_medians=%.10f)\n", cov_median, nreps, prediction_info.method_thres, mean_of_medians);


        free(tmp_runtimes);
        free(median_list);
    }

    return cov_median;
}


int check_cov_median(pred_method_info_t prediction_info, double value) {

    return (value != COEF_ERROR_VALUE) && (value < prediction_info.method_thres);

}




