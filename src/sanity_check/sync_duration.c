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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <getopt.h>
#include "mpi.h"

#include "reprompi_bench/sync/synchronization.h"
#include "reprompi_bench/sync/time_measurement.h"
#include "parse_test_options.h"

static const int OUTPUT_ROOT_PROC = 0;

void print_initial_settings(int argc, char* argv[],
        print_sync_info_t print_sync_info) {
    int my_rank;
    FILE* f;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    f = stdout;
    if (my_rank == OUTPUT_ROOT_PROC) {
        int i;
        fprintf(f, "#Command-line arguments: ");
        for (i = 0; i < argc; i++) {
            fprintf(f, " %s", argv[i]);
        }
        fprintf(f, "\n");

        print_time_parameters(f);
        print_sync_info(f);
    }

}


int main(int argc, char* argv[]) {
    int my_rank, nprocs, p;
    int master_rank;
    double runtime_s;
    reprompib_st_opts_t opts;
    reprompib_st_error_t ret;
    FILE* f;

    double *all_runtimes = NULL;
    int dummy_nrep = 1;

    // initialize synchronization functions according to the configured synchronization method
    reprompib_sync_functions_t sync_f;
    initialize_sync_implementation(&sync_f);

    init_timer();

    /* start up MPI */
    MPI_Init(&argc, &argv);
    master_rank = 0;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    if (my_rank == master_rank) {
        all_runtimes = (double*) calloc(nprocs, sizeof(double));
    }

    ret = parse_test_options(&opts, argc, argv);
    //validate_test_options_or_abort(ret, &opts);

    ret = sync_f.init_sync_module(argc, argv, dummy_nrep);
    validate_test_options_or_abort(ret, &opts);
    print_initial_settings(argc, argv, sync_f.print_sync_info);


    runtime_s = get_time();
    init_timer();
    sync_f.sync_clocks();
    sync_f.init_sync();
    runtime_s = get_time() - runtime_s;

    MPI_Gather(&runtime_s, 1, MPI_DOUBLE, all_runtimes, 1, MPI_DOUBLE, 0,
            MPI_COMM_WORLD);

    f = stdout;
    if (my_rank == master_rank) {
        fprintf(f, "p runtime\n");
        for (p = 0; p < nprocs; p++) {
            fprintf(f, "%3d %14.9f\n", p, all_runtimes[p]);
        }

        free(all_runtimes);
    }

    MPI_Finalize();
    return 0;
}
