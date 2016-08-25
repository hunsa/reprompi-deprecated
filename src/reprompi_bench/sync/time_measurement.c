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
#include "mpi.h"

#if defined ENABLE_RDTSCP || defined ENABLE_RDTSC
#include "rdtsc.h"
#endif

#include "time_measurement.h"

#if defined ENABLE_RDTSCP || defined ENABLE_RDTSC
#ifdef RDTSC_CALIBRATION
static double FREQ_HZ=0;
#elif FREQUENCY_MHZ		// Do not calibrate, set frequency to a fixed value
const double FREQ_HZ=FREQUENCY_MHZ*1.0e6;
#else
const double FREQ_HZ=2300*1.0e6;
#endif
#endif

void init_timer(void) {
#ifdef RDTSC_CALIBRATION
    uint64_t timerfreq = 0;
    HRT_INIT(0 /* do not print */, timerfreq);
    FREQ_HZ = (double)timerfreq;
#endif
}

inline double get_time(void) {
#ifdef ENABLE_RDTSCP
    return (double)rdtscp()/FREQ_HZ;
#elif ENABLE_RDTSC
    return (double)rdtsc()/FREQ_HZ;
#else
    return MPI_Wtime();
#endif
}

void print_time_parameters(FILE* f) {
    char clock[10];

    strcpy(clock, "MPI_Wtime");
#ifdef ENABLE_RDTSCP
    strcpy(clock, "RDTSCP");
    fprintf(f, "#@frequency_hz=%lf\n", FREQ_HZ);
#elif ENABLE_RDTSC
    strcpy(clock, "RDTSC");
    fprintf(f, "#@frequency_hz=%lf\n", FREQ_HZ);
#endif
    fprintf(f, "#@clock=%s\n", clock);
}
