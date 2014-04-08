LINK_LIBRARIES(minc_io ${LIBMINC_LIBRARIES})

ADD_EXECUTABLE(ezminc_rw_test ezminc_rw_test.cpp)
#ADD_EXECUTABLE(ezminc_stats   ezminc_stats.cpp)

ADD_TEST(ezminc_rw_test ezminc_rw_test ${CMAKE_CURRENT_BINARY_DIR})

ADD_EXECUTABLE(ezminc_rw_test2 minc_rw_test2.cpp)


IF(MINC_TEST_ENVIRONMENT)
 set_tests_properties( ezminc_rw_test PROPERTIES ENVIRONMENT "${MINC_TEST_ENVIRONMENT}")
ENDIF(MINC_TEST_ENVIRONMENT)
