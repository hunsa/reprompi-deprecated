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

#ifndef PRED_HELPERS_H_
#define PRED_HELPERS_H_

extern const double OUTLIER_FILTER_THRES;
extern const double OUTLIER_FILTER_MIN_MEAS;
extern const double COEF_ERROR_VALUE;

void filter_outliers_from_sorted(const double* runtimes, const long nrep, const double q1, const double q3,
        const double outlier_filter_thres, long* start_index, long* end_index) ;

#endif /* PRED_HELPERS_H_ */
