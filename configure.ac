#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.65)
AC_INIT([libgtkmapserver], [0.0.1], [azagli@libero.it])
AC_CONFIG_SRCDIR([src/gtkmapserver.c])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE(-Wall)

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AM_MAINTAINER_MODE

AC_CONFIG_MACRO_DIR([m4])

AC_CANONICAL_SYSTEM

LT_INIT

AC_LIBTOOL_WIN32_DLL

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AC_PROG_RANLIB

dnl ******************************
dnl Translations
dnl ******************************
GETTEXT_PACKAGE=libgtkmapserver
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE",
                   [The prefix for our gettext translation domains.])
AM_GLIB_GNU_GETTEXT

# Checks for libraries.
PKG_CHECK_MODULES(GTKMAPSERVER, [gtk+-3.0 >= 3
                                 goocanvas-2.0 >= 2
                                 libsoup-2.4 >= 2.36])

AC_SUBST(GTKMAPSERVER_CFLAGS)
AC_SUBST(GTKMAPSERVER_LIBS)

# Checks for header files.
AC_HEADER_STDC

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

# Checks for library functions.

# Checks for library functions.

dnl ******************************
dnl Check for Operating System
dnl ******************************

platform_win32=no

case "$host" in
*-mingw*)
    platform_win32=yes
    ;;
esac

AM_CONDITIONAL(PLATFORM_WIN32, [test $platform_win32 = yes])

# Output files
AC_CONFIG_FILES([
	libgtkmapserver.pc
	Makefile
	src/Makefile
	tests/Makefile
])
AC_OUTPUT
