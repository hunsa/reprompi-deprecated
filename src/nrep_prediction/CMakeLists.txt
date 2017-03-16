

set(NREP_PREDICTION_SRC_FILES
${SRC_DIR}/nrep_prediction/nrep_pred.c
${SRC_DIR}/nrep_prediction/parse_nrep_pred_options.c
${SRC_DIR}/benchmark_job.c
${SRC_DIR}/reprompi_bench/utils/keyvalue_store.c
${SRC_DIR}/reprompi_bench/misc.c
${SYNC_SRC_FILES}
${COLL_OPS_SRC_FILES}
${COMMON_OPTION_PARSER_SRC_FILES}
${SRC_DIR}/reprompi_bench/output_management/bench_info_output.c
${SRC_DIR}/reprompi_bench/output_management/runtimes_computation.c
${SRC_DIR}/reprompi_bench/option_parser/option_parser_helpers.c
)


add_executable(nrep_pred
${NREP_PREDICTION_SRC_FILES}
)
TARGET_LINK_LIBRARIES(nrep_pred ${COMMON_LIBRARIES} )
SET_TARGET_PROPERTIES(nrep_pred PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")


if (ENABLE_PGMPI)
    if (PGMPI_FOUND)
        add_executable(nrep_pred_pgmpicli
        ${NREP_PREDICTION_SRC_FILES}
        )
        TARGET_LINK_LIBRARIES(nrep_pred_pgmpicli ${PGMPI_CLI_LIBRARIES} ${COMMON_LIBRARIES} )
        SET_TARGET_PROPERTIES(nrep_pred_pgmpicli PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")
        
        add_executable(nrep_pred_pgmpituned
        ${NREP_PREDICTION_SRC_FILES}
        )
        TARGET_LINK_LIBRARIES(nrep_pred_pgmpituned ${PGMPI_TUNED_LIBRARIES} ${COMMON_LIBRARIES} )
        SET_TARGET_PROPERTIES(nrep_pred_pgmpituned PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")
    else(PGMPI_FOUND)
        message(STATUS "PGMPI not found. Cannot generate PGMPI-linked binaries for NREP prediction." )
    endif(PGMPI_FOUND)
endif (ENABLE_PGMPI)

