AC_PREREQ([2.69])
AC_INIT([libweb], [1.0])
AC_CONFIG_SRCDIR([Server.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIRS([m4])

AC_PROG_CXX
AC_PROG_CC

AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_PROG_AR

LT_INIT()

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [main])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_UINT16_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_CHECK_FUNCS([socket strerror])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
