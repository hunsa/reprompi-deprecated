


set(PREDICTION_SRC_FILES
${SRC_DIR}/pred_bench/prediction_methods/pred_helpers.c
${SRC_DIR}/pred_bench/prediction_methods/cov_mean.c
${SRC_DIR}/pred_bench/prediction_methods/cov_median.c
${SRC_DIR}/pred_bench/prediction_methods/rse.c
${SRC_DIR}/pred_bench/nrep_estimation.c
)


add_executable(mpibenchmarkPredNreps
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

TARGET_LINK_LIBRARIES(mpibenchmarkPredNreps ${COMMON_LIBRARIES} )
SET_TARGET_PROPERTIES(mpibenchmarkPredNreps PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")




