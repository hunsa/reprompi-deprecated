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

#ifndef PRED_OPTION_PARSER_CONSTANTS_H_
#define PRED_OPTION_PARSER_CONSTANTS_H_

#include <stdlib.h>

static const struct option default_long_options[] = {
        { "calls-list",required_argument, 0, 'a' },
        { "msizes-list", required_argument, 0, 'b' },
        { "msize-interval", required_argument, 0, 'c' },
        { "repetitions", required_argument, 0, 'r' },
        { "input-file", required_argument, 0, 'f' },
        { "verbose", no_argument, 0, 'v' },
        { "help", no_argument, 0, 0 },
        { "rep-prediction", required_argument, 0, 'p' },
        { "pred-method", required_argument, 0, 'd' },
        { "var-thres", required_argument, 0, 't' },
        { "var-win", required_argument, 0, 'w' },
        {"summary", optional_argument, 0, 's'},
        {"params", optional_argument, 0, 'z'},
        {"root-proc", required_argument, 0, '5'},
        {"operation", required_argument, 0, '6'},
        {"datatype", required_argument, 0, '7'},
        {"shuffle-jobs", no_argument, 0, '8'},
        { "output-file", required_argument, 0, '9' },
        { 0, 0, 0, 0 }
};
static const char default_opts_str[] = "vr:f:p:t:d:w:h";

static const struct option sk_long_options[] = {
        { "window-size", required_argument, 0, '1' },
        { "calls-list", required_argument, 0, 'a' },
        { "msizes-list", required_argument, 0, 'b' },
        { "msize-interval", required_argument, 0, 'c' },
        { "repetitions", required_argument, 0, 'r' },
        { "input-file", required_argument, 0, 'f' },
        { "verbose", no_argument, 0, 'v' },
        { "help", no_argument, 0, 0 },
        { "rep-prediction", required_argument, 0, 'p' },
        { "pred-method", required_argument, 0, 'd' },
        { "var-thres", required_argument, 0, 't' },
        { "var-win", required_argument, 0, 'w' },
        {"summary", optional_argument, 0, 's'},
        {"params", optional_argument, 0, 'z'},
        {"root-proc", required_argument, 0, '5'},
        {"operation", required_argument, 0, '6'},
        {"datatype", required_argument, 0, '7'},
        {"shuffle-jobs", no_argument, 0, '8'},
        { "output-file", required_argument, 0, '9' },
        { 0, 0, 0, 0 }
};
static const char sk_opts_str[] = "vr:f:p:t:d:w:h";

static const struct option ng_long_options[] = {
        { "window-size", required_argument, 0, '1' },
        { "calls-list", required_argument, 0, 'a' },
        { "msizes-list", required_argument, 0, 'b' },
        { "msize-interval", required_argument, 0, 'c' },
        { "repetitions", required_argument, 0, 'r' },
        { "input-file", required_argument, 0, 'f' },
        { "verbose", no_argument, 0, 'v' },
        { "rep-prediction", required_argument, 0, 'p' },
        { "pred-method", required_argument, 0, 'd' },
        { "var-thres", required_argument, 0, 't' },
        { "var-win", required_argument, 0, 'w' },
        { "help", no_argument, 0, 0 },
        {"summary", optional_argument, 0, 's'},
        {"params", optional_argument, 0, 'z'},
        {"root-proc", required_argument, 0, '5'},
        {"operation", required_argument, 0, '6'},
        {"datatype", required_argument, 0, '7'},
        {"shuffle-jobs", no_argument, 0, '8'},
        { "output-file", required_argument, 0, '9' },
        { 0, 0, 0, 0 }
};
static const char ng_opts_str[] = "vr:f:p:t:d:w:h";

static const struct option lm_long_options[] = {
        { "window-size", required_argument, 0, '1' },
        { "fitpoints", required_argument, 0, '2' },
        { "exchanges", required_argument, 0, '3' },
        { "calls-list", required_argument, 0, 'a' },
        { "msizes-list", required_argument, 0, 'b' },
        { "msize-interval", required_argument, 0, 'c' },
        { "repetitions", required_argument, 0, 'r' },
        { "input-file", required_argument, 0, 'f' },
        { "verbose", no_argument, 0, 'v' },
        { "help", no_argument, 0, 0 },
        { "rep-prediction", required_argument, 0, 'p' },
        { "pred-method", required_argument, 0, 'd' },
        { "var-thres", required_argument, 0, 't' },
        { "var-win", required_argument, 0, 'w' },
        {"summary", optional_argument, 0, 's'},
        {"params", optional_argument, 0, 'z'},
        {"root-proc", required_argument, 0, '5'},
        {"operation", required_argument, 0, '6'},
        {"datatype", required_argument, 0, '7'},
        {"shuffle-jobs", no_argument, 0, '8'},
        { "output-file", required_argument, 0, '9' },
        { 0, 0, 0, 0 }
};
static const char lm_opts_str[] = "vr:f:p:t:d:w:h";

#endif /* PRED_OPTION_PARSER_CONSTANTS_H_ */
