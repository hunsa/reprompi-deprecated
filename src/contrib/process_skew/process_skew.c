/* ReproMPI Process Skew Measurement
 *
 * Copyright (c) 2021 Stefan Christians
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
   This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/*
 * benchmark for measuring process skew
 */

#include <mpi.h>
#include <stdio.h>
#include <time.h>

#include "contrib/intercommunication/intercommunication.h"

#include "options_parser.h"

int main(int argc, char* argv[])
{
    // start up MPI
    MPI_Init(&argc, &argv);

    // parse command line options to launch inter-communicators
    icmb_parse_intercommunication_options(argc, argv);

    // parse process skew options
    skew_options_t opt;
    parse_process_skew_options(&opt, argc, argv);


    // shut down MPI
    MPI_Finalize();

    return 0;
}
