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
#include "reprompi_bench/option_parser/option_parser_helpers.h"
#include "reprompi_bench/option_parser/parse_options.h"
#include "reprompi_bench/option_parser/parse_common_options.h"
#include "reprompi_bench/option_parser/parse_extra_key_value_options.h"
#include "reprompi_bench/output_management/bench_info_output.h"
#include "reprompi_bench/output_management/runtimes_computation.h"
#include "reprompi_bench/output_management/results_output.h"
#include "collective_ops/collectives.h"
#include "reprompi_bench/utils/keyvalue_store.h"

static const int OUTPUT_ROOT_PROC = 0;
static const int HASHTABLE_SIZE=100;

void print_initial_settings(const reprompib_options_t* opts, const reprompib_common_options_t* common_opts, print_sync_info_t print_sync_info, const reprompib_dictionary_t* dict) {
    int my_rank, np;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &np);

    print_common_settings(common_opts, print_sync_info, dict);

    if (my_rank == OUTPUT_ROOT_PROC) {
        FILE* f;

        f = stdout;
        if (opts->n_rep > 0) {
          fprintf(f, "#@nrep=%ld\n", opts->n_rep);
          if (common_opts->output_file != NULL) {
            f = fopen(common_opts->output_file, "a");
            fprintf(f, "#@nrep=%ld\n", opts->n_rep);
            fflush(f);
            fclose(f);
          }
        }
    }
}


void reprompib_print_bench_output(job_t job, double* tstart_sec, double* tend_sec,
        sync_errorcodes_t get_errorcodes, sync_normtime_t get_global_time,
        const reprompib_options_t* opts, const reprompib_common_options_t* common_opts) {
    FILE* f = stdout;
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    if (my_rank == OUTPUT_ROOT_PROC) {
        if (common_opts->output_file != NULL) {
            f = fopen(common_opts->output_file, "a");
        }
    }

    if (opts->print_summary_methods >0)  {
        print_summary(stdout, job, tstart_sec, tend_sec, get_errorcodes, get_global_time,
                opts->print_summary_methods);
        if (common_opts->output_file != NULL) {
            print_measurement_results(f, job, tstart_sec, tend_sec,
                    get_errorcodes, get_global_time,
                    opts->verbose);
        }

    }
    else {
        print_measurement_results(f, job, tstart_sec, tend_sec,
                get_errorcodes, get_global_time,
                opts->verbose);
    }

    if (my_rank == OUTPUT_ROOT_PROC) {
        if (common_opts->output_file != NULL) {
            fflush(f);
            fclose(f);
        }
    }

}


void reprompib_parse_bench_options(int argc, char** argv) {
    int c;
    opterr = 0;

    const struct option bench_long_options[] = {
        { "help", required_argument, 0, 'h' },
        { 0, 0, 0, 0 }
    };

    while (1) {

        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, "h", bench_long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c) {
        case 'h': /* list of summary options */
            reprompib_print_benchmark_help();
            break;
        case '?':
            break;
        }
    }

    optind = 1; // reset optind to enable option re-parsing
    opterr = 1; // reset opterr to catch invalid options
}



int main(int argc, char* argv[]) {
    int my_rank, procs;
    long i, jindex;
    double* tstart_sec;
    double* tend_sec;
    reprompib_options_t opts;
    reprompib_sync_options_t sync_opts;
    reprompib_common_options_t common_opts;
    job_list_t jlist;
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
    reprompib_init_dictionary(&params_dict, HASHTABLE_SIZE);

    // initialize synchronization functions according to the configured synchronization method
    initialize_sync_implementation(&sync_f);

    // parse arguments and set-up benchmarking jobs
    print_command_line_args(argc, argv);

    reprompib_parse_bench_options(argc, argv);  // only "-h" for help

    // parse common arguments (e.g., msizes list, MPI calls to benchmark, input file)
    reprompib_parse_common_options(&common_opts, argc, argv);

    // parse extra parameters into the global dictionary
    reprompib_parse_extra_key_value_options(&params_dict, argc, argv);

    // parse the benchmark-specific arguments (nreps, summary)
    reprompib_parse_options(&opts, argc, argv);

    // parse the arguments related to the synchronization and timing method
    sync_f.parse_sync_params( argc, argv, &sync_opts);

    if (common_opts.input_file == NULL && opts.n_rep <=0) { // make sure nrep is specified when there is no input file
      reprompib_print_error_and_exit("The number of repetitions is not defined (specify the \"--nrep\" command-line argument or provide an input file)\n");
    }
    generate_job_list(&common_opts, opts.n_rep, &jlist);


    init_collective_basic_info(common_opts, procs, &coll_basic_info);
    // execute the benchmark jobs
    for (jindex = 0; jindex < jlist.n_jobs; jindex++) {
        job_t job;
        job = jlist.jobs[jlist.job_indices[jindex]];

        // start synchronization module
        sync_f.init_sync_module(sync_opts, job.n_rep);

        tstart_sec = (double*) malloc(job.n_rep * sizeof(double));
        tend_sec = (double*) malloc(job.n_rep * sizeof(double));

        if (jindex == 0) {
            print_initial_settings(&opts, &common_opts, sync_f.print_sync_info, &params_dict);
            print_results_header(&opts, common_opts.output_file, opts.verbose);
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
                sync_f.get_normalized_time, &opts, &common_opts);

        free(tstart_sec);
        free(tend_sec);

        collective_calls[job.call_index].cleanup_data(&coll_params);

        sync_f.clean_sync_module();
    }


    end_time = time(NULL);
    print_final_info(&common_opts, start_time, end_time);

    cleanup_job_list(jlist);
    reprompib_free_common_parameters(&common_opts);
    reprompib_free_parameters(&opts);
    reprompib_cleanup_dictionary(&params_dict);

    /* shut down MPI */
    MPI_Finalize();

    return 0;
}
