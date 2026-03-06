# FindNIFTI.cmake module

include(FindPackageHandleStandardArgs)

find_path(NIFTI_INCLUDE_DIR nifti1_io.h)

find_library(NIFTI_LIBRARY NAMES niftiio)

find_path(ZNZ_INCLUDE_DIR znzlib.h)

find_library(ZNZ_LIBRARY NAMES znz)

find_package_handle_standard_args(NIFTI
  REQUIRED_VARS NIFTI_LIBRARY NIFTI_INCLUDE_DIR ZNZ_LIBRARY ZNZ_INCLUDE_DIR
)

if(NIFTI_FOUND)
  mark_as_advanced(NIFTI_INCLUDE_DIR NIFTI_LIBRARY ZNZ_INCLUDE_DIR ZNZ_LIBRARY)
endif()
