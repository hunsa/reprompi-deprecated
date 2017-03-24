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

#ifndef REPROMPI_OPTION_PARSER_CONSTANTS_H_
#define REPROMPI_OPTION_PARSER_CONSTANTS_H_

#include <stdlib.h>

typedef enum reprompi_common_getopt_ids {
  REPROMPI_ARGS_NREPS = 'r',
  REPROMPI_ARGS_VERBOSE = 'v',
  REPROMPI_ARGS_HELP = 'h',
  REPROMPI_ARGS_CALLS_LIST = 500,
  REPROMPI_ARGS_MSIZES_LIST,
  REPROMPI_ARGS_MSIZES_INTERVAL,
  REPROMPI_ARGS_INPUT_FILE,
  REPROMPI_ARGS_OUTPUT_FILE,
  REPROMPI_ARGS_ROOT_PROC,
  REPROMPI_ARGS_OPERATION,
  REPROMPI_ARGS_DATATYPE,
  REPROMPI_ARGS_PINGPONG_RANKS,
  REPROMPI_ARGS_SHUFFLE_JOBS,
  REPROMPI_ARGS_SUMMARY,
  REPROMPI_ARGS_PARAMS
} reprompi_common_getopt_ids_t;


typedef enum reprompi_nrep_pred_getopt_ids {
  REPROMPI_ARGS_NREPPRED_NREP_LIMITS = 600,
  REPROMPI_ARGS_NREPPRED_PRED_METHOD,
  REPROMPI_ARGS_NREPPRED_VAR_THRES,
  REPROMPI_ARGS_NREPPRED_VAR_WIN
} reprompi_nrep_pred_getopt_ids_t;

typedef enum reprompi_win_sync_getopt_ids {
  REPROMPI_ARGS_WINSYNC_WIN_SIZE = 700,
  REPROMPI_ARGS_WINSYNC_NFITPOINTS,
  REPROMPI_ARGS_WINSYNC_NEXCHANGES,
  REPROMPI_ARGS_WINSYNC_WAITTIME
} reprompi_win_sync_getopt_ids_t;

extern const struct option reprompi_default_long_options[];
extern const char reprompi_default_opts_str[];

#endif /* REPROMPI_OPTION_PARSER_CONSTANTS_H_ */
