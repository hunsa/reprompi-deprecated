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

#ifndef REPROMPIB_PARSE_OPTIONS_H_
#define REPROMPIB_PARSE_OPTIONS_H_

#include "parse_common_options.h"
#include "option_parser_data.h"

enum {
    PRINT_MEAN = 0,
    PRINT_MEDIAN,
    PRINT_MIN,
    PRINT_MAX,
    N_SUMMARY_METHODS
};

reprompib_error_t reprompib_parse_options(reprompib_options_t* opts_p, int argc, char** argv, reprompib_dictionary_t* dict);
void reprompib_free_parameters(reprompib_options_t* opts_p);

char* const* get_summary_opts_list(void);

#endif /* REPROMPIB_PARSE_OPTIONS_H_ */
