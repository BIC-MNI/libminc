# FindNetCDF.cmake module

include(FindPackageHandleStandardArgs)

find_path(NETCDF_INCLUDE_DIR netcdf.h)

find_library(NETCDF_LIBRARY NAMES netcdf)

find_package_handle_standard_args(NETCDF
  REQUIRED_VARS NETCDF_LIBRARY NETCDF_INCLUDE_DIR
)

if(NETCDF_FOUND)
  mark_as_advanced(NETCDF_INCLUDE_DIR NETCDF_LIBRARY)
endif()
