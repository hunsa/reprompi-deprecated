
add_executable(compare_clocks
compare_clocks.c
parse_test_options.c
${SRC_DIR}/reprompi_bench/misc.c
${SRC_DIR}/reprompi_bench/sync/time_measurement.c
)

add_executable(measure_rtt
measure_rtt.c
${SRC_DIR}/reprompi_bench/misc.c
${SRC_DIR}/reprompi_bench/sync/time_measurement.c
)

set(MEASURE_CLOCK_DRIFT
measure_clock_drift.c
parse_test_options.c
${SRC_DIR}/reprompi_bench/option_parser/option_parser_constants.c
${SRC_DIR}/reprompi_bench/option_parser/option_parser_helpers.c
${SRC_DIR}/reprompi_bench/misc.c
${SYNC_SRC_FILES}
)

if(GSL_INCLUDE_DIR)
    add_executable(measure_clock_drift_HCA  
    ${MEASURE_CLOCK_DRIFT}
    )
endif(GSL_INCLUDE_DIR)

add_executable(measure_clock_drift_JK
${MEASURE_CLOCK_DRIFT}
)
add_executable(measure_clock_drift_SK
${MEASURE_CLOCK_DRIFT}
)



set(SYNC_DURATION
sync_duration.c
parse_test_options.c
${SRC_DIR}/reprompi_bench/option_parser/option_parser_constants.c
${SRC_DIR}/reprompi_bench/option_parser/option_parser_helpers.c
${SRC_DIR}/reprompi_bench/misc.c
${SYNC_SRC_FILES}
)
if(GSL_INCLUDE_DIR)
    add_executable(sync_duration_HCA
    ${SYNC_DURATION}
    )
endif(GSL_INCLUDE_DIR)
add_executable(sync_duration_JK
${SYNC_DURATION}
)
add_executable(sync_duration_SK
${SYNC_DURATION}
)

if(GSL_INCLUDE_DIR)
    TARGET_LINK_LIBRARIES(measure_clock_drift_HCA ${COMMON_LIBRARIES} )
    TARGET_LINK_LIBRARIES(sync_duration_HCA ${COMMON_LIBRARIES} )
endif(GSL_INCLUDE_DIR)

TARGET_LINK_LIBRARIES(measure_clock_drift_JK ${COMMON_LIBRARIES} )
TARGET_LINK_LIBRARIES(measure_clock_drift_SK ${COMMON_LIBRARIES} )

TARGET_LINK_LIBRARIES(sync_duration_JK ${COMMON_LIBRARIES} )
TARGET_LINK_LIBRARIES(sync_duration_SK ${COMMON_LIBRARIES} )

TARGET_LINK_LIBRARIES(compare_clocks ${COMMON_LIBRARIES} )
TARGET_LINK_LIBRARIES(measure_rtt ${COMMON_LIBRARIES} )


SET(MY_COMPILE_FLAGS_HCA "${MY_COMPILE_FLAGS} -DENABLE_WINDOWSYNC_HCA")
SET(MY_COMPILE_FLAGS_JK "${MY_COMPILE_FLAGS} -DENABLE_WINDOWSYNC_JK")
SET(MY_COMPILE_FLAGS_SK "${MY_COMPILE_FLAGS} -DENABLE_WINDOWSYNC_SK")

if(GSL_INCLUDE_DIR)
    SET_TARGET_PROPERTIES(measure_clock_drift_HCA PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS_HCA}")
    SET_TARGET_PROPERTIES(sync_duration_HCA PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS_HCA}")
endif(GSL_INCLUDE_DIR)

SET_TARGET_PROPERTIES(measure_clock_drift_JK PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS_JK}")
SET_TARGET_PROPERTIES(sync_duration_JK PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS_JK}")

SET_TARGET_PROPERTIES(measure_clock_drift_SK PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS_SK}")
SET_TARGET_PROPERTIES(sync_duration_SK PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS_SK}")

SET_TARGET_PROPERTIES(compare_clocks PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")
SET_TARGET_PROPERTIES(measure_rtt PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")



if(EXISTS ${SRC_DIR}/reprompi_bench/sync/netgauge_sync)

    add_executable(measure_clock_drift_NG
        ${MEASURE_CLOCK_DRIFT}
    )

    add_executable(sync_duration_NG
        ${SYNC_DURATION}
    )

    TARGET_LINK_LIBRARIES(measure_clock_drift_NG ${COMMON_LIBRARIES} )

    TARGET_LINK_LIBRARIES(sync_duration_NG ${COMMON_LIBRARIES} )

    SET(MY_COMPILE_FLAGS_NG "${MY_COMPILE_FLAGS} -DENABLE_WINDOWSYNC_NG")

    SET_TARGET_PROPERTIES(measure_clock_drift_NG PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS_NG}")
    SET_TARGET_PROPERTIES(sync_duration_NG PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS_NG}")
endif(EXISTS ${SRC_DIR}/reprompi_bench/sync/netgauge_sync)

