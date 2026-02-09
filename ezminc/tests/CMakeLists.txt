link_libraries(minc_io ${LIBMINC_LIBRARIES})

add_executable(ezminc_rw_test ezminc_rw_test.cpp)
#add_executable(ezminc_stats   ezminc_stats.cpp)

add_test(ezminc_rw_test ezminc_rw_test ${CMAKE_CURRENT_BINARY_DIR})

add_executable(ezminc_rw_test2 minc_rw_test2.cpp)


if(MINC_TEST_ENVIRONMENT)
 set_tests_properties( ezminc_rw_test PROPERTIES ENVIRONMENT "${MINC_TEST_ENVIRONMENT}")
endif()
