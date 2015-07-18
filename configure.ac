#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([jucpp], [1.0])
AC_CONFIG_SRCDIR([jucpp/http.cpp])

AC_SUBST([GLOBAL_CXXFLAGS], ['-Wall'])

# Checks for programs.
AX_CXX_COMPILE_STDCXX_11
AC_PROG_CXX
#AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h netdb.h netinet/in.h stddef.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT64_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_CHECK_FUNCS([dup2 gethostbyname inet_ntoa memchr memmove memset mkdir munmap rmdir select socket strchr strcspn strdup strerror strpbrk strrchr strspn])

AC_CONFIG_FILES([Makefile
                 jucpp/Makefile])
AC_OUTPUT
