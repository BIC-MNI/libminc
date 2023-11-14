execute_process(COMMAND ${CMD} ${MINC} RESULT_VARIABLE CMD_RESULT OUTPUT_FILE ${OUT})

if(CMD_RESULT)
    message(FATAL_ERROR "Error running ${CMD}")
endif()

#file(READ ${REF} CMD_REF)

#string(COMPARE EQUAL CMD_OUTPUT CMD_REF REF_MATCHES)
execute_process(
    COMMAND ${CMAKE_COMMAND} -E compare_files ${REF} ${OUT}
    RESULT_VARIABLE REF_NOT_MATCHES
    OUTPUT_QUIET
    ERROR_QUIET
    )

if(REF_NOT_MATCHES)
    message(FATAL_ERROR "Output of ${CMD} ${MINC} does not match reference: ${OUT} ${REF}")
endif()
