/* ReproMPI Process Skew Measurement
 *
 * Copyright (c) 2021 Stefan Christians
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
   This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef SKEW_OUTPUT_H
#define SKEW_OUTPUT_H

void print_command_line(int argc, char** argv);
void print_settings(const skew_options_t* skew_options, const reprompib_dictionary_t* params_dict, const reprompib_options_t* benchmark_options);
void print_header(const skew_options_t* skew_options, const reprompib_options_t* benchmark_options);
void print_result(const skew_options_t* skew_options, const reprompib_options_t* benchmark_options, double* tstart_sec);
void print_final(const skew_options_t* skew_options, const time_t start_time, const time_t end_time);

#endif /* SKEW_OUTPUT_H */
