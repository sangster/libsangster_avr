m4_define([SANGSTER_MAJOR], 0)
m4_define([SANGSTER_MINOR], 1)
m4_define([SANGSTER_PATCH], 0)
m4_define([SANGSTER_MCU], "atmega328p")
m4_define([SANGSTER_F_CPU], 16000000L)

# Define the libtool version numbers; check the Autobook, Section 11.4.
# Bump the libtool version numbers using the following algorithm:
#   if (the current interface has not been changed):
#       YAML_REVISION += 1
#   else:
#       YAML_REVISION = 0
#       YAML_CURRENT += 1
#       if (this release is backward compatible with the previous release):
#           YAML_AGE += 1
#       else:
#           YAML_AGE = 0
# m4_define([YAML_RELEASE], 0)
# m4_define([YAML_CURRENT], 2)
# m4_define([YAML_REVISION], 5)
# m4_define([YAML_AGE], 0)

# Initialize autoconf & automake.
AC_PREREQ(2.59)
AC_INIT([sangster_atmega328p], [SANGSTER_MAJOR.SANGSTER_MINOR.SANGSTER_PATCH], [])
# AC_CONFIG_AUX_DIR([config])
# AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([1.9 foreign])

# Define macro variables for the package version numbers.
AC_DEFINE(SANGSTER_VERSION_MAJOR, SANGSTER_MAJOR, [Define the major version number.])
AC_DEFINE(SANGSTER_VERSION_MINOR, SANGSTER_MINOR, [Define the minor version number.])
AC_DEFINE(SANGSTER_VERSION_PATCH, SANGSTER_PATCH, [Define the patch version number.])
AC_DEFINE(SANGSTER_VERSION_STRING, "SANGSTER_MAJOR.SANGSTER_MINOR.SANGSTER_PATCH", [Define the version string.])

# Define substitutions for the libtool version numbers.
# SANGSTER_LT_RELEASE=SANGSTER_RELEASE
# SANGSTER_LT_CURRENT=SANGSTER_CURRENT
# SANGSTER_LT_REVISION=SANGSTER_REVISION
# SANGSTER_LT_AGE=SANGSTER_AGE
# AC_SUBST(SANGSTER_LT_RELEASE)
# AC_SUBST(SANGSTER_LT_CURRENT)
# AC_SUBST(SANGSTER_LT_REVISION)
# AC_SUBST(SANGSTER_LT_AGE)

# Note: in order to update checks, run `autoscan` and look through "configure.scan".

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_NO_EXECUTABLES

MCU=SANGSTER_MCU
F_CPU=SANGSTER_F_CPU
AC_SUBST(MCU)
AC_SUBST(F_CPU)
AC_DEFINE(MCU, SANGSTER_MCU)
AC_DEFINE(F_CPU, SANGSTER_F_CPU)

# Checks for programs.
AC_PROG_CC([avr-gcc])
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

# AC_CHECK_PROG(DOXYGEN, [doxygen], [true], [false])
# AM_CONDITIONAL(DOXYGEN, [test "$DOXYGEN" = true])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Define Makefiles.
AC_CONFIG_FILES([sangster_atmega328p-0.1.pc src/Makefile Makefile])

# Generate the "configure" script.
AC_OUTPUT
