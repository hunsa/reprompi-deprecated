
cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules/")
   
project(reprompi)

set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")


set(INCLUDE_PLATFORM_CONFIG_FILE "${CMAKE_SOURCE_DIR}/platform_files/default.cmake" 
    CACHE STRING "Configure project to use a specific platform file")
include(${INCLUDE_PLATFORM_CONFIG_FILE})

find_package(GSL REQUIRED)
if (GSL_INCLUDE_DIR)
message (STATUS "GSL INCLUDES: ${GSL_INCLUDE_DIR}")
else(GSL_INCLUDE_DIR)
message (STATUS "GSL includes not found. Compiling without GSL support.")
endif(GSL_INCLUDE_DIR)


INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIR} "src")
set(COMMON_LIBRARIES ${GSL_LIBRARIES} )

option(ENABLE_WINDOWSYNC_SK "SKaMPI window-based synchronization [default: MPI_Barrier() synchronization]" off)
option(ENABLE_WINDOWSYNC_JK "Window-based synchronization with linear model of the clock skew [default: MPI_Barrier() synchronization]" off)
option(ENABLE_BENCHMARK_BARRIER "MPI_Barrier implementation [default: MPI_Barrier() synchronization]" off)

if (GSL_INCLUDE_DIR)
    option(ENABLE_WINDOWSYNC_HCA "Window-based synchronization with a hierarchical linear model of the clock skew [default: MPI_Barrier() synchronization]" off)
    option(ENABLE_GLOBAL_TIMES "MPI_Barrier synchronization with HCA-global times [default: local times]" off)
    option(ENABLE_LOGP_SYNC "Enable O(log(p)) clock synchronization for the HCA algorithm [default: disabled]" off)
endif(GSL_INCLUDE_DIR)

option(RDTSC_CALIBRATION "Calibrate the frequency of RTDSC [default: off (uses FREQUENCY_MHZ)]" off)

option(ENABLE_RDTSC "Use RDTSC for time measurements [default: MPI_Wtime()]" off)
option(ENABLE_RDTSCP "Use RDTSCP for time measurements [default: MPI_Wtime()]" off)
option(ENABLE_DOUBLE_BARRIER "Call barrier twice for synchronization [default: disabled]" off)
set(FREQUENCY_MHZ 2300 CACHE STRING "CPU Frequency (needed for RDTSCP-based time measurements)")

option(COMPILE_PRED_BENCHMARK "Enable benchmark with repetitions prediction [default: enabled]" on)
option(COMPILE_BENCH_TESTS "Enable benchmark testing [default: disabled]" off)

#if(EXISTS ${SRC_DIR}/reprompi_bench/sync/netgauge_sync)
#    option(ENABLE_WINDOWSYNC_NG "NetGauge window-based synchronization [default: MPI_Barrier() synchronization]" off)
#endif(EXISTS ${SRC_DIR}/reprompi_bench/sync/netgauge_sync)

option(COMPILE_SANITY_CHECK_TESTS "Enable to compile sanity check code" off)


############ PGtune support ########

set(PGTUNE_LIBRARY_NAME "libpgmpicli" CACHE STRING  "Change the name of the PGtune library [default: libpgmpicli]")
set(PGTUNE_LIBRARY_DEFAULT_PATH "" CACHE STRING  "Specify the path of the PGtune library")
option(ENABLE_PGTUNE "Use PGtune" on)

SET(LIBRARY_SUFFIX "so")
IF(APPLE)
 SET(LIBRARY_SUFFIX "dylib")
ENDIF(APPLE)

if (ENABLE_PGTUNE)
    find_library (PGTUNE_LIBRARY ${PGTUNE_LIBRARY_NAME}.${LIBRARY_SUFFIX}
              PATHS ${PGTUNE_LIBRARY_DEFAULT_PATH}
              PATHS ENV LD_LIBRARY_PATH DYLD_LIBRARY_PATH
              PATH_SUFFIXES lib)
    if (PGTUNE_LIBRARY)
        message(STATUS "PGtune library path: ${PGTUNE_LIBRARY}" )
        set(COMMON_LIBRARIES ${PGTUNE_LIBRARY} ${COMMON_LIBRARIES} )
        
    else(PGTUNE_LIBRARY)
        message(STATUS "PGtune library path: not found" )
        message(STATUS "Compiling without PGtune support" )
    endif()
