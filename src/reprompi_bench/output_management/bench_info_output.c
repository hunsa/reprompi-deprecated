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
#include "mpi.h"

#include "benchmarkMPIcalls.h"
#include "reprompi_bench/sync/synchronization.h"
#include "reprompi_bench/sync/time_measurement.h"
#include "reprompi_bench/option_parser/parse_common_options.h"
#include "collective_ops/collectives.h"
#include "reprompi_bench/utils/keyvalue_store.h"
#include "bench_info_output.h"

#include "version.h"

static const int OUTPUT_ROOT_PROC = 0;


char* get_mpi_operation_str (MPI_Op op) {
    if (op == MPI_BAND) return "MPI_BAND";
    if (op == MPI_BOR) return "MPI_BOR";
    if (op == MPI_BAND) return "MPI_LAND";
    if (op == MPI_BOR) return "MPI_LOR";
    if (op == MPI_MAX) return "MPI_MAX";
    if (op == MPI_MIN) return "MPI_MIN";
    if (op == MPI_SUM) return "MPI_SUM";
    if (op == MPI_PROD) return "MPI_PROD";
    return "unknown_mpi_op";
}

void print_command_line_args(int argc, char* argv[]) {
    int my_rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    if (my_rank == OUTPUT_ROOT_PROC) {
        int i;
        printf("#Command-line arguments: ");
        for (i = 0; i < argc; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }
}


void print_common_settings_to_file(FILE* f, reprompib_common_options_t opts, print_sync_info_t print_sync_info,
                                   const reprompib_dictionary_t* dict) {
    int my_rank, np;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    if (my_rank == OUTPUT_ROOT_PROC) {
        reprompib_print_dictionary(dict, f);

        fprintf(f, "#@reproMPIcommitSHA1=%s\n", git_commit);
        fprintf(f, "#@nprocs=%d\n", np);

        if (opts.enable_job_shuffling > 0) {
            fprintf(f, "#@job_shuffling_enabled=%d\n", opts.enable_job_shuffling);
        }
#ifdef ENABLE_GLOBAL_TIMES
        fprintf(f, "#@clocktype=global\n");
#ifdef ENABLE_LOGP_SYNC
        fprintf(f, "#@hcasynctype=logp\n");
#else
        fprintf(f, "#@hcasynctype=linear\n");
#endif
#elif defined(ENABLE_WINDOWSYNC)
        fprintf(f, "#@clocktype=global\n");
#else
        fprintf(f, "#@clocktype=local\n");
#endif
        print_time_parameters(f);
        print_sync_info(f);
    }
}

void print_benchmark_common_settings_to_file(FILE* f, reprompib_common_options_t opts, print_sync_info_t print_sync_info,
                                             const reprompib_dictionary_t* dict) {
    int my_rank, len;
    char type_name[MPI_MAX_OBJECT_NAME];
    MPI_Aint lb, extent;
    int datatypesize;

    MPI_Type_get_name(opts.datatype, type_name, &len);
    MPI_Type_get_extent(opts.datatype, &lb, &extent);
    MPI_Type_size(opts.datatype, &datatypesize);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    if (my_rank == OUTPUT_ROOT_PROC) {
        int i;
        if (opts.n_calls > 0) {
          fprintf(f, "#MPI calls:\n");
          for (i = 0; i < opts.n_calls; i++) {
            fprintf(f, "#\t%s\n", get_call_from_index(opts.list_mpi_calls[i]));
          }
        }
        if (opts.n_msize > 0) {
          fprintf(f, "#Message counts:\n");
          for (i = 0; i < opts.n_msize; i++) {
            fprintf(f, "#\t%ld\n", opts.msize_list[i]);
          }
        }
        fprintf(f, "#@operation=%s\n", get_mpi_operation_str(opts.operation));
        fprintf(f, "#@datatype=%s\n", type_name);
        fprintf(f, "#@datatype_extent_bytes=%zu\n", extent);
        fprintf(f, "#@datatype_size_bytes=%d\n", datatypesize);
        fprintf(f, "#@root_proc=%d\n", opts.root_proc);

        if (opts.pingpong_ranks[0] >=0 && opts.pingpong_ranks[1] >=0) {
          fprintf(f, "#@pingpong_ranks=%d,%d\n", opts.pingpong_ranks[0], opts.pingpong_ranks[1]);
        }
        print_common_settings_to_file(f, opts, print_sync_info, dict);
    }
}


void print_common_settings(reprompib_common_options_t opts, print_sync_info_t print_sync_info,
                           const reprompib_dictionary_t* dict) {
    FILE* f = stdout;
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    print_benchmark_common_settings_to_file(stdout, opts, print_sync_info, dict);
    if (my_rank == OUTPUT_ROOT_PROC) {
        if (opts.output_file != NULL) {
            f = fopen(opts.output_file, "a");
            print_benchmark_common_settings_to_file(f, opts, print_sync_info, dict);
            fflush(f);
            fclose(f);
        }
    }

}



void print_final_info(reprompib_common_options_t opts, time_t start_time, time_t end_time) {
    int my_rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    if (my_rank == OUTPUT_ROOT_PROC) {
        FILE* f;
        f = stdout;
        fprintf (f, "# Benchmark started at %s", asctime (localtime (&start_time)));
        fprintf (f, "# Execution time: %lds\n", end_time-start_time);

        if (opts.output_file != NULL) {
            f = fopen(opts.output_file, "a");
            fprintf (f, "# Benchmark started at %s", asctime (localtime (&start_time)));
            fprintf (f, "# Execution time: %lds\n", end_time-start_time);
            fflush(f);
            fclose(f);
        }
    }

}



