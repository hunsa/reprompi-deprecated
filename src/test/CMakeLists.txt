

add_executable(testbench
#parse_options.c
#option_parser_helpers.c
testbench.c
${SRC_DIR}/reprompi_bench/sync/benchmark_barrier_sync/bbarrier_sync.c
${COLL_OPS_SRC_FILES}
)
TARGET_LINK_LIBRARIES(testbench ${COMMON_LIBRARIES} )


add_executable(test_dict
test_global_dict.c
${SRC_DIR}/reprompi_bench/misc.c
${SRC_DIR}/reprompi_bench/utils/keyvalue_store.c
)
TARGET_LINK_LIBRARIES(testbench ${COMMON_LIBRARIES} )



if(COMPILE_BENCH_TESTS)
    SET(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS} -DCOMPILE_BENCH_TESTS")
endif()

SET_TARGET_PROPERTIES(testbench PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")




