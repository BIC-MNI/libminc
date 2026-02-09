# FindNIFTI.cmake module


find_path(NIFTI_INCLUDE_DIR nifti1_io.h /usr/include /usr/local/include /usr/local/bic/include)

find_library(NIFTI_LIBRARY NAMES niftiio PATHS /usr/lib /usr/local/lib /usr/local/bic/lib)

find_path(ZNZ_INCLUDE_DIR znzlib.h /usr/include /usr/local/include /usr/local/bic/include)

find_library(ZNZ_LIBRARY NAMES znz PATHS /usr/lib /usr/local/lib /usr/local/bic/lib)


if (NIFTI_INCLUDE_DIR AND NIFTI_LIBRARY AND ZNZ_INCLUDE_DIR AND ZNZ_LIBRARY)
   set(NIFTI_FOUND TRUE)
endif (NIFTI_INCLUDE_DIR AND NIFTI_LIBRARY AND ZNZ_INCLUDE_DIR AND ZNZ_LIBRARY)


if (NIFTI_FOUND)
   if (NOT NIFTI_FIND_QUIETLY)
      message(STATUS "Found NetCDF headers: ${NIFTI_INCLUDE_DIR}")
      message(STATUS "Found NetCDF library: ${NIFTI_LIBRARY}")
      message(STATUS "Found znzlib headers: ${ZNZ_INCLUDE_DIR}")
      message(STATUS "Found znzlib library: ${ZNZ_LIBRARY}")
   endif (NOT NIFTI_FIND_QUIETLY)
else (NIFTI_FOUND)
   if (NIFTI_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find NIfTI-1 I/O library")
   endif (NIFTI_FIND_REQUIRED)
endif (NIFTI_FOUND)
