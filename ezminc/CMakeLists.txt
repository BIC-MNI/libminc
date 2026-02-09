if(LIBMINC_BUILD_V2)
  add_definitions( -DMINC2 )
endif(LIBMINC_BUILD_V2)


set( MINC_IO_HEADERS 
    minc_io_exceptions.h 
    minc_io_fixed_vector.h  
    minc_io_simple_volume.h
    minc_1_rw.h
    minc_1_simple.h
    minc_1_simple_rw.h
    minc_io_4d_volume.h
   )

set( MINC_IO_SRC 
    minc_1_rw.cpp
    minc_1_simple_rw.cpp
  )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR})
add_library( minc_io ${LIBRARY_TYPE} ${MINC_IO_HEADERS} ${MINC_IO_SRC})
target_link_libraries(minc_io ${LIBMINC_LIBRARIES})

set_target_properties(minc_io
 PROPERTIES 
  SOVERSION ${LIBMINC_SOVERSION})

if(LIBMINC_INSTALL_LIB_DIR)
  install(TARGETS 
           minc_io 
          EXPORT
            ${LIBMINC_EXPORTED_TARGETS}  
    LIBRARY DESTINATION ${LIBMINC_INSTALL_LIB_DIR} COMPONENT libraries
    ARCHIVE DESTINATION ${LIBMINC_INSTALL_LIB_DIR} COMPONENT libraries
    RUNTIME DESTINATION ${LIBMINC_INSTALL_LIB_DIR} COMPONENT libraries
            )
endif(LIBMINC_INSTALL_LIB_DIR)

if(LIBMINC_INSTALL_INCLUDE_DIR)
  install(FILES  ${MINC_IO_HEADERS} DESTINATION ${LIBMINC_INSTALL_INCLUDE_DIR})
endif(LIBMINC_INSTALL_INCLUDE_DIR)
  
if(LIBMINC_BUILD_EZMINC_EXAMPLES)
  add_subdirectory(examples)  
endif(LIBMINC_BUILD_EZMINC_EXAMPLES)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif(BUILD_TESTING)
