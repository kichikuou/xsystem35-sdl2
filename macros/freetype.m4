dnl
dnl Borrow form Berlin Project.
dnl http://www.berlin-consortium.org
dnl
dnl Checks if FreeType is found.  If it is, $ac_cv_lib_freetype is
dnl set to "yes".

AC_DEFUN([AM_PATH_FREETYPE],
[dnl Get the cflags and librarie
  AC_LANG_SAVE
  AC_LANG_C
  AC_ARG_WITH(freetype-prefix, [  --with-freetype-prefix=PFX  Prefix for Freetype (optional)],
		freetype_prefix="$withval", freetype_prefix="")

dnl Check for Freetype includes.
  if test ".$freetype_prefix" != . ; then
    FREETYPE_INCLUDES=-I$freetype_prefix/include
  fi
  save_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$FREETYPE_INCLUDES $CPPFLAGS"
  AC_CHECK_HEADER(freetype/freetype.h,AC_DEFINE(FREETYPE_HAVE_DIR,1,[define this if you have freetype/freetype.h]))
  CPPFLAGS="$save_CPPFLAGS"

dnl Check for Freetype libs
  if test x$no_freetype = x ; then
    if test x$freetype_prefix != x ; then
      freetype_libs=-L$freetype_prefix/lib
    fi
    freetype_libs="$freetype_libs -lttf"

    AC_CACHE_CHECK([for working Freetype environment],
		ac_cv_lib_freetype, [
		save_LDFLAGS="$LDFLAGS"
		save_CPPFLAGS="$CPPFLAGS"
		save_LIBS="$LIBS"
		LIBS="$LIBS $freetype_libs"
		LDFLAGS="$LDFLAGS"
		CPPFLAGS="$CPPFLAGS $FREETYPE_INCLUDES"
		
		dnl Check if everything works
		AC_TRY_RUN([
#ifdef FREETYPE_HAVE_DIR
#include <freetype/freetype.h>
#else
#include <freetype.h>
#endif
#include <stdio.h>
int main (int argc, char* argv[])
{
  TT_Engine library;
  if(TT_Init_FreeType(&library) != 0)
    {
     printf("Error: Could not initialize FreeType engine!\n");
      return 1;
    }
  return 0;
}
 		], ac_cv_lib_freetype=yes,
 		ac_cv_lib_freetype=no,
 		ac_cv_lib_freetype=yes)
 
	CPPFLAGS="$save_CPPFLAGS"
	LDFLAGS="$save_LDFLAGS"
 	LIBS="$save_LIBS"
	]) dnl End of AC_CACHE_CHECK

  fi

  if test x$ac_cv_lib_freetype = xyes ; then
    FREETYPE_LIBS=$freetype_libs
  ifelse([$2], , :, [$2])   
  else

  if test ".$1" = .mandatory ; then
    AC_MSG_ERROR(Could not find freetype library!)
  fi
  ifelse([$3], , :, [$3])
  fi

  AC_SUBST(FREETYPE_LIBS)
  AC_SUBST(FREETYPE_INCLUDES)

  AC_LANG_RESTORE
])
