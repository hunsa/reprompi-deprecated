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

#ifndef BENCHMARKMPICALLS_H_
#define BENCHMARKMPICALLS_H_

#if defined (ENABLE_WINDOWSYNC_SK)
#define ENABLE_WINDOWSYNC
#undef ENABLE_WINDOWSYNC_NG
#undef ENABLE_WINDOWSYNC_JK
#undef ENABLE_WINDOWSYNC_HCA
#undef ENABLE_GLOBAL_TIMES

#elif defined (ENABLE_WINDOWSYNC_NG)
#define ENABLE_WINDOWSYNC
#undef ENABLE_WINDOWSYNC_SK
#undef ENABLE_WINDOWSYNC_JK
#undef ENABLE_WINDOWSYNC_HCA
#undef ENABLE_GLOBAL_TIMES

#elif defined (ENABLE_WINDOWSYNC_JK)
#define ENABLE_WINDOWSYNC
#undef ENABLE_WINDOWSYNC_NG
#undef ENABLE_WINDOWSYNC_SK
#undef ENABLE_WINDOWSYNC_HCA
#undef ENABLE_GLOBAL_TIMES

#elif defined (ENABLE_WINDOWSYNC_HCA)
#define ENABLE_WINDOWSYNC
#undef ENABLE_WINDOWSYNC_NG
#undef ENABLE_WINDOWSYNC_JK
#undef ENABLE_WINDOWSYNC_SK
#undef ENABLE_GLOBAL_TIMES

#else
#undef ENABLE_WINDOWSYNC
#endif

#ifdef ENABLE_GLOBAL_TIMES
#ifndef ENABLE_WINDOWSYNC
#define ENABLE_WINDOWSYNC
#endif
#endif

#include "collective_ops/collectives.h"

extern collective_ops_t collective_calls[];

#endif /* BENCHMARKMPICALLS_H_ */
