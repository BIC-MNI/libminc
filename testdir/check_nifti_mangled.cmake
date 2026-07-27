# check_nifti_mangled.cmake
#
# Asserts that libminc's built-in NIfTI implementation is exported under the
# mangled `minc_` prefix and NOT under the bare `nifti_*` names that collide
# with ITK's own bundled niftiio.  Run against a binary that pulls the libminc
# NIfTI reader chain (e.g. the nifti_readback test executable).
#
# Expects: -DNM=<nm tool>  -DTARGET=<path to binary/archive to inspect>

if(NOT EXISTS "${TARGET}")
  message(FATAL_ERROR "check_nifti_mangled: target not found: ${TARGET}")
endif()

execute_process(
  COMMAND "${NM}" "${TARGET}"
  OUTPUT_VARIABLE _syms
  RESULT_VARIABLE _rc
  ERROR_VARIABLE _err)
if(NOT _rc EQUAL 0)
  message(FATAL_ERROR "check_nifti_mangled: nm failed: ${_err}")
endif()

# A defined-text symbol named exactly `nifti_image_read` (no minc_ prefix) means
# the collision-prone symbol is still exported -> fail (this is the RED state).
if(_syms MATCHES "[ \t]T[ \t]+nifti_image_read\n" OR _syms MATCHES "[ \t]T[ \t]+nifti_image_read$")
  message(FATAL_ERROR
    "check_nifti_mangled: FOUND unmangled 'T nifti_image_read' -- libminc still "
    "exports the ITK-colliding NIfTI symbols. Expected the minc_-prefixed form.")
endif()

# The mangled symbol must be present, proving nifti was actually linked in
# under the prefix (guards against a false pass where nifti wasn't pulled).
if(NOT _syms MATCHES "minc_nifti_image_read")
  message(FATAL_ERROR
    "check_nifti_mangled: did NOT find 'minc_nifti_image_read' -- the mangled "
    "NIfTI implementation is not present in ${TARGET}.")
endif()

message(STATUS "check_nifti_mangled: OK -- nifti exported as minc_nifti_*, no bare nifti_image_read")
