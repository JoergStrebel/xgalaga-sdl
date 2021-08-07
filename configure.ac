dnl Process this file with autoconf to produce a configure script.
AC_INIT(XGalaga-SDL, 2.1.1.0-sdl)
AM_INIT_AUTOMAKE

dnl Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O

# Enable compilation warnings if gcc is used.
if test "${ac_cv_c_compiler_gnu}" = "yes"; then
  CFLAGS="$CFLAGS \
    -ggdb \
    -Wall \
	-W \
    -Wcast-align \
    -Wcast-qual \
    -Wmissing-declarations \
    -Wmissing-prototypes \
    -Wpointer-arith \
    -Wreturn-type \
    -Wstrict-prototypes \
    "
fi # ac_cv_c_compiler_gnu

SDL_CFLAGS=`sdl2-config --cflags`
SDL_LIBS=`sdl2-config --libs`
SDL_LIBS="$SDL_LIBS -lSDL2_ttf -lSDL2_image -lSDL2_mixer -lSDL2_gfx"

# Silent rules; but check if supported.
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES([SDL2], [sdl2, SDL2_image, SDL2_mixer, SDL2_gfx])
PKG_CHECK_MODULES([SDL2_ttf], [SDL2_ttf])
AC_DEFINE([HAVE_LIBSDLX_TTF], [1], [Build with TTF font support])

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h malloc.h strings.h unistd.h)

dnl Checks for library functions.
AC_CHECK_FUNCS(random lrand48)

AC_SUBST(datadir)
AC_SUBST(bindir)
AC_SUBST(SDL_CFLAGS)
AC_SUBST(SDL_LIBS)

AC_CONFIG_HEADERS([src/config.h])

AC_OUTPUT(Makefile src/Makefile)

dnl Display configuration
AC_MSG_RESULT([
        $PACKAGE $VERSION
        =====

        prefix:                 ${prefix}
        sysconfdir:             ${sysconfdir}
        libdir:                 ${libdir}
        includedir:             ${includedir}
        datadir:                ${datadir}

        compiler:               ${CC}
        cflags:                 ${CFLAGS}
        ldflags:                ${LDFLAGS}
        SDL cflags:             ${SDL_CFLAGS}
        SDL libs:               ${SDL_LIBS}
        
])