endif()
########################################

SET(MY_COMPILE_FLAGS "")

if(RDTSC_CALIBRATION)
    SET(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS} -DRDTSC_CALIBRATION")
endif()

if(ENABLE_RDTSC)
    SET(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS} -DENABLE_RDTSC")
endif()

if(ENABLE_RDTSCP)
    SET(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS} -DENABLE_RDTSCP")
endif()

if(FREQUENCY_MHZ)
    SET(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS} -DFREQUENCY_MHZ=${FREQUENCY_MHZ}")
endif()

if(ENABLE_WINDOWSYNC_SK)
    SET(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS} -DENABLE_WINDOWSYNC_SK")
endif()

#if(ENABLE_WINDOWSYNC_NG)
#    SET(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS} -DENABLE_WINDOWSYNC_NG")
#endif()

if(ENABLE_WINDOWSYNC_JK)
    SET(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS} -DENABLE_WINDOWSYNC_JK")
endif()

if(ENABLE_BENCHMARK_BARRIER)
    SET(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS} -DENABLE_BENCHMARK_BARRIER")
endif()

if(ENABLE_DOUBLE_BARRIER)
    SET(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS} -DENABLE_DOUBLE_BARRIER")
endif()


if (GSL_INCLUDE_DIR)
    SET(HCA_FILES
                    "${SRC_DIR}/reprompi_bench/sync/hca_sync/hca_parse_options.c"
                    "${SRC_DIR}/reprompi_bench/sync/hca_sync/hca_sync.c"
                    )

    if(ENABLE_WINDOWSYNC_HCA)
        SET(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS} -DENABLE_WINDOWSYNC_HCA")
    endif()
    if(ENABLE_GLOBAL_TIMES)
        SET(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS} -DENABLE_GLOBAL_TIMES")
    endif()

    if(ENABLE_LOGP_SYNC)
        SET(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS} -DENABLE_LOGP_SYNC")
    endif()
endif(GSL_INCLUDE_DIR)


set(COLL_OPS_SRC_FILES
${SRC_DIR}/collective_ops/collectives.c
${SRC_DIR}/collective_ops/mpi_collectives.c
${SRC_DIR}/collective_ops/mpi_allgather_mockups.c
${SRC_DIR}/collective_ops/mpi_allreduce_mockups.c
${SRC_DIR}/collective_ops/mpi_bcast_mockups.c
${SRC_DIR}/collective_ops/mpi_gather_mockups.c
${SRC_DIR}/collective_ops/mpi_reduce_mockups.c
${SRC_DIR}/collective_ops/mpi_reduce_scatter_mockups.c
${SRC_DIR}/collective_ops/mpi_scan_mockups.c
${SRC_DIR}/collective_ops/mpi_scatter_mockups.c
)

set(COMMON_OPTION_PARSER_SRC_FILES
${SRC_DIR}/reprompi_bench/option_parser/parse_common_options.c
${SRC_DIR}/reprompi_bench/option_parser/option_parser_helpers.c
)

if(EXISTS ${SRC_DIR}/reprompi_bench/sync/netgauge_sync)
    SET(NG_FILES
        ${SRC_DIR}/reprompi_bench/sync/netgauge_sync/ng_parse_options.c
        ${SRC_DIR}/reprompi_bench/sync/netgauge_sync/ng_sync.c
        )
endif(EXISTS ${SRC_DIR}/reprompi_bench/sync/netgauge_sync)

