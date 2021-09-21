set(PROCESS_SKEW_SRC_FILES
${SRC_DIR}/contrib/process_skew/process_skew.c
${SRC_DIR}/contrib/process_skew/options_parser.c
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
