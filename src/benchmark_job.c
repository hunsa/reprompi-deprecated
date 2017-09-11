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
#include <stdlib.h>
#include <assert.h>
#include "mpi.h"

#include "collective_ops/collectives.h"
#include "reprompi_bench/misc.h"
#include "benchmark_job.h"

static const int LEN_JOB_BATCH = 40;
static const int OUTPUT_ROOT_PROC = 0;
static const int INPUT_ROOT_PROC = 0;

void cleanup_job_list(job_list_t jobs) {
  if (jobs.job_indices) {
    free(jobs.job_indices);
  }
  if (jobs.jobs) {
    free(jobs.jobs);
  }
}

/*
 * Read jobs from input file into jlist
 *
 */
void read_input_jobs(char* file_name, job_list_t* jlist) {
  FILE* file;
  int result = 0;
  int expected_result = 0;
  int len_jobs;
  char mpi_call[100];
  size_t msize;
  long nrep;
  int mpi_call_index;

  jlist->jobs = (job_t*) calloc(LEN_JOB_BATCH, sizeof(job_t));

  len_jobs = 0;
  file = fopen(file_name, "r");
  if (file) {

    while (1) {
      result = fscanf(file, "%s %zu %ld", mpi_call, &msize, &nrep);

      /* number of job parameters: MPI call, msize, nrep */
      expected_result = 3;

      if (result == EOF) {
        break;
      }
      if (result != expected_result) /* incorrectly formatted file */
      {
        fprintf(stderr, "ERROR: Incorrectly formatted input file: %s\n", file_name);
        break;
      }

      //printf("%s %ld %ld\n", mpi_call, count, nrep);

      // fill in job info
      mpi_call_index = get_call_index(mpi_call);
      if (mpi_call_index == -1) {
        /* unknown MPI call - read the next line */
        fprintf(stderr, "ERROR: Unknown MPI call: %s\n", mpi_call);
        continue;
      }
      jlist->jobs[len_jobs].call_index = mpi_call_index;
      jlist->jobs[len_jobs].msize = msize;
      jlist->jobs[len_jobs].n_rep = nrep;

      len_jobs++;

      // increase number of allocated jobs
      if (len_jobs % LEN_JOB_BATCH == 0) {
        jlist->jobs = (job_t*) realloc(jlist->jobs, (len_jobs + LEN_JOB_BATCH) * sizeof(job_t));
      }
    }

    fclose(file);
  } else {
    fprintf(stderr, "ERROR: Cannot open input file: %s\n", file_name);
    exit(0);
  }

  if (len_jobs > 0) {
    jlist->jobs = (job_t*) realloc(jlist->jobs, len_jobs * sizeof(job_t));
  }

  jlist->n_jobs = len_jobs;

}

void generate_job_list(const reprompib_common_options_t *opts, const int predefined_n_rep, job_list_t* jlist) {
  int sizeindex, cindex, i;
  int my_rank;
  int datatypesize;

  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  jlist->n_jobs = 0;
  jlist->job_indices = NULL;
  jlist->jobs = NULL;

  /* read jobs from input file and ignore other options */
  if (opts->input_file != NULL) {
    MPI_Datatype basetypes[] = { MPI_INT, MPI_AINT, MPI_AINT, MPI_LONG};
    int blocks[] =  { 1, 1, 1, 1 };
    MPI_Aint disp[] = {0, sizeof(int), sizeof(size_t) + sizeof(int), 2 *sizeof(size_t) + sizeof(int)};
    MPI_Datatype job_info_dt;

    if (my_rank == INPUT_ROOT_PROC) {
      read_input_jobs(opts->input_file, jlist);
    }
    // send the number of jobs to all processes
    MPI_Bcast(&(jlist->n_jobs), 1, MPI_INT, INPUT_ROOT_PROC, MPI_COMM_WORLD);

    if (jlist->n_jobs > 0) {
      if (my_rank != INPUT_ROOT_PROC) { // processes other than the root need to allocate job lists
        jlist->jobs = (job_t*) calloc(jlist->n_jobs, sizeof(job_t));
      }

      // create datatype to hold a job info
      MPI_Type_create_struct ( 4, blocks, disp, basetypes, &job_info_dt);
      MPI_Type_commit (&job_info_dt);

      // broadcast the job list to all processes
      MPI_Bcast(jlist->jobs, jlist->n_jobs, job_info_dt, INPUT_ROOT_PROC, MPI_COMM_WORLD);

      // free datatype
      MPI_Type_free(&job_info_dt);
    }

  } else {
    /* create job list according to command line options */

    if (opts->n_msize > 0 && opts->n_calls > 0) /* we have jobs to run */
    {
      // create the benchmark jobs
      jlist->jobs = (job_t*) malloc(opts->n_calls * opts->n_msize * sizeof(job_t));

      i = 0;
      for (sizeindex = 0; sizeindex < opts->n_msize; sizeindex++) {
        for (cindex = 0; cindex < opts->n_calls; cindex++) {
          jlist->jobs[i].msize = opts->msize_list[sizeindex];
          jlist->jobs[i].call_index = opts->list_mpi_calls[cindex];
          jlist->jobs[i].n_rep = predefined_n_rep;

          i++;
        }
      }

      jlist->n_jobs = i;
    }
  }

  if (jlist->n_jobs <= 0) {
    reprompib_print_error_and_exit(
        "No experiments defined (specify the \"--calls-list\", \"--msizes-list\" command-line arguments or provide an input file)\n");
  }

  // compute counts for each collective and
  // make sure the message sizes specified at the command line (or in the input file)
  // are multiples of the datatype size
  MPI_Type_size(opts->datatype, &datatypesize);
  for (i = 0; i < jlist->n_jobs; i++) {
    assert(datatypesize > 0);
    jlist->jobs[i].count = jlist->jobs[i].msize / datatypesize;

    if (jlist->jobs[i].count * datatypesize != jlist->jobs[i].msize) {
      if (my_rank == OUTPUT_ROOT_PROC) {
        printf("ERROR: Message size %zu for %s is not a multiple of the datatype size (%d)\n\n", jlist->jobs[i].msize,
            get_call_from_index(jlist->jobs[i].call_index), datatypesize);
      }
      MPI_Finalize();
      exit(0);
    }
  }

  if (jlist->n_jobs > 0) {

    // shuffle the list of job indices
    jlist->job_indices = (int*) malloc(jlist->n_jobs * sizeof(int));

    if (my_rank == OUTPUT_ROOT_PROC) {
      for (i = 0; i < jlist->n_jobs; i++) {
        jlist->job_indices[i] = i;
      }
      if (opts->enable_job_shuffling > 0) {
        shuffle(jlist->job_indices, jlist->n_jobs);
      }

    }
    MPI_Bcast(jlist->job_indices, jlist->n_jobs, MPI_INT, OUTPUT_ROOT_PROC,
    MPI_COMM_WORLD);
  }
}

