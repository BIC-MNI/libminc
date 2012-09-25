dnl Process this file with autoconf to produce a configure script.

AC_INIT([libminc],[2.2.00],[a.janke@gmail.com])

AC_CONFIG_SRCDIR([libsrc2/minc2.h])
AC_CONFIG_AUX_DIR(ac_config_aux)
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([config.h])

AC_REVISION($Revision: 6.45 $)

AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

AM_PROG_CC_C_O

AC_HEADER_TIME
AC_HEADER_DIRENT
AC_CHECK_HEADERS(sys/time.h sys/stat.h sys/wait.h unistd.h)
AC_CHECK_HEADERS(fcntl.h pwd.h float.h values.h)

AC_CHECK_TYPES([int32_t, int16_t])
# AC_DISABLE_SHARED
AC_PROG_LIBTOOL

smr_WITH_BUILD_PATH

# Check for required C libraries
mni_REQUIRE_LIB(m,[#include <math.h>],[double x = sqrt(3.);])
mni_REQUIRE_LIB(netcdf,[#include <netcdf.h>],[int i = ncopen("",0);])
AC_ARG_ENABLE(disminc2,
    [  --disable-minc2         disable HDF5 (MINC 2) functionality],
    [ case "${disableval}" in
      yes) disminc2=true ;;
      no)  disminc2=false ;;
      *) AC_MSG_ERROR(bad value ${disableval} for --disable-minc2) ;;
      esac],
    [disminc2=false])
AM_CONDITIONAL(MINC2, test x$disminc2 = xfalse)

# force certain HDF5 function calls to the 1.8.x versions
AC_DEFINE_UNQUOTED([H5Acreate_vers],[2],[Force the 1.8.x version])

if test x$disminc2 = xfalse; then
mni_REQUIRE_LIB(z, [#include <zlib.h>],[compress2;])
mni_REQUIRE_LIB(hdf5,[#include <hdf5.h>],[int f = H5Fopen("",0,H5P_DEFAULT);])
AC_DEFINE_UNQUOTED([MINC2],[1],[Define if MINC 2.0 is enabled.])
else
AC_DEFINE_UNQUOTED([MINC2],[0],[Define if MINC 2.0 is enabled.])
fi

AC_REPLACE_FUNCS(strdup)
AC_CHECK_FUNCS(mkstemp tempnam tmpnam)
AC_CHECK_FUNCS(getpwnam select strerror sysconf)

# Functions required for execute_decompress_command().
AC_FUNC_FORK
AC_CHECK_FUNCS(system popen)

AC_CONFIG_FILES([
Makefile 
epm-header
testdir/Makefile
])


