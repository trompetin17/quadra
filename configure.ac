dnl Process this file with autoconf to produce a configure script.

AC_INIT
AC_PREREQ(2.61)
AC_CONFIG_SRCDIR([source/quadra.cpp])
AM_QUAGMIRE

AC_ARG_ENABLE(framerate,
	AS_HELP_STRING([--enable-framerate], [framerate indicator]))
AC_ARG_ENABLE(version-check,
	AS_HELP_STRING([--disable-version-check], [New version notification]))

AC_LANG(C++)
AC_ARG_PROGRAM
AC_PROG_CXX

AC_CHECK_LIB(SDL, SDL_Init, SDL_LIBS="-lSDL", nosdl=1)	
if test -n "$nosdl"; then
	AC_LANG_PUSH([Objective C])
	AC_CHECK_HEADER(Cocoa/Cocoa.h, withcocoa=1)
	AC_LANG_POP([Objective C])
	if test -n "$withcocoa"; then
		# If we're on a Cocoa platform, we'll rely on just the header check, and
		# assume the linker options, since autoconf does not have tests for Darwin
		# frameworks.
		AC_PROG_OBJC
		SDL_MAIN_SRCS=source/macosx/SDLMain.m
		SDL_LIBS="-framework SDL -framework Cocoa"
		unset nosdl
	fi
fi
AC_CHECK_LIB(z, compress,, nozlib=1)
AC_CHECK_LIB(png, png_create_read_struct,, nolibpng=1)
AC_CHECK_HEADER(SDL.h,, nosdl=1)
AC_CHECK_HEADER(zlib.h,, nozlib=1)
AC_CHECK_HEADER(png.h,, nolibpng=1)
AC_CHECK_HEADERS([sys/utsname.h])
AC_CHECK_FUNCS([uname])

if test -n "$nosdl"; then
	AC_MSG_WARN([are we ok?])
fi

if test -n "$nosdl"; then
	AC_MSG_WARN([SDL is required and could not be found!])
	missing_required=yes
fi

if test -n "$nozlib"; then
	AC_MSG_WARN([zlib is required and could not be found!])
	missing_required=yes
fi

if test -n "$nolibpng"; then
	AC_MSG_WARN([libpng is required and could not be found!])
	missing_required=yes
fi

if test -n "$missing_required"; then
	AC_MSG_ERROR([Required dependencies missing.])
fi

if test "$enable_framerate" = "yes"; then
   	AC_DEFINE(FRAMECOUNTER,, [Framerate indicator.])
fi

if test "$enable_version_check" != "no"; then
   	AC_DEFINE(ENABLE_VERSION_CHECK,, [Check for new versions.])
fi

AC_DEFINE(UGS_LINUX, 1, [FIXME: temporary hack, until we properly detect stuff.])
AC_SUBST(SDL_MAIN_SRCS, [$SDL_MAIN_SRCS])
AC_SUBST(SDL_LIBS, [$SDL_LIBS])

AC_CONFIG_HEADERS([include/autoconf.h])
AC_OUTPUT
