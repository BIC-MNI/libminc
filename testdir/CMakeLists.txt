link_libraries(${LIBMINC_LIBRARIES})


macro(minc_test cmd)
  # minc 1 version
  if(HAVE_MINC1)
    add_test(NAME ${cmd}-1
        COMMAND
        ${CMAKE_COMMAND}
          -DCMD=$<TARGET_FILE:${cmd}>
          -DREF=${CMAKE_CURRENT_SOURCE_DIR}/${cmd}.out
          -DOUT=${CMAKE_CURRENT_BINARY_DIR}/${cmd}.1.out
          -P ${CMAKE_CURRENT_SOURCE_DIR}/run_test_minc.cmake
        )
  endif(HAVE_MINC1)
  
  add_test(NAME ${cmd}-2
  COMMAND
  ${CMAKE_COMMAND}
    -DCMD=$<TARGET_FILE:${cmd}>
    -DMINC=-2
    -DREF=${CMAKE_CURRENT_SOURCE_DIR}/${cmd}.out
    -DOUT=${CMAKE_CURRENT_BINARY_DIR}/${cmd}.2.out
    -P ${CMAKE_CURRENT_SOURCE_DIR}/run_test_minc.cmake
  )

  if(MINC_TEST_ENVIRONMENT)
   set_tests_properties( ${cmd}-1 PROPERTIES ENVIRONMENT "${MINC_TEST_ENVIRONMENT}")
  endif(MINC_TEST_ENVIRONMENT)
  
  if(MINC_TEST_ENVIRONMENT)
   set_tests_properties( ${cmd}-2 PROPERTIES ENVIRONMENT "${MINC_TEST_ENVIRONMENT}")
  endif(MINC_TEST_ENVIRONMENT)
endmacro(minc_test)


macro(add_minc_test name cmd)

  add_test( ${name}
        ${cmd} ${ARGV2} ${ARGV3} ${ARGV4} ${ARGV5} ${ARGV6} ${ARGV7} ${ARGV8} ${ARGV9} ${ARGV10} ${ARGV11}
        ${ARGV12} ${ARGV13} ${ARGV14} ${ARGV15} ${ARGV16} ${ARGV17} ${ARGV18} ${ARGV19} ${ARGV20} ${ARGV21} ${ARGV22}
        ${ARGV23} ${ARGV24} ${ARGV25} ${ARGV26}
   )
   
   if(MINC_TEST_ENVIRONMENT)
    set_tests_properties( ${name} PROPERTIES ENVIRONMENT "${MINC_TEST_ENVIRONMENT}")
   endif(MINC_TEST_ENVIRONMENT)
   
endmacro(add_minc_test)


if(LIBMINC_MINC1_SUPPORT)

  add_executable(minc_tst minc.c)
  add_executable(icv icv.c)
  add_executable(icv_vec icv_vec.c)
  add_executable(icv_dim1 icv_dim1.c)
  add_executable(icv_dim icv_dim.c)
  add_executable(icv_fillvalue icv_fillvalue.c)
  add_executable(icv_range icv_range.c)
  add_executable(mincapi mincapi.c)
  add_executable(minc_types minc_types.c)
  add_executable(test_mconv test_mconv.c)
  add_executable(minc_long_attr minc_long_attr.c)
  add_executable(minc_conversion minc_conversion.c)

  # running tests
  minc_test(minc_types)
  minc_test(icv_dim1)
  minc_test(icv_dim)
  minc_test(icv_fillvalue)
  minc_test(icv_range)

  #add_minc_test(arg_parse ${CMAKE_CURRENT_SOURCE_DIR}/run_test_arg_parse_cmake.sh ${CMAKE_CURRENT_BINARY_DIR}/test_arg_parse)
  add_test(NAME arg_parse
    COMMAND
    ${CMAKE_COMMAND}
      -DCMD=$<TARGET_FILE:test_arg_parse>
      -P ${CMAKE_CURRENT_SOURCE_DIR}/run_test_arg_parse.cmake
  )
  
  set_tests_properties( arg_parse PROPERTIES ENVIRONMENT "${MINC_TEST_ENVIRONMENT}")

  add_minc_test(icv icv)
  add_minc_test(icv_vec icv_vec)
  add_minc_test(minc minc_tst)
  add_minc_test(mincapi mincapi)
  add_minc_test(test_mconv test_mconv)
  add_minc_test(minc_long_attr_10k minc_long_attr 10000)
  add_minc_test(minc_long_attr_100k minc_long_attr 100000)
  add_minc_test(minc_long_attr_1m minc_long_attr 1000000)
  add_minc_test(minc_conversion minc_conversion)
endif(LIBMINC_MINC1_SUPPORT)

# Volume IO tests
add_executable(volume_test volume_test.c)
add_test(volume_test-1 volume_test)

add_executable(multidim_test multidim_test.c)
add_test(volume_multidim_test multidim_test)

add_executable(test_xfm   vio_xfm_test/test-xfm.c)
target_link_libraries(test_xfm ${VOLUME_IO_LIBRARY} ${LIBMINC_LIBRARIES})

add_executable(copy_xfm   vio_xfm_test/copy-xfm.c)
target_link_libraries(copy_xfm ${VOLUME_IO_LIBRARY} ${LIBMINC_LIBRARIES})

add_executable(create_grid_xfm create_grid_xfm.c)
target_link_libraries(create_grid_xfm ${VOLUME_IO_LIBRARY} ${LIBMINC_LIBRARIES})

add_executable(verify_xfm   vio_xfm_test/verify_xfm.c)
target_link_libraries(verify_xfm ${VOLUME_IO_LIBRARY} ${LIBMINC_LIBRARIES})

