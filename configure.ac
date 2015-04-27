dnl Process this file with autoconf to produce a configure script.

AC_INIT(libminc, [2.1.12], [robert.d.vincent@mcgill.ca])
AC_CONFIG_SRCDIR([libsrc/minc.h])
AC_CONFIG_AUX_DIR(ac_config_aux)
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([config.h])

AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

AM_PROG_CC_C_O
AC_PROG_RANLIB

AC_HEADER_TIME
AC_HEADER_DIRENT
AC_CHECK_HEADERS(sys/time.h sys/stat.h sys/wait.h unistd.h)
AC_CHECK_HEADERS(fcntl.h pwd.h float.h values.h)

AC_CHECK_TYPES([int32_t, int16_t])
dnl Build only static libs by default
AC_DISABLE_SHARED
AC_PROG_LIBTOOL

# Check for required C libraries
AC_CHECK_LIB(m,sqrt)
AC_CHECK_LIB(netcdf,ncopen, [], [AC_MSG_ERROR([Can't find -lnetcdf])])

# Override default behavior, as znzlib.c uses HAVE_ZLIB, not HAVE_LIBZ.
AC_CHECK_LIB(z, compress2, [
                LIBS="-lz $LIBS"
                AC_DEFINE([HAVE_ZLIB], [1], [Define to 1 if you have 'z' library (-lz)])
                ])
AC_CHECK_LIB(hdf5,H5Fopen, [], [AC_MSG_ERROR([Can't find -lhdf5])])
# Build the whole works.
AC_DEFINE([HAVE_MINC2],[1],[Define if MINC 2.0 is enabled.])
AC_DEFINE([HAVE_MINC1],[1],[Define if MINC 1.0 is enabled.])
AC_DEFINE([MINC2],[1],[Define if MINC 2.0 is enabled.])
AC_DEFINE([MINC_VERSION],[VERSION],[Library version.])

dnl Replacement implementations for systems that lack the function.

dnl need ac 2.57 for this? AC_CONFIG_LIBOBJ_DIR(libsrc)
AC_REPLACE_FUNCS(strdup)

dnl Verify existence of mkstemp, tempnam, and tmpnam
AC_CHECK_FUNCS(mkstemp tempnam tmpnam)

dnl Verify existence of some functions we'd like to use
AC_CHECK_FUNCS(getpwnam select strerror sysconf)

# Functions required for execute_decompress_command().
AC_FUNC_FORK
AC_CHECK_FUNCS(system popen)

AC_CHECK_FUNCS(sleep)

AC_CONFIG_FILES([
Makefile
])
AC_OUTPUT
