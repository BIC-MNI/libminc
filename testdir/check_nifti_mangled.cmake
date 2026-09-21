# check_nifti_mangled.cmake
#
# Asserts that every symbol libminc's built-in NIfTI exports carries the
# `minc_` prefix, so none of them can collide with another NIfTI copy in the
# same program (ITK bundles one). Run once per archive, libniftiio.a and
# libznz.a: an archive always retains its defined symbols unstripped on every
# platform, unlike a linked/possibly-stripped executable.
#
# Checking every symbol, not a sample, is the point: when the nifti_clib pin
# moved, three new exports slipped past nifti_mangle.h while a check of
# nifti_image_read alone kept passing.
#
# Portable across GNU/ELF and Apple/Mach-O `nm`: Mach-O prefixes C symbols
# with a leading '_' (`_minc_nifti_image_read`), so the patterns allow an
# optional `_`. S is Mach-O's type for data outside __text/__data/__bss.
#
# Expects: -DNM=<nm tool>  -DTARGET=<path to libniftiio.a or libznz.a>

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

# Every defined global symbol, as " T name" entries.
string(REGEX MATCHALL "[ \t][TDBRS][ \t]+_?[A-Za-z_][A-Za-z0-9_]*" _globals "${_syms}")

set(_bare ${_globals})
list(FILTER _bare EXCLUDE REGEX "[ \t]_?minc_")
if(_bare)
  string(REGEX REPLACE "[ \t][TDBRS][ \t]+" "" _bare "${_bare}")
  message(FATAL_ERROR
    "check_nifti_mangled: symbols without the minc_ prefix in ${TARGET}: ${_bare}\n"
    "Add them to cmake-modules/nifti_mangle.h (see the recipe in its header).")
endif()

# Guards against a false pass on an archive that holds no NIfTI code at all,
# e.g. a slim-LTO object whose IR sections were stripped.
if(NOT _globals)
  message(FATAL_ERROR
    "check_nifti_mangled: no minc_ symbols in ${TARGET} -- the mangled NIfTI "
    "implementation is not present.")
endif()

list(LENGTH _globals _n)
message(STATUS "check_nifti_mangled: OK -- all ${_n} symbols in ${TARGET} carry the minc_ prefix")
