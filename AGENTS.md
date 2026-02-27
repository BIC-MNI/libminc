# AGENTS.md - libminc

Guidelines for AI coding agents working in this repository.

## Project Overview

libminc is a C/C++ library for the MINC (Medical Image NetCDF) file format,
used for medical imaging I/O. It produces `libminc2` (C) and `libminc_io`
(C++ ezminc wrapper). Dependencies: HDF5 (required), ZLIB (required),
NetCDF (optional, for MINC1), NIfTI (optional).

## Build Commands

```bash
# Configure (from repo root)
cmake -B build -DCMAKE_BUILD_TYPE=Release \
  -DLIBMINC_MINC1_SUPPORT=ON \
  -DBUILD_TESTING=ON

# Build
cmake --build build -j$(nproc)

# Run all tests
cd build && ctest --verbose

# Run a single test by name (use -R for regex match)
cd build && ctest --verbose -R minc2-full-test
cd build && ctest --verbose -R "minc2-hyper-test$"

# List all available test names
cd build && ctest -N

# Run a single test executable directly (from build/testdir/)
./testdir/minc2-full-test
```

### Key CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `LIBMINC_BUILD_SHARED_LIBS` | OFF | Build shared libraries |
| `LIBMINC_MINC1_SUPPORT` | OFF | Enable MINC1 format (needs NetCDF) |
| `LIBMINC_BUILD_EZMINC` | ON | Build C++ ezminc wrapper |
| `LIBMINC_USE_NIFTI` | OFF | NIfTI format support |
| `LIBMINC_USE_ASAN` | OFF | Address Sanitizer |
| `BUILD_TESTING` | ON | Build test suite |

### No Linter/Formatter

There is no clang-format, clang-tidy, or other automated formatting tool
configured. Match the style of the surrounding code when making changes.

## Directory Structure

- `libsrc/` - MINC1 API (C), header: `minc.h`
- `libsrc2/` - MINC2 API (C), header: `minc2.h`
- `libcommon/` - Shared utilities (error handling, arg parsing, timestamps)
- `volume_io/` - Volume I/O library (C), header: `volume_io.h`
- `ezminc/` - C++ wrapper ("Easy MINC"), classes in `namespace minc`
- `nifti/` - Bundled NIfTI-1 I/O library
- `testdir/` - C test suite (CTest)
- `ezminc/tests/` - C++ test suite
- `cmake-modules/` - Custom CMake Find/Build modules

## Code Style Guidelines

### Naming Conventions

- **Public C functions (MINC2):** `mi` prefix + snake_case: `miopen_volume`, `miget_data_type`, `micreate_dimension`
- **Internal/private C functions:** `MI_` prefix or `_` prefix: `MI_convert_type`, `_generate_ident`
- **Typedefs:** lowercase + `_t` suffix: `mihandle_t`, `mitype_t`, `midimhandle_t`
- **Macros/constants:** `ALL_CAPS` with prefix: `MI_MAX_VAR_DIMS`, `MI2_OPEN_READ`, `MI_TYPE_FLOAT`
- **String constant macros:** `MI` + PascalCase: `MIxspace`, `MIhistory`, `MIvarid`
- **volume_io types:** `VIO_` prefix + PascalCase: `VIO_Point`, `VIO_Real`, `VIO_Volume`, `VIO_Status`
- **C++ classes (ezminc):** snake_case: `minc_1_reader`, `minc_1_writer`, `minc_1_base`
- **C++ member variables:** underscore prefix + snake_case: `_slab_len`, `_datatype`, `_io_datatype`
- **C++ methods:** snake_case: `setup_read_float`, `next_slice`, `close`
- **`PRIVATE` = `static`**, **`SEMIPRIVATE` = (empty)** -- macros defined in `libsrc/minc_useful.h`

### Include Style

- Angle brackets `<>` for system/external headers; quotes `""` for project-local headers
- Include guards use `#ifndef`/`#define` (no `#pragma once` anywhere)
- Use `extern "C" { }` wrapping when C headers are included from C++

### Formatting

There is no single enforced style. Match the subdirectory you are editing:

- **libsrc:** ~3-space indent, K&R brace style
- **libsrc2:** 2-space indent, K&R brace style (confirmed by kate modelines in files)
- **volume_io:** 4-space indent, Allman brace style, extra spaces: `return( value );`
- **ezminc:** 2-space indent, mixed Allman/K&R brace style
- Spaces (not tabs) are predominant in all subdirectories

### Types

- MINC2 API types: `mihandle_t` (volume handle), `midimhandle_t` (dimension), `mitype_t` (data type enum), `misize_t` (size type)
- volume_io types: `VIO_Real` (double), `VIO_Volume`, `VIO_Point`, `VIO_Vector`, `VIO_Status` (`VIO_OK`/`VIO_ERROR`)
- Standard C types are used throughout; `size_t` for sizes, `double` for floating-point
- Prefer the project's type aliases over raw types in the respective modules

### Error Handling

**MINC1/MINC2 C code** uses return codes with macro-based error logging:
```c
MI_SAVE_ROUTINE_NAME("function_name");
/* ... */
if (error_condition) {
  MI_LOG_ERROR(MI2_MSG_GENERIC, "description");
  MI_RETURN(MI_ERROR);
}
MI_RETURN(MI_NOERROR);
```
- `MI_NOERROR` = 0 (success), `MI_ERROR` = -1 (failure)
- Every function using these macros must call `MI_SAVE_ROUTINE_NAME` at entry

**volume_io** returns `VIO_Status` enum: `VIO_OK` or `VIO_ERROR`

**ezminc (C++)** uses exceptions:
- `minc::generic_error` class (note: does NOT inherit `std::exception`)
- `CHECK_MINC_CALL(expr)` macro wraps C API calls and throws on failure

### Comment Style

- **libsrc, volume_io:** MNI Header blocks with `@NAME`, `@DESCRIPTION`, `@CREATED` annotations
- **libsrc2:** Doxygen comments (`/** */`, `\param`, `\ingroup`, `\file`, `\brief`)
- **ezminc:** `//!` Doxygen-compatible comments for class/method documentation
- Prefer `/* */` for C code; `//` is acceptable in C++ and libsrc2

### C++ Specifics (ezminc)

- All code lives in `namespace minc { }`
- Pre-C++11 style: no smart pointers, `auto`, `override`, or `nullptr` in existing code
- Heavy STL usage: `std::vector`, `std::string`, `<iostream>`
- Partial RAII: destructors call `close()`, but construction requires explicit `open()`
- Template functions use `typeid()` for type dispatching

## Testing

Tests use CTest. Two patterns exist:
1. **Direct execution:** test binary runs, exit code 0 = pass
2. **Output comparison:** CMake scripts (`run_test_*.cmake`) run the binary and diff output against `.out` reference files in `testdir/`

Test files in `testdir/` are named after the API they test:
- `minc2-*.c` - MINC2 API tests
- `icv*.c` - Image Conversion Variable tests (MINC1)
- `volume_test.c` - Volume I/O tests
- `test_xfm.c` - Transform tests

Some tests have ordering dependencies set via `set_property(TEST ... DEPENDS ...)`.

## CI

GitHub Actions (`.github/workflows/cmake.yml`):
- Platforms: Ubuntu 22.04/24.04, macOS 14/15
- Matrix: static/shared, MINC1 on/off
- Build type: Release
- Uses ccache for build acceleration
