/* ReproMPI Process Skew Measurement
 *
 * Copyright (c) 2021 Stefan Christians
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
   This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef SKEW_OPTIONS_PARSER_H
#define SKEW_OPTIONS_PARSER_H

typedef struct {
    int use_window;
    int use_mpi_barrier;
    int use_dissemination_barrier;
    int use_double_barrier;
    char* output_file;
} skew_options_t;

void parse_process_skew_options (skew_options_t* opt, int argc, char** argv);
void free_process_skew_options(skew_options_t* opt);

#endif /* SKEW_OPTIONS_PARSER_H */
