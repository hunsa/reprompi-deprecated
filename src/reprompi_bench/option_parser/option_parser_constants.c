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
#include <stdlib.h>
#include <getopt.h>
#include "option_parser_constants.h"

const struct option reprompi_default_long_options[] = {
        { "repetitions", required_argument, 0, REPROMPI_ARGS_NREPS },
        { "calls-list",required_argument, 0, REPROMPI_ARGS_CALLS_LIST },
        { "msizes-list", required_argument, 0, REPROMPI_ARGS_MSIZES_LIST },
        { "msize-interval", required_argument, 0, REPROMPI_ARGS_MSIZES_INTERVAL },
        { "input-file", required_argument, 0, REPROMPI_ARGS_INPUT_FILE },
        { "output-file", required_argument, 0, REPROMPI_ARGS_OUTPUT_FILE },
        {"root-proc", required_argument, 0, REPROMPI_ARGS_ROOT_PROC},
        {"operation", required_argument, 0, REPROMPI_ARGS_OPERATION},
        {"datatype", required_argument, 0, REPROMPI_ARGS_DATATYPE},
        {"pingpong-ranks", required_argument, 0, REPROMPI_ARGS_PINGPONG_RANKS},
        {"shuffle-jobs", no_argument, 0, REPROMPI_ARGS_SHUFFLE_JOBS},
        {"summary", optional_argument, 0, REPROMPI_ARGS_SUMMARY},
        {"params", optional_argument, 0, REPROMPI_ARGS_PARAMS},

        { "rep-prediction", required_argument, 0, REPROMPI_ARGS_NREPPRED_NREP_LIMITS },
        { "pred-method", required_argument, 0, REPROMPI_ARGS_NREPPRED_PRED_METHOD },
        { "var-thres", required_argument, 0, REPROMPI_ARGS_NREPPRED_VAR_THRES },
        { "var-win", required_argument, 0, REPROMPI_ARGS_NREPPRED_VAR_WIN },

        { "window-size", required_argument, 0, REPROMPI_ARGS_WINSYNC_WIN_SIZE },
        { "fitpoints", required_argument, 0, REPROMPI_ARGS_WINSYNC_NFITPOINTS },
        { "exchanges", required_argument, 0, REPROMPI_ARGS_WINSYNC_NEXCHANGES },
        { "wait-time", required_argument, 0, REPROMPI_ARGS_WINSYNC_WAITTIME },

        { "verbose", no_argument, 0, REPROMPI_ARGS_VERBOSE },
        { "help", no_argument, 0, 0 },
        { 0, 0, 0, 0 }
};
const char reprompi_default_opts_str[] = "vr:h";

