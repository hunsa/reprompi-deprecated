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
#include <errno.h>

#include "mem_allocation.h"


void* reprompi_calloc(size_t count, size_t elem_size) {
  void *buf = NULL;

#ifdef OPTION_BUFFER_ALIGNMENT
  int is_power_of_two;
  int err;
  size_t align = OPTION_BUFFER_ALIGNMENT;

  assert(align > 0);
  assert(align % sizeof(void*) == 0);   // multiple of sizeof(void*)
  is_power_of_two = !(align & (align-1));
  assert(is_power_of_two);

  err = posix_memalign(&buf, align, count*elem_size);

  if (err == ENOMEM) {
    fprintf(stderr, "Cannot allocate memory with size %zu Bytes\n", count*elem_size);
    exit(1);
  }
  if (err == EINVAL) {
    fprintf(stderr, "Cannot allocate memory with alignment %zu\n", align);
    exit(1);
  }
#else
  errno = 0;
  buf = calloc(count, elem_size);

  if (errno == ENOMEM) {
    fprintf(stderr, "Cannot allocate memory with size %zu Bytes\n", count*elem_size);
    exit(1);
  }
#endif

  return buf;
}
