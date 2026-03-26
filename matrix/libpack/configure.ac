#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([pack],[0.0.0],[nikita.shubin@kemeke.ru])
AM_INIT_AUTOMAKE([pack],[0.0.0])
AC_CONFIG_SRCDIR([src/crc16.c])
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_INSTALL
# Checks for programs.
AC_PROG_CC

AC_CANONICAL_HOST

# Init libtool
LT_INIT
# Checks for libraries
AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h])

AC_ARG_ENABLE(debug,
AS_HELP_STRING([--enable-debug],
               [enable debugging, default: no]),
[case "${enableval}" in
             yes) debug=true ;;
             no)  debug=false ;;
             *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
esac],
[debug=false])

AM_CONDITIONAL(DEBUG, test x"$debug" = x"true")

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([
Makefile 
src/Makefile
])

AC_PREFIX_DEFAULT()

AC_OUTPUT
