dnl Process this file with autoconf to produce a configure script.
AC_INIT(yapa, 2.0.0)
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)

AC_SUBST(PACKAGE)
AC_SUBST(VERSION)

AC_CANONICAL_HOST

dnl Checks for programs.
AC_GNU_SOURCE
AC_PROG_CC
dnl If we're using gcc, we want warning flags
test -n "$GCC" && WARNFLAGS="-W -Wall -Wbad-function-cast -Wcast-align -Winline -Wmissing-declarations -Wmissing-prototypes -Wnested-externs -Wpointer-arith -Wshadow -Wstrict-prototypes -Wundef -Werror"
AC_SUBST(WARNFLAGS)

AC_CHECK_LIB(Imlib2,imlib_load_image,IMLIB2_LIBS="-lImlib2",IMLIB2_LIBS="")
AC_SUBST(IMLIB2_LIBS)
AC_CHECK_LIB(exif,exif_data_new_from_file,EXIF_LIBS="-lexif",EXIF_LIBS="")
AC_SUBST(EXIF_LIBS)

AH_VERBATIM([_ZZENABLE_NLS],
[#ifdef ENABLE_NLS
#include <libintl.h>
#define _(msgid) dgettext(PACKAGE, msgid)
#define N_(msgid) msgid
#else
#define _(msgid) (msgid)
#define N_(msgid) msgid
#endif /* ENABLE_NLS */])

AC_OUTPUT(Makefile src/Makefile)
