# FindNetCDF.cmake module


find_path(NETCDF_INCLUDE_DIR netcdf.h /usr/include /usr/local/include /usr/local/bic/include)

find_library(NETCDF_LIBRARY NAMES netcdf PATHS /usr/lib /usr/local/lib /usr/local/bic/lib)


if (NETCDF_INCLUDE_DIR AND NETCDF_LIBRARY)
   set(NETCDF_FOUND TRUE)
endif (NETCDF_INCLUDE_DIR AND NETCDF_LIBRARY)


if (NETCDF_FOUND)
   if (NOT NETCDF_FIND_QUIETLY)
      message(STATUS "Found NetCDF headers: ${NETCDF_INCLUDE_DIR}")
      message(STATUS "Found NetCDF library: ${NETCDF_LIBRARY}")
   endif (NOT NETCDF_FIND_QUIETLY)
else (NETCDF_FOUND)
   if (NETCDF_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find NetCDF")
   endif (NETCDF_FIND_REQUIRED)
endif (NETCDF_FOUND)
