


set(PREDICTION_SRC_FILES
${SRC_DIR}/pred_bench/prediction_methods/pred_helpers.c
${SRC_DIR}/pred_bench/prediction_methods/cov_mean.c
${SRC_DIR}/pred_bench/prediction_methods/cov_median.c
${SRC_DIR}/pred_bench/prediction_methods/rse.c
${SRC_DIR}/pred_bench/nrep_estimation.c
)


set(MPIBENCH_PREDNREPS_FILES
benchmarkMPIcallsPred.c
parse_options.c
pred_benchmark_job.c
${SRC_DIR}/reprompi_bench/misc.c
${SRC_DIR}/reprompi_bench/utils/keyvalue_store.c
${PREDICTION_SRC_FILES}
${SYNC_SRC_FILES}
${COMMON_OUTPUT_MAN_SRC_FILES}
${COMMON_OPTION_PARSER_SRC_FILES}
${COLL_OPS_SRC_FILES}
)


add_executable(mpibenchPredNreps
${MPIBENCH_PREDNREPS_FILES}
)
TARGET_LINK_LIBRARIES(mpibenchPredNreps ${COMMON_LIBRARIES} )
SET_TARGET_PROPERTIES(mpibenchPredNreps PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")



if (ENABLE_PGMPI)
    if (PGMPI_FOUND)
        add_executable(mpibenchPredNreps_pgmpicli
        ${MPIBENCH_PREDNREPS_FILES}
        )
        TARGET_LINK_LIBRARIES(mpibenchPredNreps_pgmpicli ${PGMPI_CLI_LIBRARIES} ${COMMON_LIBRARIES} )
        SET_TARGET_PROPERTIES(mpibenchPredNreps_pgmpicli PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")
        
        add_executable(mpibenchPredNreps_pgmpituned
        ${MPIBENCH_PREDNREPS_FILES}
        )
        TARGET_LINK_LIBRARIES(mpibenchPredNreps_pgmpituned ${PGMPI_TUNED_LIBRARIES} ${COMMON_LIBRARIES} )
        SET_TARGET_PROPERTIES(mpibenchPredNreps_pgmpituned PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")
    else(PGMPI_FOUND)
        message(STATUS "PGMPI not found. Cannot generate PGMPI-linked binaries for NREP prediction." )
    endif(PGMPI_FOUND)
endif (ENABLE_PGMPI)

