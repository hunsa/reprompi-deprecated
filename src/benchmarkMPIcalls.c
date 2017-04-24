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

#include "reprompi_bench/misc.h"
#include "reprompi_bench/sync/synchronization.h"
#include "reprompi_bench/sync/time_measurement.h"
#include "benchmark_job.h"
#include "reprompi_bench/option_parser/parse_options.h"
#include "reprompi_bench/option_parser/option_parser_helpers.h"
#include "reprompi_bench/output_management/bench_info_output.h"
#include "reprompi_bench/output_management/runtimes_computation.h"
#include "reprompi_bench/output_management/results_output.h"
#include "collective_ops/collectives.h"
#include "reprompi_bench/utils/keyvalue_store.h"
#include "benchmarkMPIcalls.h"

static const int OUTPUT_ROOT_PROC = 0;

void print_initial_settings(reprompib_options_t opts, print_sync_info_t print_sync_info, const reprompib_dictionary_t* dict) {
    int my_rank, np;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    print_common_settings(opts.common_opt, print_sync_info, dict);

    if (my_rank == OUTPUT_ROOT_PROC) {
        FILE* f;

        f = stdout;
        if (opts.n_rep > 0) {
          fprintf(f, "#@nrep=%ld\n", opts.n_rep);
          if (opts.common_opt.output_file != NULL) {
            f = fopen(opts.common_opt.output_file, "a");
            fprintf(f, "#@nrep=%ld\n", opts.n_rep);
            fflush(f);
            fclose(f);
          }
        }
    }
}


void reprompib_print_bench_output(job_t job, double* tstart_sec, double* tend_sec,
        sync_errorcodes_t get_errorcodes, sync_normtime_t get_global_time,
        reprompib_options_t opts) {
    FILE* f = stdout;
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    if (my_rank == OUTPUT_ROOT_PROC) {
        if (opts.common_opt.output_file != NULL) {
            f = fopen(opts.common_opt.output_file, "a");
        }
    }

    if (opts.n_print_summary_selected >0)  {
        print_summary(stdout, job, tstart_sec, tend_sec, get_errorcodes, get_global_time,
                opts.print_summary_methods);
        if (opts.common_opt.output_file != NULL) {
            print_measurement_results(f, job, tstart_sec, tend_sec,
                    get_errorcodes, get_global_time,
                    opts.common_opt.verbose);
        }

    }
    else {
        print_measurement_results(f, job, tstart_sec, tend_sec,
                get_errorcodes, get_global_time,
                opts.common_opt.verbose);
    }

    if (my_rank == OUTPUT_ROOT_PROC) {
        if (opts.common_opt.output_file != NULL) {
            fflush(f);
            fclose(f);
        }
    }

}



int main(int argc, char* argv[]) {
    int my_rank, procs;
    long i, jindex;
    double* tstart_sec;
    double* tend_sec;
    reprompib_options_t opts;
    job_list_t jlist;
    reprompib_error_t ret;
    collective_params_t coll_params;
    basic_collective_params_t coll_basic_info;
    time_t start_time, end_time;
    reprompib_sync_functions_t sync_f;

    reprompib_dictionary_t params_dict;

    /* start up MPI
     *
     * */
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &procs);

    // initialize time measurement functions
    init_timer();
    start_time = time(NULL);

    //initialize dictionary
    reprompib_init_dictionary(&params_dict);

    // initialize synchronization functions according to the configured synchronization method
    initialize_sync_implementation(&sync_f);

    // parse arguments and set-up benchmarking jobs
    print_command_line_args(argc, argv);

    ret = reprompib_parse_options(&opts, argc, argv, &params_dict);
    reprompib_validate_common_options_or_abort(ret, &(opts.common_opt), reprompib_print_benchmark_help);

    init_collective_basic_info(opts.common_opt, procs, &coll_basic_info);
    generate_job_list(&(opts.common_opt), opts.n_rep, &jlist);


    // execute the benchmark jobs
    for (jindex = 0; jindex < jlist.n_jobs; jindex++) {
        job_t job;
        job = jlist.jobs[jlist.job_indices[jindex]];

        // start synchronization module
        ret = sync_f.init_sync_module(argc, argv, job.n_rep);
        reprompib_validate_common_options_or_abort(ret, &(opts.common_opt), reprompib_print_benchmark_help);
        if (ret != SUCCESS) {
            break;
        }

        tstart_sec = (double*) malloc(job.n_rep * sizeof(double));
        tend_sec = (double*) malloc(job.n_rep * sizeof(double));

        if (jindex == 0) {
            print_initial_settings(opts, sync_f.print_sync_info, &params_dict);
            print_results_header(opts);
        }

        collective_calls[job.call_index].initialize_data(coll_basic_info, job.count, &coll_params);

        // initialize synchronization
        sync_f.sync_clocks();
        sync_f.init_sync();

        // execute MPI call nrep times
        for (i = 0; i < job.n_rep; i++) {
            sync_f.start_sync();

            tstart_sec[i] = sync_f.get_time();
            collective_calls[job.call_index].collective_call(&coll_params);
            tend_sec[i] = sync_f.get_time();

            sync_f.stop_sync();
        }

        //print summarized data
        reprompib_print_bench_output(job, tstart_sec, tend_sec, sync_f.get_errorcodes,
                sync_f.get_normalized_time, opts);

        free(tstart_sec);
        free(tend_sec);

        collective_calls[job.call_index].cleanup_data(&coll_params);

        sync_f.clean_sync_module();
    }


    end_time = time(NULL);
    print_final_info(opts.common_opt, start_time, end_time);

    cleanup_job_list(jlist);
    reprompib_free_parameters(&opts);

    reprompib_cleanup_dictionary(&params_dict);

    /* shut down MPI */
    MPI_Finalize();

    return 0;
}
