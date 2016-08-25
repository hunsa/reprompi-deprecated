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

#ifndef TESTBENCH_H_
#define TESTBENCH_H_


extern collective_ops_t collective_calls[];

enum tests {
    TEST_SCATTER_AS_BCAST = 0,
    TEST_ALLGATHER_AS_ALLREDUCE,
    TEST_GATHER_AS_REDUCE,
    TEST_ALLGATHER_GATHERBCAST,
    TEST_BCAST_AS_SCATTERALLGATHER,
    TEST_ALLREDUCE_AS_REDUCEBCAST
};

static const int N_TESTS = 6;


//int test_scatter_as_bcast(int nprocs, test_params_t params);

/*
int (*test_functions[])(int nprocs, test_params_t params) = {
        [TEST_SCATTER_AS_BCAST] = &test_scatter_as_bcast,
        [TEST_ALLGATHER_AS_ALLREDUCE] = &test_scatter_as_bcast,
        [TEST_GATHER_AS_REDUCE] = &test_scatter_as_bcast,
        [TEST_ALLGATHER_GATHERBCAST] = &test_scatter_as_bcast,
        [TEST_BCAST_AS_SCATTERALLGATHER] = &test_scatter_as_bcast,
        [TEST_ALLREDUCE_AS_REDUCEBCAST] = &test_scatter_as_bcast
};
*/



#endif /* TESTBENCH_H_ */



