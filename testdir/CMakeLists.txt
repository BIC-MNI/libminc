LINK_LIBRARIES(${LIBMINC_LIBRARIES})

IF(LIBMINC_MINC1_SUPPORT)

  ADD_EXECUTABLE(minc_tst minc.c)
  ADD_EXECUTABLE(icv icv.c)

  ADD_EXECUTABLE(icv_dim1 icv_dim1.c)
  ADD_EXECUTABLE(icv_dim icv_dim.c)
  ADD_EXECUTABLE(icv_fillvalue icv_fillvalue.c)
  ADD_EXECUTABLE(icv_range icv_range.c)
  ADD_EXECUTABLE(mincapi mincapi.c)
  ADD_EXECUTABLE(minc_types minc_types.c)
  ADD_EXECUTABLE(test_mconv test_mconv.c)

  #ADD_EXECUTABLE(test_speed test_speed.c)


  ADD_EXECUTABLE(create_grid_xfm create_grid_xfm.c)
  TARGET_LINK_LIBRARIES(create_grid_xfm ${VOLUME_IO_LIBRARY} ${LIBMINC_LIBRARIES} m)
  
MACRO(minc_test cmd)
  # minc 1 version
  IF(HAVE_MINC1)
    ADD_TEST(${cmd}-1 ${CMAKE_CURRENT_SOURCE_DIR}/run_test_cmake.sh ${CMAKE_CURRENT_BINARY_DIR}/${cmd} ${CMAKE_CURRENT_SOURCE_DIR}/${cmd}.out)
  ENDIF(HAVE_MINC1)
  
  ADD_TEST(${cmd}-2 ${CMAKE_CURRENT_SOURCE_DIR}/run_test2_cmake.sh ${CMAKE_CURRENT_BINARY_DIR}/${cmd} ${CMAKE_CURRENT_SOURCE_DIR}/${cmd}.out)
ENDMACRO(minc_test)

  # running tests
  minc_test(minc_types)
  minc_test(icv_dim1)
  minc_test(icv_dim)
  minc_test(icv_fillvalue)
  minc_test(icv_range)

  ADD_TEST(arg_parse ${CMAKE_CURRENT_SOURCE_DIR}/run_test_arg_parse_cmake.sh ${CMAKE_CURRENT_BINARY_DIR}/test_arg_parse)
  ADD_TEST(icv icv)
  ADD_TEST(minc minc_tst)

  ADD_TEST(mincapi mincapi)
  ADD_TEST(test_mconv test_mconv)


ENDIF(LIBMINC_MINC1_SUPPORT)


# Volume IO tests
ADD_EXECUTABLE(test_xfm   vio_xfm_test/test-xfm.c)
TARGET_LINK_LIBRARIES(test_xfm ${VOLUME_IO_LIBRARY} ${LIBMINC_LIBRARIES})


# TODO port these test to cmake
#ADD_TEST(create_grid_xfm create_grid_xfm)
#ADD_TEST(test_speed test_speed)
#ADD_TEST(test_xfm test_xfm)

ADD_TEST(test_xfm_1 test_xfm 10000 ${CMAKE_CURRENT_SOURCE_DIR}/vio_xfm_test/t1.xfm)
ADD_TEST(test_xfm_2 test_xfm 10000 ${CMAKE_CURRENT_SOURCE_DIR}/vio_xfm_test/t2.xfm)

IF(HAVE_MINC1)
  # nonlnear transform test will fail without MINC1 
  ADD_TEST(test_xfm_3 test_xfm 10000 ${CMAKE_CURRENT_SOURCE_DIR}/vio_xfm_test/t3.xfm 0.9)
ENDIF(HAVE_MINC1)


#IF(LIBMINC_BUILD_V2)
#common tests
ADD_EXECUTABLE(test_arg_parse test_arg_parse.c)
ADD_TEST(test_arg_parse test_arg_parse)


#MINC2 tests
ADD_EXECUTABLE(minc2-convert-test minc2-convert-test.c)
ADD_EXECUTABLE(minc2-create-test-images-2 minc2-create-test-images-2.c)
ADD_EXECUTABLE(minc2-create-test-images minc2-create-test-images.c)
ADD_EXECUTABLE(minc2-datatype-test minc2-datatype-test.c)
ADD_EXECUTABLE(minc2-dimension-test minc2-dimension-test.c)
ADD_EXECUTABLE(minc2-full-test minc2-full-test.c)
ADD_EXECUTABLE(minc2-grpattr-test minc2-grpattr-test.c)
ADD_EXECUTABLE(minc2-hyper-test-2 minc2-hyper-test-2.c)
ADD_EXECUTABLE(minc2-hyper-test minc2-hyper-test.c)
ADD_EXECUTABLE(minc2-label-test minc2-label-test.c)
#ADD_EXECUTABLE(minc2-m2stats minc2-m2stats.c)
ADD_EXECUTABLE(minc2-multires-test minc2-multires-test.c)
ADD_EXECUTABLE(minc2-record-test minc2-record-test.c)
ADD_EXECUTABLE(minc2-slice-test minc2-slice-test.c)
ADD_EXECUTABLE(minc2-valid-test minc2-valid-test.c)
ADD_EXECUTABLE(minc2-vector_dimension-test minc2-vector_dimension-test.c)
ADD_EXECUTABLE(minc2-volprops-test minc2-volprops-test.c)
ADD_EXECUTABLE(minc2-read-rgb minc2-read-rgb.c)
ADD_EXECUTABLE(minc2-read-metadata minc2-read-metadata.c)

ADD_TEST(minc2-convert-test minc2-convert-test)
ADD_TEST(minc2-create-test-images-2 minc2-create-test-images-2)
ADD_TEST(minc2-create-test-images minc2-create-test-images)
ADD_TEST(minc2-datatype-test minc2-datatype-test)
ADD_TEST(minc2-dimension-test minc2-dimension-test)
ADD_TEST(minc2-full-test minc2-full-test)
ADD_TEST(minc2-grpattr-test minc2-grpattr-test)
ADD_TEST(minc2-hyper-test-2 minc2-hyper-test-2)
ADD_TEST(minc2-hyper-test minc2-hyper-test)
ADD_TEST(minc2-label-test minc2-label-test)
#ADD_TEST(minc2-m2stats minc2-m2stats)
ADD_TEST(minc2-multires-test minc2-multires-test)
ADD_TEST(minc2-record-test minc2-record-test)
ADD_TEST(minc2-slice-test minc2-slice-test)
ADD_TEST(minc2-valid-test minc2-valid-test)
ADD_TEST(minc2-vector_dimension-test minc2-vector_dimension-test)
ADD_TEST(minc2-volprops-test minc2-volprops-test)

#ENDIF(LIBMINC_BUILD_V2)
