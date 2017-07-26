AC_PREREQ(2.59)
AC_INIT(cpp-common, 1.0.0.0, isold.wang@qq.com)
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE
AM_PROG_LIBTOOL

dnl debug release
AC_ARG_ENABLE([debug], AC_HELP_STRING([--enable-debug], [debug info (default is no)]),[debug=yes], [debug=no])
if test "$debug" = "no"; then
echo "compile with release"
CXXFLAGS="-w -O3 -DNDEBUG -Dx86-64"
else
echo "compile with debug"
CXXFLAGS="-O0 -g -ggdb -Wall -D_DEBUG -Dx86-64"
fi

LDFLAGS = " -lc -lpthread -o "

dnl Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

dnl Checks for libraries.
AC_CHECK_LIB([c], [main])
AC_CHECK_LIB([dl], [main])
AC_CHECK_LIB([pthread], [main])

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h memory.h netinet/in.h stdlib.h string.h sys/socket.h sys/time.h sys/timeb.h termios.h unistd.h])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

AC_CHECK_SIZEOF(int,4)

dnl Checks for library functions.
AC_FUNC_MALLOC
AC_TYPE_SIGNAL
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([ftime getpass gettimeofday inet_ntoa memmove memset mkdir socket strchr strrchr strstr strtol])

AC_CONFIG_FILES([Makefile
		include/Makefile
		src/Makefile])
AC_OUTPUT
