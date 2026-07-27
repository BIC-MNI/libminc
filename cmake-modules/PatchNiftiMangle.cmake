# PatchNiftiMangle.cmake
#
# Prepends the libminc NIfTI symbol-mangling #defines (nifti_mangle.h) to the
# nifti_clib public headers so that:
#   - nifti1_io.c / znzlib.c compile with every definition renamed to minc_*,
#   - every consumer that includes <nifti1_io.h>/<znzlib.h> (libminc, nii2mnc,
#     mnc2nii, ...) transparently uses the same minc_* names,
# while ITK's own bundled nifti (built from unpatched headers) stays unprefixed.
#
# The mangle block is inlined into the headers (rather than shipped as a
# separate include) because nifti_clib v3.0.0 installs only an explicit header
# list, so a standalone nifti_mangle.h would not reach installed-header
# consumers. Idempotent via the MINC_NIFTI_MANGLE_H guard.
#
# Expects: -DSRC=<nifti source dir>  -DMANGLE=<path to nifti_mangle.h>

if(NOT EXISTS "${MANGLE}")
  message(FATAL_ERROR "PatchNiftiMangle: mangle header not found: ${MANGLE}")
endif()
file(READ "${MANGLE}" _mangle)

set(_headers
  "${SRC}/niftilib/nifti1_io.h"
  "${SRC}/znzlib/znzlib.h")

foreach(_f ${_headers})
  if(NOT EXISTS "${_f}")
    message(FATAL_ERROR "PatchNiftiMangle: header not found: ${_f}")
  endif()
  file(READ "${_f}" _orig)
  if(_orig MATCHES "MINC_NIFTI_MANGLE_H")
    message(STATUS "PatchNiftiMangle: ${_f} already patched")
  else()
    file(WRITE "${_f}" "${_mangle}\n${_orig}")
    message(STATUS "PatchNiftiMangle: prefixed nifti symbols in ${_f}")
  endif()
endforeach()
