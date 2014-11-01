AC_PREREQ([2.63])
AC_INIT([libetnaviv], [1.0.0], [], [libetnaviv])

AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([1.10 foreign dist-bzip2])
AM_MAINTAINER_MODE([enable])

# Enable quiet compiles on automake 1.11.
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Check for programs
AC_PROG_CXX
AC_PROG_CC

AC_USE_SYSTEM_EXTENSIONS

# Initialize libtool
LT_PREREQ([2.2])
LT_INIT([disable-static])

AC_CHECK_FUNCS([clock_gettime], [CLOCK_LIB=],
               [AC_CHECK_LIB([rt], [clock_gettime], [CLOCK_LIB=-lrt],
                             [AC_MSG_ERROR([Could not find clock_gettime])])])
AC_SUBST([CLOCK_LIB])

# Userspace Vivante logging 
AC_ARG_ENABLE(debug,
              AC_HELP_STRING([--enable-debug],
                             [Enable debug output [[default=no]]]),
              [ENABLE_DEBUG="$enableval"],
              [ENABLE_DEBUG=no])

AC_ARG_ENABLE(vivhook,
              AC_HELP_STRING([--enable-vivhook],
                             [Enable Vivante libGAL userspace logging [[default=no]]]),
              [ENABLE_VIVHOOK="$enableval"],
              [ENABLE_VIVHOOK=no])

AC_ARG_WITH(galcore-include,
            AC_HELP_STRING([--with-galcore-include=PATH],
                           [specify directory for installed galcore include files [[default=/usr/local/include/HAL]]]),
            [galcore_include="$withval"],
            [galcore_include="/usr/local/include/HAL"])

GALCORE_CFLAGS="-I$galcore_include"

AS_IF([test x$ENABLE_DEBUG != xno],
      [
	AC_DEFINE(DEBUG, 1, [Enable debugging code])
])

AS_IF([test x$ENABLE_VIVHOOK != xno],
      [
   PKG_CHECK_MODULES(VIVHOOK, [libvivhook >= 1.0.0], [HAVE_VIVHOOK=yes], [HAVE_VIVHOOK=no])
])

AC_MSG_CHECKING([whether to build with debugging support])
AS_IF([test x$ENABLE_DEBUG = xyes],
      [AC_DEFINE(HAVE_ENABLE_DEBUG,1,[Enable debugging support])])
AM_CONDITIONAL(HAVE_ENABLE_DEBUG, test x$ENABLE_DEBUG = xyes)
AC_MSG_RESULT([$ENABLE_DEBUG])

AC_MSG_CHECKING([whether to build with vivhook userspace logging support])
AS_IF([test x$HAVE_VIVHOOK = xyes],
      [AC_DEFINE(HAVE_ENABLE_VIVHOOK,1,[Enable libvivhook userspace logging support])])
AM_CONDITIONAL(HAVE_ENABLE_VIVHOOK, test x$ENABLE_VIVHOOK = xyes)
AC_MSG_RESULT([$ENABLE_VIVHOOK])

saved_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $GALCORE_CFLAGS"
AC_CHECK_HEADERS([gc_hal.h gc_hal_base.h],
                 [],
                 [AC_MSG_ERROR([required galcore header file missing])])
AC_CHECK_HEADERS([gc_hal_driver.h gc_hal_kernel_buffer.h],
                 [],
                 [AC_MSG_ERROR([required galcore header file missing])],
                 [#include <gc_hal.h>])
CPPFLAGS="$saved_CPPFLAGS"

AC_SUBST([GALCORE_CFLAGS])

pkgconfigdir=${libdir}/pkgconfig
AC_SUBST(pkgconfigdir)

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	libetnaviv.pc
])
AC_OUTPUT
