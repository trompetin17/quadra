dnl Process this file with autoconf to produce a configure script.

AC_INIT
AC_CONFIG_SRCDIR([source/quadra.cpp])
AC_ARG_PROGRAM
AC_PROG_CXX
AC_CHECK_HEADERS([sys/utsname.h])
AC_CHECK_FUNCS([uname])
AM_QUAGMIRE
AC_DEFINE(UGS_LINUX, 1, [FIXME: temporary hack, until we properly detect stuff.])
AC_CONFIG_HEADERS([include/autoconf.h])
AC_OUTPUT
