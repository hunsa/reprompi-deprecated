
set(BENCH_LIB_FILES
${SRC_DIR}/reprompi_bench/benchmark_lib/reproMPIbenchmark.c
${SRC_DIR}/reprompi_bench/benchmark_lib/results_output.c
${SRC_DIR}/reprompi_bench/option_parser/parse_options.c
${SRC_DIR}/reprompi_bench/option_parser/option_parser_helpers.c
${SRC_DIR}/reprompi_bench/misc.c
${SRC_DIR}/reprompi_bench/utils/keyvalue_store.c
)

add_library(reproMPIbench 
${BENCH_LIB_FILES}
${SYNC_SRC_FILES}
${COMMON_OUTPUT_MAN_SRC_FILES}
${COLL_OPS_SRC_FILES}
)


TARGET_LINK_LIBRARIES(reproMPIbench ${COMMON_LIBRARIES} )

SET_TARGET_PROPERTIES(reproMPIbench PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")

install(TARGETS reproMPIbench DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

install(DIRECTORY ${SRC_DIR}/
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include
        FILES_MATCHING PATTERN "*.h"
        PATTERN "CMakeFiles" EXCLUDE
        PATTERN "buf_manager*" EXCLUDE
        PATTERN "collective_ops*" EXCLUDE
        PATTERN "nrep_prediction*" EXCLUDE
        PATTERN "pred_bench*" EXCLUDE
        PATTERN "sanity_check*" EXCLUDE
        PATTERN "tools*" EXCLUDE
        PATTERN "version*" EXCLUDE
        PATTERN "test*" EXCLUDE
        PATTERN "benchmark_job*" EXCLUDE
        PATTERN "benchmarkMPIcalls*" EXCLUDE
        )

#install(DIRECTORY ${SRC_DIR}/ 
#        DESTINATION ${CMAKE_INSTALL_PREFIX}/include
#        FILES_MATCHING PATTERN "*.h"
#        PATTERN "CMakeFiles" EXCLUDE
#        PATTERN "sanity_check" EXCLUDE
#        PATTERN "test*" EXCLUDE
#        PATTERN "benchmark_job*" EXCLUDE
#        PATTERN "benchmarkMPIcalls*" EXCLUDE
#        )
