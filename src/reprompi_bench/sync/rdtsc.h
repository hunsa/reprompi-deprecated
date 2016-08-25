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


#ifndef RDTSC_H_
#define RDTSC_H_

#include <unistd.h>
#include <stdint.h>


typedef struct {
    uint64_t l;
    uint64_t h;
}x86_64_timeval_t;

#define HRT_TIMESTAMP_T x86_64_timeval_t

#define HRT_GET_TIMESTAMP(t1)  __asm__ __volatile__ ("rdtsc" : "=a" (t1.l), "=d" (t1.h));

#define HRT_GET_ELAPSED_TICKS(t1, t2, numptr)	*numptr = (((( uint64_t ) t2.h) << 32) | t2.l) - \
														  (((( uint64_t ) t1.h) << 32) | t1.l);

#define HRT_GET_TIME(t1, time) time = (((( uint64_t ) t1.h) << 32) | t1.l)

#define HRT_CALIBRATE(freq) do {  \
  static volatile HRT_TIMESTAMP_T t1, t2; \
  static volatile uint64_t elapsed_ticks, min = (uint64_t)(~0x1); \
  int notsmaller=0; \
  while(notsmaller<3) { \
    HRT_GET_TIMESTAMP(t1); \
    sleep(1); \
    HRT_GET_TIMESTAMP(t2); \
    HRT_GET_ELAPSED_TICKS(t1, t2, &elapsed_ticks); \
    \
    notsmaller++; \
    if(elapsed_ticks<min) { \
      min = elapsed_ticks; \
      notsmaller = 0; \
    } \
  } \
  freq = min; \
} while(0);

#define HRT_INIT(print, freq) do {\
  if(print) printf("# initializing x86-64 timer (takes some seconds)\n"); \
  HRT_CALIBRATE(freq); \
} while(0)

__inline__ unsigned long long rdtsc(void)
{
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

__inline__ unsigned long long rdtscp(void) {
    unsigned long long tsc;
    __asm__ __volatile__("rdtscp; "         // serializing read of tsc
            "shl $32,%%rdx; "// shift higher 32 bits stored in rdx up
            "or %%rdx,%%rax"// and or onto rax
            : "=a"(tsc)// output to tsc variable
            :
            : "%rcx", "%rdx");// rcx and rdx are clobbered
    return tsc;
}


#endif /* RDTSC_H_ */
