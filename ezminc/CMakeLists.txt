if(LIBMINC_BUILD_V2)
  # Definition applied to target below, not globally
endif()


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

add_library( minc_io ${LIBRARY_TYPE} ${MINC_IO_HEADERS} ${MINC_IO_SRC})

# Alias target for FetchContent / add_subdirectory consumers
add_library(LIBMINC::minc_io ALIAS minc_io)

# Link against the minc2 target (not the variable) -- transitive deps flow automatically
target_link_libraries(minc_io PUBLIC ${LIBMINC_LIBRARY})

# Target-scoped compile definitions (replaces global add_definitions(-DMINC2))
if(LIBMINC_BUILD_V2)
  target_compile_definitions(minc_io PRIVATE MINC2)
endif()

# Target-scoped include directories (replaces global include_directories)
target_include_directories(minc_io
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set_target_properties(minc_io
 PROPERTIES 
  SOVERSION ${LIBMINC_SOVERSION}
  POSITION_INDEPENDENT_CODE ON)

if(LIBMINC_INSTALL_LIB_DIR AND (LIBMINC_IS_TOP_LEVEL OR LIBMINC_EXTERNALLY_CONFIGURED))
  install(TARGETS 
           minc_io 
          EXPORT
            ${LIBMINC_EXPORTED_TARGETS}  
    LIBRARY DESTINATION ${LIBMINC_INSTALL_LIB_DIR} COMPONENT libraries
    ARCHIVE DESTINATION ${LIBMINC_INSTALL_LIB_DIR} COMPONENT libraries
    RUNTIME DESTINATION ${LIBMINC_INSTALL_LIB_DIR} COMPONENT libraries
            )
endif()

if(LIBMINC_INSTALL_INCLUDE_DIR AND (LIBMINC_IS_TOP_LEVEL OR LIBMINC_EXTERNALLY_CONFIGURED))
  install(FILES  ${MINC_IO_HEADERS} DESTINATION ${LIBMINC_INSTALL_INCLUDE_DIR})
endif()
  
if(LIBMINC_BUILD_EZMINC_EXAMPLES)
  add_subdirectory(examples)  
endif()

if(BUILD_TESTING AND (LIBMINC_IS_TOP_LEVEL OR LIBMINC_EXTERNALLY_CONFIGURED))
  add_subdirectory(tests)
endif()
