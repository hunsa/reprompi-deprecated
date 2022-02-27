


add_executable(verify_clock
verify_clock.c
${SRC_DIR}/reprompi_bench/sync/time_measurement.c
# intercommunication
${INTERCOMM_SOURCE_FILES}
)
SET_TARGET_PROPERTIES(verify_clock PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")



