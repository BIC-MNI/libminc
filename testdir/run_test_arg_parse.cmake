macro(check IN REF)
    separate_arguments(IN_LST UNIX_COMMAND "${IN}")

    execute_process(COMMAND ${CMD} ${IN_LST} RESULT_VARIABLE CMD_RESULT OUTPUT_VARIABLE OUT OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(NOT ${CMD_RESULT} EQUAL 0)
        message(FATAL_ERROR "Command ${CMD} failed with result ${CMD_RESULT}")
    endif()
    if(NOT "${OUT}" STREQUAL "${REF}")
        message(FATAL_ERROR "Command ${CMD} \"${IN}\" returned \"${OUT}\" instead of \"${REF}\"")
    endif()
endmacro()


check(""                   "const_a:0 const_b:0 int_a:0 int_b:0 long_a:0 long_b:0")
check("-const_a"           "const_a:1 const_b:0 int_a:0 int_b:0 long_a:0 long_b:0")
check("-const_b"           "const_a:0 const_b:1 int_a:0 int_b:0 long_a:0 long_b:0")
check("-const_a -const_b"  "const_a:1 const_b:1 int_a:0 int_b:0 long_a:0 long_b:0")
check("-const_b -const_a"  "const_a:1 const_b:1 int_a:0 int_b:0 long_a:0 long_b:0")

check("-int_a 33"          "const_a:0 const_b:0 int_a:33 int_b:0 long_a:0 long_b:0")
check("-int_a -3"          "const_a:0 const_b:0 int_a:-3 int_b:0 long_a:0 long_b:0")
check("-int_b 22"          "const_a:0 const_b:0 int_a:0 int_b:22 long_a:0 long_b:0")
check("-int_b -2"          "const_a:0 const_b:0 int_a:0 int_b:-2 long_a:0 long_b:0")
check("-int_a -1 -int_b 3" "const_a:0 const_b:0 int_a:-1 int_b:3 long_a:0 long_b:0")
check("-int_b -1 -int_a 3" "const_a:0 const_b:0 int_a:3 int_b:-1 long_a:0 long_b:0")

check("-long_a 12"            "const_a:0 const_b:0 int_a:0 int_b:0 long_a:12 long_b:0")
check("-long_a -99"           "const_a:0 const_b:0 int_a:0 int_b:0 long_a:-99 long_b:0")
check("-long_b -12"           "const_a:0 const_b:0 int_a:0 int_b:0 long_a:0 long_b:-12")
check("-long_b 99"            "const_a:0 const_b:0 int_a:0 int_b:0 long_a:0 long_b:99")
check("-long_a 3 -long_b -9"  "const_a:0 const_b:0 int_a:0 int_b:0 long_a:3 long_b:-9")
check("-long_b 3 -long_a -9"  "const_a:0 const_b:0 int_a:0 int_b:0 long_a:-9 long_b:3")

check("-long_a -99 -int_b 3 -const_b"  "const_a:0 const_b:1 int_a:0 int_b:3 long_a:-99 long_b:0")

check("-nonsense"          "const_a:0 const_b:0 int_a:0 int_b:0 long_a:0 long_b:0")

