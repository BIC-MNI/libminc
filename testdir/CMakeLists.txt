LINK_LIBRARIES(${LIBMINC_LIBRARIES})

MACRO(minc_test cmd)
  # minc 1 version
  IF(HAVE_MINC1)
    ADD_TEST(${cmd}-1 ${CMAKE_CURRENT_SOURCE_DIR}/run_test_cmake.sh ${CMAKE_CURRENT_BINARY_DIR}/${cmd} ${CMAKE_CURRENT_SOURCE_DIR}/${cmd}.out)
  ENDIF(HAVE_MINC1)
  
  ADD_TEST(${cmd}-2 ${CMAKE_CURRENT_SOURCE_DIR}/run_test2_cmake.sh ${CMAKE_CURRENT_BINARY_DIR}/${cmd} ${CMAKE_CURRENT_SOURCE_DIR}/${cmd}.out)
  
  IF(MINC_TEST_ENVIRONMENT)
   set_tests_properties( ${cmd}-1 PROPERTIES ENVIRONMENT "${MINC_TEST_ENVIRONMENT}")
  ENDIF(MINC_TEST_ENVIRONMENT)
  
  IF(MINC_TEST_ENVIRONMENT)
   set_tests_properties( ${cmd}-2 PROPERTIES ENVIRONMENT "${MINC_TEST_ENVIRONMENT}")
  ENDIF(MINC_TEST_ENVIRONMENT)
ENDMACRO(minc_test)

MACRO(add_minc_test name cmd)

  ADD_TEST( ${name}
      ${cmd} ${ARGV2} ${ARGV3} ${ARGV4} ${ARGV5} ${ARGV6} ${ARGV7} ${ARGV8} ${ARGV9} ${ARGV10} ${ARGV11}
        ${ARGV12} ${ARGV13} ${ARGV14} ${ARGV15} ${ARGV16} ${ARGV17} ${ARGV18} ${ARGV19} ${ARGV20} ${ARGV21} ${ARGV22}
        ${ARGV23} ${ARGV24} ${ARGV25} ${ARGV26}
   )
   
   IF(MINC_TEST_ENVIRONMENT)
    set_tests_properties( ${name} PROPERTIES ENVIRONMENT "${MINC_TEST_ENVIRONMENT}")
   ENDIF(MINC_TEST_ENVIRONMENT)
   
ENDMACRO(add_minc_test)


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
  ADD_EXECUTABLE(minc_long_attr minc_long_attr.c)
  ADD_EXECUTABLE(minc_conversion minc_conversion.c)

  #ADD_EXECUTABLE(test_speed test_speed.c)

  # running tests
  minc_test(minc_types)
  minc_test(icv_dim1)
  minc_test(icv_dim)
  minc_test(icv_fillvalue)
  minc_test(icv_range)

  add_minc_test(arg_parse ${CMAKE_CURRENT_SOURCE_DIR}/run_test_arg_parse_cmake.sh ${CMAKE_CURRENT_BINARY_DIR}/test_arg_parse)
  add_minc_test(icv icv)
  add_minc_test(minc minc_tst)
  add_minc_test(mincapi mincapi)
  add_minc_test(test_mconv test_mconv)
  add_minc_test(minc_long_attr_10k minc_long_attr 10000)
  add_minc_test(minc_long_attr_100k minc_long_attr 100000)
  add_minc_test(minc_long_attr_1m minc_long_attr 1000000)
  add_minc_test(minc_conversion minc_conversion)
ENDIF(LIBMINC_MINC1_SUPPORT)

ADD_EXECUTABLE(nifti_test nifti_test.c)
ADD_TEST(nifti_test-1 nifti_test)

# Volume IO tests
ADD_EXECUTABLE(test_xfm   vio_xfm_test/test-xfm.c)
TARGET_LINK_LIBRARIES(test_xfm ${VOLUME_IO_LIBRARY} ${LIBMINC_LIBRARIES})

ADD_EXECUTABLE(copy_xfm   vio_xfm_test/copy-xfm.c)
TARGET_LINK_LIBRARIES(copy_xfm ${VOLUME_IO_LIBRARY} ${LIBMINC_LIBRARIES})

ADD_EXECUTABLE(create_grid_xfm create_grid_xfm.c)
TARGET_LINK_LIBRARIES(create_grid_xfm ${VOLUME_IO_LIBRARY} ${LIBMINC_LIBRARIES})

ADD_EXECUTABLE(verify_xfm   vio_xfm_test/verify_xfm.c)
TARGET_LINK_LIBRARIES(verify_xfm ${VOLUME_IO_LIBRARY} ${LIBMINC_LIBRARIES})

