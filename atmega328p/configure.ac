m4_define([SANGSTER_MAJOR], 1)
m4_define([SANGSTER_MINOR], 0)
m4_define([SANGSTER_PATCH], 0)
m4_define([SANGSTER_MCU], "atmega328p")
m4_define([SANGSTER_F_CPU], 16000000L)


# Initialize autoconf & automake.
AC_PREREQ(2.65)
AC_INIT([libsangster_avr_atmega328p], [SANGSTER_MAJOR.SANGSTER_MINOR.SANGSTER_PATCH], [])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([build-aux])
AM_INIT_AUTOMAKE([1.9 foreign])
AC_PREFIX_DEFAULT("/usr/avr")

# Define macro variables for the package version numbers.
AC_DEFINE(SANGSTER_VERSION_MAJOR, SANGSTER_MAJOR, [Define the major version number.])
AC_DEFINE(SANGSTER_VERSION_MINOR, SANGSTER_MINOR, [Define the minor version number.])
AC_DEFINE(SANGSTER_VERSION_PATCH, SANGSTER_PATCH, [Define the patch version number.])
AC_DEFINE(SANGSTER_VERSION_STRING, "SANGSTER_MAJOR.SANGSTER_MINOR.SANGSTER_PATCH", [Define the version string.])

# Define substitutions for the libtool version numbers.
SANGSTER_LT_RELEASE=SANGSTER_RELEASE
SANGSTER_LT_CURRENT=SANGSTER_CURRENT
SANGSTER_LT_REVISION=SANGSTER_REVISION
AC_SUBST(SANGSTER_LT_RELEASE)
AC_SUBST(SANGSTER_LT_CURRENT)
AC_SUBST(SANGSTER_LT_REVISION)

LT_INIT([static])

MCU=SANGSTER_MCU
F_CPU=SANGSTER_F_CPU
AC_SUBST(MCU)
AC_SUBST(F_CPU)
AC_DEFINE(MCU, SANGSTER_MCU)
AC_DEFINE(F_CPU, SANGSTER_F_CPU)

# Checks for programs.
AC_PROG_CC([avr-gcc])
AC_PROG_CC_STDC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

# Doxygen
AC_CHECK_PROGS([DOXYGEN], [doxygen])
if test -x "$DOXYGEN"; then
    AC_MSG_WARN([Doxygen not found - continuing without Doxygen support])
fi
AM_CONDITIONAL([HAVE_DOXYGEN], [test -n "$DOXYGEN"])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_INT16_T
AC_TYPE_UINT32_T

PKG_CONFIG_PATH=$PKG_CONFIG_PATH:/usr/avr/lib/pkgconfig
export PKG_CONFIG_PATH

# PKG_INSTALLDIR([/usr/avr/lib/pkgconfig])
PKG_CHECK_MODULES([COMMON], [libsangster_avr_common-1.0 >= 1.0], [],
              AC_MSG_ERROR([Could not find AVR Common lib]);
)
# AC_CHECK_LIB([sangster_avr_common], [twi_handle_vect], [], [
#               AC_MSG_ERROR([Could not find AVR Common lib]);
# ]);

# Define Makefiles.
AC_CONFIG_FILES([libsangster_avr_atmega328p-1.0.pc src/Makefile docs/Makefile Makefile])
AM_COND_IF([HAVE_DOXYGEN], [AC_CONFIG_FILES([docs/Doxyfile])])

# Generate the "configure" script.
AC_OUTPUT
