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

#ifndef REPROMPI_MISC_H__
#define REPROMPI_MISC_H__
#include <stdint.h>


double repro_min(double a, double b);
double repro_max(double a, double b);
void shuffle(int *array, size_t n);

int reprompib_str_to_long(const char *str, long* result);
void reprompib_print_error_and_exit(const char* error_str);

long gcd(long a, long b);
long lcm(long a, long b);

#endif /* REPROMPI_MISC_H__ */
