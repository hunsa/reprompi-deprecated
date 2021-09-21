/* ReproMPI Process Skew Measurement
 *
 * Copyright (c) 2021 Stefan Christians
 *
 * based on
 * ReproMPI Benchmark results_output.c
   Copyright 2015 Alexandra Carpen-Amarie, Sascha Hunold
   Research Group for Parallel Computing
   Faculty of Informatics
   Vienna University of Technology, Austria
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
   This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <stdio.h>
#include <time.h>

#include "reprompi_bench/option_parser/parse_extra_key_value_options.h"
#include "reprompi_bench/option_parser/parse_options.h"
#include "reprompi_bench/sync/joneskoenig_sync/jk_sync.h"
#include "reprompi_bench/sync/time_measurement.h"
#include "version.h"

#include "contrib/intercommunication/intercommunication.h"

#include "options_parser.h"
#include "output.h"

static const int OUTPUT_ROOT_PROC = 0;

void print_command_line(int argc, char** argv)
{
    if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC)) {
        int i;
        printf("#Command-line arguments: ");
        for (i = 0; i < argc; i++) {
            printf(" %s", argv[i]);
        }
        printf("\n");
    }
}

void print_header(const skew_options_t* skew_options, const reprompib_options_t* benchmark_options )
{
    if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC))
    {
        FILE * f;

        f = stdout;
        // print summary to stdout
        if ( benchmark_options->print_summary_methods >0)
        {
          int i;
          fprintf(f, "%14s %10s %10s ", "test", "total_nrep", "valid_nrep");

          for (i=0; i<reprompib_get_number_summary_methods(); i++)
          {
            summary_method_info_t* s = reprompib_get_summary_method(i);
            if ( benchmark_options->print_summary_methods & s->mask)
            {
              fprintf(f, "%10s_sec ", s->name);
            }
          }
          fprintf(f, "\n");
        }


        // print results to file (if specified)
        if (skew_options->output_file != NULL)
        {
            f = fopen(skew_options->output_file, "a");
        }

        if (skew_options->output_file != NULL || benchmark_options->print_summary_methods==0)
        {
            if (benchmark_options->verbose)
            {
                // print measurement times for each process
                fprintf(f, "process ");
            }

            if (skew_options->use_window)
            {
                fprintf(f, "%14s %10s %10s ", "test", "nrep", "errorcode");
            }
            else
            {
                fprintf(f, "%14s %10s ", "test", "nrep");
            }

            if (benchmark_options->verbose)
            {
                if (skew_options->use_window)
                {
                    fprintf(f, "%14s %14s %14s %14s \n", "loc_tstart_sec", "loc_tend_sec", "gl_tstart_sec", "gl_tend_sec");
                }
                else
                {
                    fprintf(f,  "%14s %14s \n", "loc_tstart_sec", "loc_tend_sec");
                }
            } else {
                fprintf(f,  "%14s \n", "runtime_sec");
            }
        }

        if (skew_options->output_file != NULL) {
            fflush(f);
            fclose(f);
        }
    }
}

static void print_settings_to_file(FILE* f, const skew_options_t* skew_options, const reprompib_dictionary_t* params_dict, const reprompib_options_t* benchmark_options)
{
    // print_benchmark_common_settings_to_file
    fprintf(f, "#MPI calls:\n");
    fprintf(f, "#\t%s\n", "process_skew");

    // print_common_settings_to_file
    reprompib_print_dictionary(params_dict, f);
    fprintf(f, "#@reproMPIcommitSHA1=%s\n", git_commit);
    if (icmb_is_intercommunicator())
    {
        fprintf(f, "#@localprocs=%d\n", icmb_initiator_size());
        fprintf(f, "#@remoteprocs=%d\n", icmb_responder_size());
    }
    else{
        fprintf(f, "#@nprocs=%d\n", icmb_local_size());
    }
    fprintf(f, "#@clocktype=global\n");
    print_time_parameters(f);
    jk_print_sync_parameters(f);

    // print_initial_settings
    fprintf(f, "#@nrep=%ld\n", benchmark_options->n_rep);

    // process skew options
    if (skew_options->use_window)
    {
        fprintf(f, "#@process_sync=window\n");
    }
    else if (skew_options->use_mpi_barrier)
    {
        fprintf(f, "#@sync=MPI_Barrier\n");
    }
    else if (skew_options->use_dissemination_barrier)
    {
        fprintf(f, "#@sync=BBarrier\n");
    }
    if (skew_options->use_double_barrier)
    {
        fprintf(f, "#@doublebarrier=true\n");
    }
}

void print_settings(const skew_options_t* skew_options, const reprompib_dictionary_t* params_dict, const reprompib_options_t* benchmark_options)
{
    if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC))
    {
        FILE* f = stdout;
        print_settings_to_file(f, skew_options, params_dict, benchmark_options);
        if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC)) {
            if (skew_options->output_file != NULL) {
                f = fopen(skew_options->output_file, "a");
                print_settings_to_file(f, skew_options, params_dict, benchmark_options);
                fflush(f);
                fclose(f);
            }
        }
    }
}

void print_final(const skew_options_t* skew_options, const time_t start_time, const time_t end_time)
{
    if (icmb_has_initiator_rank(OUTPUT_ROOT_PROC)) {
        FILE* f;
        f = stdout;
        fprintf (f, "# Benchmark started at %s", asctime (localtime (&start_time)));
        fprintf (f, "# Execution time: %lds\n", (long int)(end_time-start_time));

        if (skew_options->output_file != NULL) {
            f = fopen(skew_options->output_file, "a");
            fprintf (f, "# Benchmark started at %s", asctime (localtime (&start_time)));
            fprintf (f, "# Execution time: %lds\n", (long int)(end_time-start_time));
            fflush(f);
            fclose(f);
        }
    }
}