set(SYNC_SRC_FILES
${SRC_DIR}/reprompi_bench/sync/time_measurement.c
${SRC_DIR}/reprompi_bench/sync/synchronization.c
${SRC_DIR}/reprompi_bench/sync/skampi_sync/sk_parse_options.c
${SRC_DIR}/reprompi_bench/sync/skampi_sync/sk_sync.c
${SRC_DIR}/reprompi_bench/sync/joneskoenig_sync/jk_parse_options.c
${SRC_DIR}/reprompi_bench/sync/joneskoenig_sync/jk_sync.c
${SRC_DIR}/reprompi_bench/sync/mpibarrier_sync/barrier_sync.c
${SRC_DIR}/reprompi_bench/sync/benchmark_barrier_sync/bbarrier_sync.c
${HCA_FILES}
${NG_FILES}
)



# generate commit version
find_package(Git)
if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --verify HEAD RESULT_VARIABLE res_var OUTPUT_VARIABLE GIT_COM_ID )
    if( NOT ${res_var} EQUAL 0 )
        set(GIT_COM_ID "unknown")
        message( WARNING "Git failed (not a repo). Build will not contain git revision info." )
    endif()
    string( REPLACE "\n" "" GIT_COMMIT_ID ${GIT_COM_ID} )
else()
    set(GIT_COMMIT_ID "unknown_git_not_found")
    message( WARNING "Git not found. Build will not contain git revision info." )
endif()
configure_file("${SRC_DIR}/version.h.in" "${SRC_DIR}/version.h" @ONLY)


set(COMMON_OUTPUT_MAN_SRC_FILES
${SRC_DIR}/version.h
${SRC_DIR}/reprompi_bench/output_management/bench_info_output.c
${SRC_DIR}/reprompi_bench/output_management/runtimes_computation.c
)


add_executable(mpibenchmark
${SRC_DIR}/benchmarkMPIcalls.c
${SRC_DIR}/benchmark_job.c
${SRC_DIR}/reprompi_bench/misc.c
${SRC_DIR}/reprompi_bench/utils/keyvalue_store.c
# synchronization methods
${SYNC_SRC_FILES}
# output
${COMMON_OUTPUT_MAN_SRC_FILES}
${SRC_DIR}/reprompi_bench/output_management/results_output.c
# command-line options
${COMMON_OPTION_PARSER_SRC_FILES}
${SRC_DIR}/reprompi_bench/option_parser/parse_options.c
# collective functions
${COLL_OPS_SRC_FILES}
)
TARGET_LINK_LIBRARIES(mpibenchmark ${COMMON_LIBRARIES} )
SET_TARGET_PROPERTIES(mpibenchmark PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")



if (COMPILE_SANITY_CHECK_TESTS)
    ADD_SUBDIRECTORY(${SRC_DIR}/sanity_check)
endif()

if (COMPILE_PRED_BENCHMARK)
    ADD_SUBDIRECTORY(${SRC_DIR}/pred_bench)
endif()

if (COMPILE_BENCH_TESTS)
    ADD_SUBDIRECTORY(${SRC_DIR}/test)
endif()


#ADD_SUBDIRECTORY(${SRC_DIR}/reprompi_bench/benchmark_lib)



set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "1")
set(CPACK_SOURCE_GENERATOR "TBZ2")


set(BENCHMARK_NAME ${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH})
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${BENCHMARK_NAME}-src")
set(DIST_ARCHIVE "${BENCHMARK_NAME}-dist")

SET(CPACK_SOURCE_IGNORE_FILES
"/CMakeFiles/"
"/_CPack_Packages/"
"/bin/"
"/include/"
"/lib/"
"/scripts/"
"/create_package.sh"
"/Makefile"
"\\\\.tar\\\\.bz2$"
"/\\\\.git/"
"/\\\\.settings/"
"/\\\\..*"
"CMakeCache.txt"
"cmake_install\\\\.cmake"
"CPackConfig\\\\.cmake"
"CPackSourceConfig\\\\.cmake"
"sources\\\\.org"
"${CPACK_SOURCE_IGNORE_FILES}"
)

INCLUDE(CPack)

add_custom_target(pack COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
add_custom_target(web_package ${CMAKE_HOME_DIRECTORY}/create_package.sh ${CPACK_SOURCE_PACKAGE_FILE_NAME} ${CMAKE_HOME_DIRECTORY}/${DIST_ARCHIVE})


