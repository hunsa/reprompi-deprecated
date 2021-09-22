set(PROCESS_SKEW_SRC_FILES
${SRC_DIR}/contrib/process_skew/process_skew.c
${SRC_DIR}/contrib/process_skew/options_parser.c
${SRC_DIR}/contrib/process_skew/output.c
# reprompi benchmark
${SRC_DIR}/reprompi_bench/misc.c
${SRC_DIR}/reprompi_bench/utils/keyvalue_store.c
# options parsing
${COMMON_OPTION_PARSER_SRC_FILES}
${SRC_DIR}/reprompi_bench/option_parser/parse_extra_key_value_options.c
${SRC_DIR}/reprompi_bench/option_parser/parse_options.c
# collective operations
${COLL_OPS_SRC_FILES}
# synchronization
${SYNC_SRC_FILES}
# intercommunication
${INTERCOMM_SOURCE_FILES}
)

add_executable(process_skew
${PROCESS_SKEW_SRC_FILES}
)
TARGET_LINK_LIBRARIES(process_skew ${COMMON_LIBRARIES} )

if(COMPILE_PROCESS_SKEW)
    SET(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS} -DCOMPILE_PROCESS_SKEW")
endif()

SET_TARGET_PROPERTIES(process_skew PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")