#add_test(create_grid_xfm create_grid_xfm)
#add_test(test_speed test_speed)


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
add_executable(test_arg_parse test_arg_parse.c)
add_minc_test(test_arg_parse test_arg_parse)


#MINC2 tests
add_executable(minc2-convert-test minc2-convert-test.c)
add_executable(minc2-create-test-images-2 minc2-create-test-images-2.c)
add_executable(minc2-create-test-images minc2-create-test-images.c)
add_executable(minc2-datatype-test minc2-datatype-test.c)
add_executable(minc2-large-attribute minc2-large-attribute.c)
add_executable(minc2-dimension-test minc2-dimension-test.c)
add_executable(minc2-full-test minc2-full-test.c)
add_executable(minc2-grpattr-test minc2-grpattr-test.c)
add_executable(minc2-hyper-test-2 minc2-hyper-test-2.c)
add_executable(minc2-hyper-test minc2-hyper-test.c)
add_executable(minc2-label-test minc2-label-test.c)
#add_executable(minc2-m2stats minc2-m2stats.c)
add_executable(minc2-multires-test minc2-multires-test.c)
add_executable(minc2-record-test minc2-record-test.c)
add_executable(minc2-slice-test minc2-slice-test.c)
add_executable(minc2-valid-test minc2-valid-test.c)
add_executable(minc2-vector_dimension-test minc2-vector_dimension-test.c)
add_executable(minc2-volprops-test minc2-volprops-test.c)
add_executable(minc2-read-rgb minc2-read-rgb.c)
add_executable(minc2-read-metadata minc2-read-metadata.c)

add_executable(minc2-leak-test minc2-leak-test.c)
add_executable(minc2-float-voxel-test minc2-float-voxel-test.c)

add_minc_test(minc2-convert-test          minc2-convert-test)
add_minc_test(minc2-create-test-images    minc2-create-test-images 
                                          ${CMAKE_CURRENT_BINARY_DIR}/2D_minc2.mnc 
                                          ${CMAKE_CURRENT_BINARY_DIR}/3D_minc2.mnc 
                                          ${CMAKE_CURRENT_BINARY_DIR}/4D_minc2.mnc)

add_minc_test(minc2-create-test-images-2  minc2-create-test-images-2
                                          ${CMAKE_CURRENT_BINARY_DIR}/3D_minc2_int.mnc 
                                          ${CMAKE_CURRENT_BINARY_DIR}/3D_minc2_float.mnc 
                                          )

add_minc_test(minc2-large-attribute-10k   minc2-large-attribute 10000   ${CMAKE_CURRENT_BINARY_DIR}/3D_attr_10000.mnc)
add_minc_test(minc2-large-attribute-100k  minc2-large-attribute 100000  ${CMAKE_CURRENT_BINARY_DIR}/3D_attr_100000.mnc)
add_minc_test(minc2-large-attribute-1m    minc2-large-attribute 1000000 ${CMAKE_CURRENT_BINARY_DIR}/3D_attr_1000000.mnc)

add_minc_test(minc2-datatype-test         minc2-datatype-test  
                                          ${CMAKE_CURRENT_BINARY_DIR}/datatype_minc2.mnc)
                                          
add_minc_test(minc2-dimension-test        minc2-dimension-test)
add_minc_test(minc2-full-test             minc2-full-test)
add_minc_test(minc2-grpattr-test          minc2-grpattr-test)
add_minc_test(minc2-hyper-test-2          minc2-hyper-test-2)
add_minc_test(minc2-hyper-test            minc2-hyper-test)
add_minc_test(minc2-label-test            minc2-label-test)
#add_minc_test(minc2-m2stats minc2-m2stats)
add_minc_test(minc2-multires-test         minc2-multires-test)
add_minc_test(minc2-record-test           minc2-record-test)


add_minc_test(minc2-slice-test            minc2-slice-test 
                                          ${CMAKE_CURRENT_BINARY_DIR}/3D_minc2.mnc 
                                          ${CMAKE_CURRENT_BINARY_DIR}/3D_minc2_int.mnc 
                                          ${CMAKE_CURRENT_BINARY_DIR}/3D_minc2_float.mnc
                                          )
                                          

add_minc_test(minc2-valid-test            minc2-valid-test
                                          ${CMAKE_CURRENT_BINARY_DIR}/2D_minc2.mnc 
                                          ${CMAKE_CURRENT_BINARY_DIR}/3D_minc2.mnc 
                                          ${CMAKE_CURRENT_BINARY_DIR}/3D_minc2_int.mnc 
                                          ${CMAKE_CURRENT_BINARY_DIR}/3D_minc2_float.mnc
                                          ${CMAKE_CURRENT_BINARY_DIR}/4D_minc2.mnc
)
add_minc_test(minc2-vector_dimension-test minc2-vector_dimension-test)
add_minc_test(minc2-volprops-test         minc2-volprops-test)

add_minc_test(minc2-leak-test             minc2-leak-test)

add_minc_test(minc2-float-voxel-test      minc2-float-voxel-test
                                          ${CMAKE_CURRENT_BINARY_DIR}/test-dbl.mnc
                                          )

set_property(TEST minc2-slice-test APPEND PROPERTY DEPENDS minc2-create-test-images) 
set_property(TEST minc2-slice-test APPEND PROPERTY DEPENDS minc2-create-test-images-2) 
set_property(TEST minc2-valid-test APPEND PROPERTY DEPENDS minc2-create-test-images) 
set_property(TEST minc2-valid-test APPEND PROPERTY DEPENDS minc2-create-test-images-2) 

