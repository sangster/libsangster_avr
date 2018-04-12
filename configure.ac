AC_PREREQ(2.69)
AC_INIT([libsangster_avr], [1.0.0])
AC_PREFIX_DEFAULT([/usr/avr])

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([build-aux])

AC_CONFIG_SRCDIR([src/sangster/pinout.h])

AM_INIT_AUTOMAKE([foreign -Wall -Werror])

# Checks for programs.
AC_PROG_CC([avr-gcc])
# AC_PROG_CC_C99
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB
AM_PROG_AR

# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_CHECK_FUNCS([memset])

AC_CHECK_PROGS([DOXYGEN], [doxygen], [
    AC_MSG_WARN([Doxygen not found - continuing without Doxygen support])
])
AM_CONDITIONAL([HAVE_DOXYGEN], [test -n "$DOXYGEN"])

# AC_CONFIG_FILES([Makefile
#                  docs/Makefile])
AC_CONFIG_FILES([Makefile
                 src/Makefile
                 docs/Makefile])
AM_COND_IF([HAVE_DOXYGEN], [AC_CONFIG_FILES([docs/Doxyfile])])


AC_OUTPUT