#ADD_TEST(create_grid_xfm create_grid_xfm)
#ADD_TEST(test_speed test_speed)

add_minc_test(test_xfm_1 test_xfm 10000 ${CMAKE_CURRENT_SOURCE_DIR}/vio_xfm_test/t1.xfm)
add_minc_test(test_xfm_2 test_xfm 10000 ${CMAKE_CURRENT_SOURCE_DIR}/vio_xfm_test/t2.xfm)
add_minc_test(test_xfm_3 test_xfm 10000 ${CMAKE_CURRENT_SOURCE_DIR}/vio_xfm_test/t3.xfm 0.9)

add_minc_test(copy_xfm copy_xfm ${CMAKE_CURRENT_SOURCE_DIR}/vio_xfm_test/t3.xfm ${CMAKE_CURRENT_BINARY_DIR}/t3_copy.xfm)

add_minc_test(verify_xfm_1 verify_xfm
 ${CMAKE_CURRENT_SOURCE_DIR}/vio_xfm_test/t3.xfm
 ${CMAKE_CURRENT_SOURCE_DIR}/vio_xfm_test/random2 
 1e-6)

add_minc_test(verify_xfm_float verify_xfm
 ${CMAKE_CURRENT_SOURCE_DIR}/vio_xfm_test/test_float.xfm
 ${CMAKE_CURRENT_SOURCE_DIR}/vio_xfm_test/verify_xfm_table.txt
 1e-9)

add_minc_test(verify_xfm_short verify_xfm
  ${CMAKE_CURRENT_SOURCE_DIR}/vio_xfm_test/test_short.xfm
  ${CMAKE_CURRENT_SOURCE_DIR}/vio_xfm_test/verify_xfm_table.txt
  1e-4)
  
add_minc_test(verify_xfm_byte verify_xfm
  ${CMAKE_CURRENT_SOURCE_DIR}/vio_xfm_test/test_byte.xfm
  ${CMAKE_CURRENT_SOURCE_DIR}/vio_xfm_test/verify_xfm_table.txt
  1e-1) 

add_minc_test(verify_xfm_2 verify_xfm 
 ${CMAKE_CURRENT_BINARY_DIR}/t3_copy.xfm
 ${CMAKE_CURRENT_SOURCE_DIR}/vio_xfm_test/random2
 1e-3)

set_property(TEST verify_xfm_2 APPEND PROPERTY DEPENDS copy_xfm)


#common tests
ADD_EXECUTABLE(test_arg_parse test_arg_parse.c)
add_minc_test(test_arg_parse test_arg_parse)


#MINC2 tests
ADD_EXECUTABLE(minc2-convert-test minc2-convert-test.c)
ADD_EXECUTABLE(minc2-create-test-images-2 minc2-create-test-images-2.c)
ADD_EXECUTABLE(minc2-create-test-images minc2-create-test-images.c)
ADD_EXECUTABLE(minc2-datatype-test minc2-datatype-test.c)
ADD_EXECUTABLE(minc2-large-attribute minc2-large-attribute.c)
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

add_minc_test(minc2-convert-test minc2-convert-test)
add_minc_test(minc2-create-test-images-2 minc2-create-test-images-2)
add_minc_test(minc2-create-test-images minc2-create-test-images)
add_minc_test(minc2-large-attribute-10k minc2-large-attribute 10000)
add_minc_test(minc2-large-attribute-100k minc2-large-attribute 100000)
add_minc_test(minc2-large-attribute-1m minc2-large-attribute 1000000)
add_minc_test(minc2-datatype-test minc2-datatype-test)
add_minc_test(minc2-dimension-test minc2-dimension-test)
add_minc_test(minc2-full-test minc2-full-test)
add_minc_test(minc2-grpattr-test minc2-grpattr-test)
add_minc_test(minc2-hyper-test-2 minc2-hyper-test-2)
add_minc_test(minc2-hyper-test minc2-hyper-test)
add_minc_test(minc2-label-test minc2-label-test)
#add_minc_test(minc2-m2stats minc2-m2stats)
add_minc_test(minc2-multires-test minc2-multires-test)
add_minc_test(minc2-record-test minc2-record-test)
add_minc_test(minc2-slice-test minc2-slice-test)
add_minc_test(minc2-valid-test minc2-valid-test)
add_minc_test(minc2-vector_dimension-test minc2-vector_dimension-test)
add_minc_test(minc2-volprops-test minc2-volprops-test)

