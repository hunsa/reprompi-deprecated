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


#ifndef PARSE_TEST_OPTIONS_H_
#define PARSE_TEST_OPTIONS_H_


typedef enum {
    SUCCESS = 0,
    ERROR_NREP_NULL
} reprompib_st_error_t;


typedef struct opt {
    long n_rep; /* --repetitions */
    int steps;  /* --steps */
    char testname[256];
} reprompib_st_opts_t;

reprompib_st_error_t parse_test_options(reprompib_st_opts_t* opts_p, int argc, char **argv);

void validate_test_options_or_abort(reprompib_st_error_t errorcode, reprompib_st_opts_t* opts_p);

#endif /* PARSE_TEST_OPTIONS_H_ */
