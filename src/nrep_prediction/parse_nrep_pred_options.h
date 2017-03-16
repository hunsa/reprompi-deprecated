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

#ifndef REPROMPI_NREP_PRED_PARSE_OPTIONS_H_
#define REPROMPI_NREP_PRED_PARSE_OPTIONS_H_

typedef struct pred_opt {

  long min_nrep;
  long max_nrep;

  double threshold;
  double time_limit_s;
  int* nrep_per_pred_round;
  int n_pred_rounds;
} nrep_pred_options_t;

void nrep_pred_free_params(nrep_pred_options_t* opts_p);

void nrep_pred_parse_params(int argc, char** argv, nrep_pred_options_t* opts_p);

void nrep_pred_print_cli_args_to_file(const char* filename, const nrep_pred_options_t* opts);

#endif /* REPROMPI_NREP_PRED_PARSE_OPTIONS_H_ */
