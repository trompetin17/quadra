dnl Process this file with autoconf to produce a configure script.

AC_INIT
AC_CONFIG_SRCDIR([source/quadra.cpp])

AC_ARG_ENABLE(framerate,
	AS_HELP_STRING([--enable-framerate], [framerate indicator]))
AC_ARG_ENABLE(version-check,
	AS_HELP_STRING([--disable-version-check], [New version notification]))

AC_ARG_PROGRAM
AC_PROG_CXX
AC_CHECK_HEADERS([sys/utsname.h])
AC_CHECK_FUNCS([uname])
AM_QUAGMIRE

if test "$enable_framerate" = "yes"; then
   	AC_DEFINE(FRAMECOUNTER,, [Framerate indicator])
fi

if test "$enable_version_check" != "no"; then
   	AC_DEFINE(ENABLE_VERSION_CHECK,, [Check for new versions.])
fi

AC_DEFINE(UGS_LINUX, 1, [FIXME: temporary hack, until we properly detect stuff.])
AC_CONFIG_HEADERS([include/autoconf.h])
AC_OUTPUT
