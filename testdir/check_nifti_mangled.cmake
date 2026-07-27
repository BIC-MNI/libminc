# check_nifti_mangled.cmake
#
# Asserts that libminc's built-in NIfTI implementation is exported under the
# mangled `minc_` prefix and NOT under the bare `nifti_*` names that collide
# with ITK's own bundled niftiio.  Run against the built libniftiio.a archive
# (NIFTI_LIBRARY): an archive always retains its defined symbols unstripped on
# every platform, unlike a linked/possibly-stripped executable.
#
# Portable across GNU/ELF and Apple/Mach-O `nm`:
#   - Mach-O prefixes C symbols with a leading '_' (so `_minc_nifti_image_read`
#     vs GNU's `minc_nifti_image_read`); the patterns allow an optional `_`.
#   - A trailing boundary keeps `nifti_image_read` from matching inside
#     `nifti_image_read_bricks`.
#
# Expects: -DNM=<nm tool>  -DTARGET=<path to libniftiio.a>

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

# A defined text symbol named exactly `nifti_image_read` (bare, i.e. not the
# minc_-mangled form) means the collision-prone symbol is still exported -> the
# RED state.  `_?` tolerates Mach-O's leading underscore; the trailing class
# excludes `nifti_image_read_bricks` etc.  The `_minc_` form is not matched
# because `[ \t]+_?nifti_image_read` requires whitespace right before the name.
if(_syms MATCHES "[ \t][TtDd][ \t]+_?nifti_image_read[ \t\r\n]" OR
   _syms MATCHES "[ \t][TtDd][ \t]+_?nifti_image_read$")
  message(FATAL_ERROR
    "check_nifti_mangled: FOUND unmangled 'nifti_image_read' -- libminc still "
    "exports the ITK-colliding NIfTI symbols. Expected the minc_-prefixed form.")
endif()

# The mangled symbol must be present as a defined text symbol, proving the
# prefix was applied (guards against a false pass where nifti wasn't built).
if(NOT (_syms MATCHES "[ \t][TtDd][ \t]+_?minc_nifti_image_read[ \t\r\n]" OR
        _syms MATCHES "[ \t][TtDd][ \t]+_?minc_nifti_image_read$"))
  message(FATAL_ERROR
    "check_nifti_mangled: did NOT find defined 'minc_nifti_image_read' -- the "
    "mangled NIfTI implementation is not present in ${TARGET}.")
endif()

message(STATUS "check_nifti_mangled: OK -- nifti exported as minc_nifti_*, no bare nifti_image_read")
