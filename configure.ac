dnl
dnl configure.ac for xsystem35
dnl -----
AC_INIT(configure.ac)
AC_CANONICAL_TARGET

dnl
dnl Require autoconf version 2.59
dnl
AC_PREREQ(2.59)

dnl
dnl EXTRAxxx is for misc CFLAGS and LIBS
dnl
EXTRACFLAGS=
EXTRALIBS=
EXTRALDFLAGS="-rdynamic"

dnl
dnl versions
dnl
XSYS35_MAJOR=1
XSYS35_MINOR=7
XSYS35_MICRO=3
XSYS35_EXTRA=pre5

XSYS35_VERSION=$XSYS35_MAJOR.$XSYS35_MINOR.$XSYS35_MICRO$XSYS35_EXTRA
VERSION_MA=$XSYS35_MAJOR.$XSYS35_MINOR.$XSYS35_MICRO
VERSION_MI=$XSYS35_EXTRA
AC_SUBST(VERSION_MA)
AC_SUBST(VERSION_MI)

dnl
dnl For automake.
dnl 
VERSION=$XSYS35_VERSION
PACKAGE=xsystem35

dnl
dnl Initialize automake stuff
dnl
AM_INIT_AUTOMAKE($PACKAGE, $VERSION)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

dnl
dnl Pick up m4 macros
dnl 
dnl AM_ACLOCAL_INCLUDE(m4)
AM_ACLOCAL_INCLUDE(macros)

dnl
dnl add subdirectory for libltdl
dnl
dnl AC_CONFIG_SUBDIRS(libltdl)

dnl
dnl Checks for programs.
dnl
AC_PROG_CC
dnl AC_PROG_INSTALL
dnl AC_PROG_MAKE_SET
dnl AC_ISC_POSIX
dnl AC_C_VAR_FUNC 

dnl
dnl Check for libltdl
dnl 
AC_CONFIG_SUBDIRS(libltdl)
AC_LIBTOOL_DLOPEN
AC_LIBLTDL_CONVENIENCE([libltdl])
AC_LIB_LTDL
AC_PROG_LIBTOOL
AC_SUBST([LTDLINCL])
AC_SUBST([LIBLTDL])

dnl AM_PROG_LIBTOOL
dnl AC_SUBST(INCLTDL)
dnl AC_SUBST(LIBLTDL)
dnl AC_SUBST(LIBADD_DL)
dnl AC_LIBLTDL_INSTALLABLE
dnl AC_WITH_LTDL

dnl
dnl Check for gettext
dnl 
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION(0.11.5)

dnl
dnl Checks for header files.
dnl
AC_PATH_X

dnl
dnl Checks for typedefs, structures, and compiler characteristics.
dnl
dnl AC_C_CONST
dnl AC_C_INLINE
dnl AC_TYPE_PID_T
dnl AC_TYPE_SIZE_T
dnl AC_HEADER_TIME
dnl AC_STRUCT_TM
AC_C_BIGENDIAN
AC_CHECK_HEADERS(libgen.h)


dnl
dnl Check X11
dnl
AC_PATH_XTRA
AC_SUBST(X_CFLAGS)
AC_SUBST(X_LIBS)

dnl
dnl Checks for library functions.
dnl
AC_CHECK_FUNCS(gettimeofday mkdtemp)

dnl
dnl zlib
dnl
AC_CHECK_LIB(z, zlibVersion,
	[ AC_CHECK_HEADER(zlib.h,
          have_zlib=yes
          ZLIB_LIBS="-lz",)], AC_MSG_ERROR(zlib needed))
AC_SUBST(ZLIB_LIBS)



dnl
dnl check pkg-config program
dnl
dnl AC_PATH_PROG(PKGCONFIG,pkg-config,no)
dnl enable_pkgc=no
dnl if test x"$PKGCONFIG" != "xno"; then
dnl   AC_ARG_ENABLE(pkg-config,
dnl      [  --enable-pkg-config     Enable pkg-config for check glib|gtk [default=no]],
dnl      [enable_pkgc=yes])
dnl fi

dnl
dnl check glib
dnl
no_glib12=no
must_gtk=no
AM_PATH_GLIB(1.2.0,
  AC_DEFINE(ENABLE_GLIB,1,[Define this if you have glib >= 1.2.0]),
    [AC_MSG_RESULT(No glib-1.2 found. Next check glib-2.0)
     no_glib12=yes
    ]
  )

if test "x$no_glib12" = xyes; then
  AM_PATH_GLIB_2_0(2.0.0,
    AC_DEFINE(ENABLE_GLIB,1,[Define this if you have glib >= 1.2.0]),
    [AC_MSG_RESULT(No glib-2.x found. Next check glib-1.0 in gtk)
     must_gtk=yes
    ]
  )
fi

dnl if test x"$enable_pkgc" = "xno"; then
dnl  AM_PATH_GLIB(1.2.0,
dnl	       AC_DEFINE(ENABLE_GLIB,1,[Define this if you have glib >= 1.2.0]),must_gtk=yes)
dnl  if test x"$must_gtk" = "xyes"; then
dnl    AC_MSG_RESULT("May be 1.0.x ?. You can't be disable gtk")
dnl  fi
dnl else
dnl  AC_ARG_WITH(glib-target,
dnl             [  --with-glib-target       target glib [default=glib]],
dnl             [glibtarget="glib$withval"],[glibtarget="glib"])
dnl  PKG_CHECK_MODULES(BASE_DEPENDENCEIS, $glibtarget >= 1.2.0)
dnl  GLIB_CFLAGS="`$PKGCONFIG --cflags $glibtarget`"
dnl  GLIB_LIBS="`$PKGCONFIG --libs $glibtarget`"
dnl  AC_DEFINE(ENABLE_GLIB,1,[Define this if you have glib >= 1.2.0])
dnl fi

dnl
dnl check for gtk
dnl
AC_ARG_ENABLE(gtk,
  [  --enable-gtk            Enable gtk-based widget [default=yes]],,
  [enable_gtk=yes])
if test "x$must_gtk" = xyes; then
  if test "x$enable_gtk" != xyes; then
    enable_gtk=yes
  fi
fi

no_gtk1x=no
have_gtk=no
if test "x$enable_gtk" = xyes; then
  AM_PATH_GTK(1.0.0,
    AC_DEFINE(ENABLE_GTK,1,[Define this if you have gtk+ >= 1.0.0]),
    [AC_MSG_RESULT(No gtk-1.x found. Next check gtk-2.x)
     no_gtk1x=yes
    ]
  )
  if test "x$no_gtk1x" = xyes; then
    AM_PATH_GTK_2_0(2.0.0,
      AC_DEFINE(ENABLE_GTK,1,[Define this if you have gtk+ >= 1.0.0]),
      AC_MSG_ERROR(*** check for GTK failed ***)
    )
  fi
  have_gtk=yes
fi


dnl if test x"$enable_gtk" = "xyes"; then
dnl   if test x"$enable_pkgc" = "xno"; then
dnl    AM_PATH_GTK(1.0.0,
dnl                AC_DEFINE(ENABLE_GTK,1,[Define this if you have gtk+ >= 1.0.0]),
dnl		AC_MSG_ERROR(*** check for GTK failed ***))
dnl  else
dnl    AC_ARG_WITH(gtk-target,
dnl               [  --with-gtk-target       target gtk+ [default=gtk+]],
dnl               [gtktarget="gtk+$withval"],[gtktarget="gtk+"])
dnl    PKG_CHECK_MODULES(BASE_DEPENDENCEIS, $gtktarget >= 1.0.0)
dnl    GTK_CFLAGS="`$PKGCONFIG --cflags $gtktarget`"
dnl    GTK_LIBS="`$PKGCONFIG --libs $gtktarget`"
dnl     AC_DEFINE(ENABLE_GTK,1,[Define this if you have gtk+ >= 1.0.0])
dnl   fi
dnl   have_gtk=yes
dnl fi

AM_CONDITIONAL(HAVE_GTK, test "x$have_gtk" = xyes)
SRC_MENU=
if test "x$have_gtk" = xyes ; then
  SRC_MENU="menu.c menu_callback.c menu_gui.c"
else
  SRC_MENU="menu_null.c"
fi
AC_SUBST(SRC_MENU)

dnl for gtk-1.0
if test "x$must_gtk" = xyes; then
  GLIB_CFLAGS="$GTK_CFLAGS"
  GLIB_LIBS="$GTK_LIBS"
fi


dnl
dnl audio check
dnl
AC_ARG_ENABLE(audio,
  [  --enable-audio[=mode_list]  Enable audio (Specify comma separated mode list):
                              default:  Automatically select audio device.
                              oss:      OSS (Open Sound System) device
                              sun:      sun style interface
                              alsa:     ALSA pcm device
                              esd:      EsounD],
  [ enable_audio=$enableval],
  [ enable_audio=yes])

dnl yes -> default
test "x$enable_audio" = xyes && enable_audio=default

if test "x$enable_audio" != xno; then
  for i in `echo $enable_audio | sed 's/,/ /g'`; do
    eval "enable_$i=yes"
  done
  if test "x$enable_default" = xyes; then
    enable_audio=default
  fi
fi

if test "x$enable_audio" = xdefault; then
  case "$target" in
    *-*-linux*|*-*-freebsd*|*-*-netbsd*|*-*-openbsd*)
      enable_oss=yes
      ;;
    *-*-solaris*)
      enable_sun=yes
      ;;
    *)
      enable_esd=yes
      ;;
  esac
fi


dnl
dnl Sun style audio interface
dnl
have_sunaudio=no
if test "x$enable_sun" = xyes; then
  AC_MSG_CHECKING(for Sun audio support)
  AC_TRY_COMPILE([
            #include <sys/types.h>
            #include <sys/audioio.h>
        ],[
            audio_info_t audio_info;
            AUDIO_INITINFO(&audio_info);
        ],[
            have_sunaudio=yes
        ])
  AC_MSG_RESULT($have_sunaudio)
  if test "x$have_sunaudio" = xyes; then
    AC_DEFINE(ENABLE_SUNAUDIO,1,[define this if you use sun stlye audio interface])
 fi
fi


dnl
dnl OSS
dnl
have_ossaudio=no
if test "x$enable_oss" = xyes; then
  AC_MSG_CHECKING(for OSS audio support)
  case "$target" in
      *-*-netbsd*)
        EXTRALIBS="$EXTRALIBS -lossaudio"
        have_ossaudio=yes
        ;;
      *-*-openbsd*)
        EXTRACFLAGS="$EXTRACFLAGS -I/usr/local/lib/oss/include"
        EXTRALIBS="$EXTRALIBS -lossaudio"
        have_ossaudio=yes
        ;;
      *-*-linux*|*-*-freebsd*)
        have_ossaudio=yes
        ;;
      *)
        have_ossaudio=no
        ;;
  esac
  AC_MSG_RESULT($have_ossaudio)
  if test "x$have_ossaudio" = xyes; then
    AC_DEFINE(ENABLE_OSS,1,[define this if you use OSS audio interface])
  fi
fi


dnl
dnl ALSA
dnl
dnl if test x"$enable_alsa" = "xyes"; then
dnl   AM_PATH_ALSA(0.5.0,
dnl           [ AC_DEFINE(ENABLE_ALSA,1,[Define this if you have Alsa (libasound) installed])
dnl           if test x"$have_alsa09" = "xyes"; then
dnl             AC_DEFINE(ENABLE_ALSA09,1,[Define this if you have alsa 0.9.x and more installed])
dnl           fi
dnl           if test x"$have_alsa05" = "xyes"; then
dnl             AC_DEFINE(ENABLE_ALSA05,1,[Define this if you have alsa 0.5.x installed])
dnl           fi
dnl         ],
dnl   AC_MSG_RESULT(*** All of ALSA dependent parts will be disabled ***))
dnl fi

if test "x$enable_alsa" = xyes; then
  AM_PATH_ALSA(0.9.0,
               [ AC_DEFINE(ENABLE_ALSA09,1,[Define this if you have alsa 0.9.x and more installed])
                 have_alsa09=yes
               ],
               have_alsa09=no)
  if test "x$have_alsa09" = xno; then
    AM_PATH_ALSA(0.5.0,
                 [ AC_DEFINE(ENABLE_ALSA05,1,[Define this if you have 0.5.x <= alsa < 0.9.0 installed])
                   have_alsa05=yes
                 ],
                 have_alsa05=no)
  fi
  if test "x$have_alsa09" = xno -a "x$have_alsa05" = xno; then
    AC_MSG_RESULT(*** All of ALSA dependent parts will be disabled ***)
  else
    AC_DEFINE(ENABLE_ALSA,1,[Define this if you have Alsa (libasound) installed])
  fi 
fi


dnl
dnl ESD
dnl
no_esd=yes
if test "x$enable_esd" = xyes; then
  AM_PATH_ESD(0.2.8,
	  AC_DEFINE(ENABLE_ESD,1,[Define this if you have ESD (libesd) installed]),
	  AC_MSG_RESULT(*** All of ESD dependent parts will be disabled ***))
fi


dnl
dnl Turn on default audio output mode
dnl
DEFAULT_PLAYMODE=
AC_ARG_WITH(default-output,
  [  --with-default-output=<mode>  Specify default output mode (optional):
                                   default|oss|alsa|esd|sun],
  [ if test "$enable_audio" != no; then
      DEFAULT_PLAYMODE=$withval
      eval "enable_$DEFAULT_PLAYMODE=yes"
    else
      AC_MSG_WARN(--with-default-output=$withval: audio is not enabled)
    fi]
)
AC_MSG_CHECKING(default output mode)
if test "x$DEFAULT_PLAYMODE" = x; then
  DEFAULT_PLAYMODE=`echo $enable_audio | sed 's/,.*//'`
fi
case ".$DEFAULT_PLAYMODE" in
  .default)  def_am=AUDIO_PCM_ANY ;;
  .oss)      def_am=AUDIO_PCM_OSS ;;
  .alsa)     def_am=AUDIO_PCM_ALSA ;;
  .esd)      def_am=AUDIO_PCM_ESD ;;
  .sun)      def_am=AUDIO_PCM_SUN ;;
  *)         def_am= ;;
esac
if test "x$have_alsa09" = xyes; then
  if test "x$def_am" != x; then
    def_am="$def_am09"
  fi
fi
AC_MSG_RESULT($def_am)
if test "x$def_am" != x; then
  AC_DEFINE_UNQUOTED(DEFAULT_AUDIO_MODE, $def_am, [default audio mode])
fi



dnl
dnl OSS/ALSA/ESD
dnl
SRC_AUDIO=
if test "x$have_ossaudio" = xyes; then
  SRC_AUDIO="$SRC_AUDIO audio_oss.c"
fi
if test "x$have_sunaudio" = xyes; then
  SRC_AUDIO="$SRC_AUDIO audio_sun.c"
fi
if test "x$have_alsa05" = xyes; then
  SRC_AUDIO="$SRC_AUDIO audio_alsa.c"
fi
if test "x$have_alsa09" = xyes; then
  SRC_AUDIO="$SRC_AUDIO audio_alsa09.c"
fi
if test "x$no_esd" != xyes; then
  SRC_AUDIO="$SRC_AUDIO audio_esd.c"
fi
AC_SUBST(SRC_AUDIO)

dnl
dnl cdrom device
dnl
AC_ARG_ENABLE(cdrom,
  [  --enable-cdrom[=mode_list]  Enable CD-adio (Specify comma separated mode list):
                              (default:  Automatically select cdrom device)
                              linux:    linux API (Linux/Solaris)
                              bsd:      BSD API (FreeBSD/NetBSD/OpenBSD)
                              irix:     IRIX API
                              mp3:      play mp3 file using external/piped player],
  [ enable_cdrom=$enableval],
  [ enable_cdrom=yes])

dnl yes -> default
test "x$enable_cdrom" = xyes -a enable_cdrom=default

if test "x$enable_cdrom" != xno; then
  for i in `echo $enable_cdrom | sed 's/,/ /g'`; do
    eval "enable_cd_$i=yes"
  done
  if test "x$enable_cd_default" = xyes; then
    enable_cdrom=default
  fi
fi

if test "x$enable_cdrom" = xdefault; then
  case "$target" in
    *-*-linux*|*-*-solaris*)
      enable_cd_linux=yes
      ;;
    *-*-freebsd*|*-*-netbsd*|*-*-openbsd*)
      enable_cd_bsd=yes
      ;;
    *-irix*)
      enable_cd_irix=yes
      ;;
    *-dec-*)
      enable_cd_mp3=yes
      ;;
    *)
      AC_MSG_WARN(No --enable-cdrom=default cdrom for $target)
      ;;
  esac
fi

SRC_CDROM=
if test "x$enable_cd_linux" = xyes; then
  SRC_CDROM="cdrom.Linux.c"
  AC_DEFINE(ENABLE_CDROM_LINUX,1,[Define this if your use cdrom ioctl in linux or Solaris])
elif test "x$enable_cd_bsd" = xyes; then
  SRC_CDROM="$SRC_CROM cdrom.FreeBSD.c"
  AC_DEFINE(ENABLE_CDROM_BSD,1,[Define this if you use cdrom iotctl in *BSD])
elif test "x$enable_cd_irix" = xyes; then
  SRC_CDROM="$SRC_CDROM cdrom.Irix.c"
  AC_DEFINE(ENABLE_CDROM_IRIX,1,[Define this if you use cdrom ctrl library in IRX])
else
  SRC_CDROM="$SRC_CDROM cdrom.empty.c"
fi
if test "x$enable_cd_mp3" = xyes; then
  SRC_CDROM="$SRC_CDROM cdrom.mp3.c"
  AC_DEFINE(ENABLE_CDROM_MP3,1,[Define this if you use external player for cdrom audio])
fi
AC_SUBST(SRC_CDROM)

AC_ARG_WITH(cdromdev,    [  --with-cdromdev=#       CDROM Device Name. [default=/dev/cdrom]],	cdromdev="$withval",cdromdev="/dev/cdrom")
AC_DEFINE_UNQUOTED(CDROM_DEVICE,"$cdromdev",[cdrom device])

dnl
dnl Check if X_LOCALE definition is necessary
dnl
AC_ARG_WITH(locale, [  --with-locale=LOCALE    locale name you want to use(ex. ja_JP.ujis) ])
AC_MSG_CHECKING(need -DX_LOCALE)

AC_TRY_RUN([
#include <stdio.h>
#include <locale.h>

int
main ()
{
  return setlocale (LC_ALL, "${with_locale}") == NULL;
}],
need_x_locale=no,
need_x_locale=yes,
need_x_locale=no)
AC_MSG_RESULT($need_x_locale)

if test "x$need_x_locale" = xyes; then
  EXTRACFLAGS="$EXTRACFLAGS -DX_LOCALE"
fi

dnl
dnl OS dependent param
dnl
case "$target" in
  *-*-linux*)
    ;;
  *-*-freebsd*)
    EXTRALIBS="$EXTRALIBS -lxpg4"
    EXTRACFLAGS="$EXTRACFLAGS -I/usr/local/include"
    EXTRALDFLAGS="$EXTRALDFLAGS -L/usr/local/lib"
    ;;
  *-irix*)
    chk_rgb=yes
    EXTRALIBS="$EXTRALIBS -lcdaudio -lds -lmediad"
    ;;
  sparc-*)
    chk_rgb=yes
    ;;
esac

dnl
dnl Force to set Color RGB order (need to check for sparc)
dnl
if test "x$chk_rgb" = xyes; then
  AC_ARG_ENABLE(bgr,
    [  --enable-bgr            Color order is BGR [default is no for most arch] 
                          You should check whether your X server is RGB or
                          BGR order, especially in sparc architecture.],,
    [ enable_bgr=yes ])
fi
if test "x$enable_bgr" = xyes; then
  AC_DEFINE(BGR_ORDER,1,[Define this if X11 Pixmap color order is BGR])
else
  AC_DEFINE(RGB_ORDER,1,[Define this if X11 Pixmap color order is RGB])
fi

dnl
dnl set ALSA param
dnl
if test "x$have_alsa" = xyes ; then
    AC_ARG_WITH(alsa-card,    [  --with-alsa-card=#      ALSA Card No.],
	alsacard="$withval",alsacard="0")
    AC_ARG_WITH(alsa-pcmdev,  [  --with-alsa-pcmdev=#    ALSA PCM device No.],
	alsapcmdev="$withval",alsapcmdev="0")
    AC_ARG_WITH(alsa-mixerdev,[  --with-alsa-mixerdev=#  ALSA Mixer device No.],
	alsamixerdev="$withval",alsamixerdev="0")
    ALSACARD=$alsacard
    AC_DEFINE_UNQUOTED(ALSACARD,"$alsacard",[alsa card no])
    MIXERDEV_ALSA=$alsamixerdev
    AUDIODEV_ALSA=$alsapcmdev
    AC_DEFINE_UNQUOTED(AUDIODEV_ALSA,"$AUDIODEV_ALSA",[aduio device])
    AC_DEFINE_UNQUOTED(MIXERDEV_ALSA,"$MIXERDEV_ALSA",[mixer device])
    AC_CHECK_LIB(asound,snd_mixer_element_read,ALSAMIX_H=mixer_alsa.h)
    AC_SUBST(ALSAMIX_H)
fi

dnl
dnl set OSS param
dnl
if test "x$have_ossaudio" = xyes ; then
    AC_ARG_WITH(audiodev,[  --with-oss-audiodev=DEVFILE DSP DEVICE],
       audiodev="$withval",ossaudiodev="/dev/dsp")
    AC_ARG_WITH(mixerdev,[  --with-oss-mixerdev=DEVFILE MIXER DEVICE],
       mixerdev="Ec$withval",ossmixerdev="/dev/mixer")
    AUDIODEV_OSS=$ossaudiodev
    MIXERDEV_OSS=$ossmixerdev
    AC_DEFINE_UNQUOTED(AUDIODEV_OSS,"$AUDIODEV_OSS",[aduio device])
    AC_DEFINE_UNQUOTED(MIXERDEV_OSS,"$MIXERDEV_OSS",[mixer device])
fi


dnl
dnl Select Midi Play Methods
dnl
AC_ARG_ENABLE(midi,
  [  --enable-midi[=mode_list]  Enable midi (Specify comma separated mode list):
                             extp:     External midi player(default)
                             raw:      Raw midi interface
                             seq:      Sequencer interface],
  [ enable_midi=$enableval],
  [ enable_midi=yes])

dnl yes -> default
test "x$enable_midi" = xyes && enable_midi=default

if test "x$enable_midi" != xno; then
  for i in `echo $enable_midi | sed 's/,/ /g'`; do
    eval "enable_midi_$i=yes"
  done
fi

if test "x$enable_midi" = xdefault; then
  case "$target" in
    *)
      enable_midi_extp=yes
      ;;
  esac
fi

SRC_MIDI=
if test "x$enable_midi_extp" = xyes; then
  SRC_MIDI="midi.extplayer.c"
  AC_DEFINE(ENABLE_MIDI_EXTPLAYER,1,[define this if you use external midi player])
fi
if test "x$enable_midi_raw" = xyes || test "x$enable_midi_seq" = xyes; then
  SRC_MIDI="$SRC_MIDI midi.rawmidi.c midifile.c"
  if  test "x$enable_midi_raw" = xyes; then
    AC_DEFINE(ENABLE_MIDI_RAWMIDI,1,[define this if you use oss raw midi interface])
  fi
  if  test "x$enable_midi_seq" = xyes; then
    AC_DEFINE(ENABLE_MIDI_SEQMIDI,1,[define thsi if you use oss sequencer interface])
  fi
fi
AC_SUBST(SRC_MIDI)

dnl 
dnl set midi player
dnl 
AC_ARG_WITH(midi-player,[  --with-midi-player=xxx  External Midi Player [default=playmidi -4]],
	midiplayer="$withval",midiplayer="playmidi -4")
AC_DEFINE_UNQUOTED(MIDI_PLAYER,"$midiplayer",[midi player])

dnl
dnl Quiet mode for external midi player
dnl 
AC_ARG_ENABLE(qmidi,[  --enable-qmidi          No display midi stdout],
	  AC_DEFINE(QUITE_MIDI,1,[define this if you stop midi stdout]))

dnl 
dnl Midi device for rawmidi mode
dnl
AC_ARG_WITH(mididev,    [  --with-mididev=#        MIDI Raw Device Name. [default=/dev/midi]], mididev="$withval", mididev="/dev/midi")
AC_DEFINE_UNQUOTED(MIDI_DEVICE,"$mididev",[midi device])

dnl 
dnl Sequencer device for seqmidi mode
dnl
AC_ARG_WITH(seqdev,    [  --with-seqdev=#         MIDI Sequencer Device Name. [default=/dev/sequencer]], mididev="$withval", seqdev="/dev/sequencer")
AC_DEFINE_UNQUOTED(SEQ_DEVICE,"$seqdev",[sequencer device])


dnl
dnl general cachae size 
dnl
AC_ARG_WITH(cachesize,[  --with-cachesize=#      General Cache size (MB)],
  AC_DEFINE_UNQUOTED(CACHE_TOTALSIZE,$withval,[General Cache size (MB)]))


dnl
dnl SDL
dnl
SRC_GRAPHDEV="xcore_draw.c xcore_video.c xcore_mode.c xcore_event.c xcore_cursor.c xcore_maskupdate.c image.c"
AC_ARG_ENABLE(sdl,
	  [  --enable-sdl            SDL [default=no]    ],,
	  [ enable_sdl=no ])
if test "x$enable_sdl" = xyes ; then
  KEEPLIBS=$LIBS
  KEEPCFLAGS=$CFLAGS
  KEEPLDFLAGS=$LDFLAGS
  AM_PATH_SDL(1.0.0,have_sdl=yes,have_sdl=no)
  LIBS=$SDL_LIBS
  CFLAGS=$SDL_CFLAGS
  AC_CHECK_LIB(SDL,SDL_NumJoysticks,have_sdljoy=yes)
  AC_MSG_CHECKING(for SDL RLE accell)
  sdlrle=no
  sdlralpha=no
  AC_TRY_RUN([
#include <SDL.h>
int main( int argc,char **argv) {
#ifndef SDL_RLEACCEL
  exit(1);
#else
  exit(0);
#endif
}
  ],
    AC_MSG_RESULT("yes");sdlrle=yes,
    AC_MSG_RESULT("no"),
    AC_MSG_RESULT("unknown")
  )
  if test "x$sdlrle" = xyes ; then
    AC_MSG_CHECKING(for SDL alpha value)
    AC_TRY_RUN([
#include <SDL.h>
int main( int argc,char **argv) {
	SDL_Surface *s1,*s2;
	unsigned short *x;
	s1=SDL_AllocSurface(SDL_SWSURFACE,8,8,16,0,0,0,0);
	s2=SDL_AllocSurface(SDL_SWSURFACE,8,8,16,0,0,0,0);
	SDL_FillRect(s1,NULL,0);
	SDL_FillRect(s2,NULL,0xFFFF);
	SDL_SetAlpha(s2,SDL_SRCALPHA|SDL_RLEACCEL,255);
	SDL_BlitSurface(s2,NULL,s1,NULL);
	x=s1->pixels;
	if( *x )
		exit(1);
	exit(0);
}
      ],    
      AC_MSG_RESULT("normal"),
      AC_MSG_RESULT("revese");sdlralpha=yes,
      AC_MSG_RESULT("unknown")
    )
  fi
  LIBS=$KEEPLIBS
  CFLAGS=$KEEPCFLAGS
  LDFLAGS=$KEEPLDFLAGS
fi
if test "x$have_sdl" = xyes ; then
  AC_DEFINE(ENABLE_SDL,1,[define this if you use SDL])
  SDL=1
  SRC_GRAPHDEV="sdl_video.c sdl_mode.c sdl_draw.c sdl_event.c sdl_image.c sdl_cursor.c image.c"
  AC_SUBST(SDL)
  if test "x$sdlrle" = xyes ; then
    AC_DEFINE(HAVE_SDLRLE,1,[define this if you have SDLRLE])
  fi
  if test "x$sdlralpha" = xyes ; then
    AC_DEFINE(HAVE_SDLRALPHA,1,[define this if you have SDLRALPHA])
  fi
fi
AC_SUBST(SRC_GRAPHDEV)

dnl
dnl joystick
dnl
AC_ARG_ENABLE(joy,
  [  --enable-joy            Use joystick (only linux and SDL) [default=no]],,
  [  enable_joy=no] )
if test "x$enable_joy" = xyes ; then
  if test "x$have_sdljoy" = xyes ; then
    AC_DEFINE(HAVE_SDLJOY,1,[define this if you use SDL joystick interface])
    JOY_S=joystick_sdl
  else
    AC_CHECK_HEADERS(linux/joystick.h,
      [JOY_S=joystick_linux],
      [JOY_S=joystick_dmy])
  fi
else
  JOY_S=joystick_dmy
fi
AC_SUBST(JOY_S)

AC_ARG_WITH(joydev,    [  --with-joydev=#         Joystick Device Name. [default=/dev/js0]],	joydev="$withval",joydev="/dev/js0")
AC_DEFINE_UNQUOTED(JOY_DEVICE,"$joydev",[joystick device])


dnl
dnl FreeType (Font)
dnl
have_ttf=no
SRC_FONT=
if test "x$have_sdl" = xyes; then
  if test "x$enable_gtk" = xyes; then
    SRC_FONT="font_gtk.c"
    AC_DEFINE(ENABLE_GTKFONT,1,[define this if you use gtk(gdk) font interface])
  fi
else
  SRC_FONT="font_x11.c"
  AC_DEFINE(ENABLE_X11FONT,1,[define this if you use X11 font interface])
fi
AC_CHECK_FT2(2.0.0,
	     AC_DEFINE(ENABLE_FT2,1,[Define this if you have freetype2 installed]))
if test "x$no_ft" = xyes; then
  AM_PATH_FREETYPE(1.0.0,
	AC_DEFINE(ENABLE_TTF,1,[Define this if you have old(1.x) freetype installed]) have_ttf=yes,have_ttf=no)
fi
if test "x$no_ft" != xyes; then
   SRC_FONT="$SRC_FONT font_freetype2.c cp932tojis0213.c"
   EXTRALIBS="$EXTRALIBS $FT2_LIBS"
   EXTRACFLAGS="$EXTRACFLAGS $FT2_CFLAGS"
elif test "x$have_ttf" = xyes; then
   SRC_FONT="$SRC_FONT font_ttf.c"
   EXTRALIBS="$EXTRALIBS $FREETYPE_LIBS"
   EXTRACFLAGS="$EXTRACFLAGS $FREETYPE_INCLUDES"
fi
dnl
dnl check null
if test x"SRC_FONT" = "x"; then
  AC_MSG_ERROR("*** No font device available ***");
fi
AC_SUBST(SRC_FONT)


dnl
dnl XFree86extendions
dnl
AC_CHECK_LIB(Xxf86vm, XF86VidModeQueryExtension,
	[ AC_DEFINE(HAVE_XF86VMODE,1,[Define this if you have libXxf86vm installed])
        XXF86VM_LIB="-lXxf86vm"],,
	$X_LIBS $X_PRE_LIBS -lXext -lX11 $X_EXTRA_LIBS)
X_LIBS="$X_LIBS $XXF86VM_LIB -lXext"

AC_ARG_ENABLE(xrender,
  [  --enable-xrender        Use XRender extension (future work) [default=disable]],,
  enable_xrender="no")
if test "x$enable-xrender" = xyes; then
  AC_DEFINE(ENABLE_XRENDER,1,[define this if you use X11 render etension])
fi


dnl
dnl MMX
dnl 
SRC_ABLEND=
AC_PATH_PROG(HAVE_NASM,nasm,no)
if test "x$HAVE_NASM" != xno; then
  case "$target" in
    *i*86-*)
      AC_ARG_ENABLE(mmx,
           [  --enable-mmx            MMX support [default=yes]],,
           [ enable_mmx=yes ])
      if test "x$enable_mmx" = xyes ; then
        AC_DEFINE(ENABLE_MMX,1,[define this if you want to use MMX instructions])
        SRC_ABLEND="alpha_blend16.s haveunit.s"
      fi;;
  esac
fi
AC_SUBST(SRC_ABLEND)


dnl
dnl Vorbis input
dnl
AM_PATH_OGG(
           [ AM_PATH_VORBIS([], 
                            AC_MSG_ERROR([OGG/VORBIS needed])) 
           ], 
           AC_MSG_ERROR([OGG/VORBIS needed]))
AM_CONDITIONAL(HAVE_VORBIS, [test "x$no_ogg" != xyes -a "x$no_vorbis" != xyes])

dnl
dnl Test for libjpeg
dnl
JPEG_CFLAGS=""
JPEG_LIBS=""
available_jpeg=no
AC_ARG_WITH(libjpeg-prefix,
   [  --with-libjpeg-prefix=PFX Prefix where libjpeg is install(optional)], libjpeg_prefix="$withval", libjpeg_prefix="")

if test "x$libjpeg_prefix" != x ; then
  JPEG_CFLAGS="-I$libjpeg_prefix/include"
  JPEG_LIBS="-L$libjpeg_prefix/lib"
fi

ac_save_CFLAGS="$CFLAGS"
ac_save_LIBS="$LIBS"
CFLAGS="$CFLAGS $JPEG_CFLAGS"   
LIBS="$LIBS $JPEG_LIBS"

AC_CHECK_LIB(jpeg, jpeg_destroy_decompress,
   available_jpeg=yes,
   available_jpeg=no)

if test "x$available_jpeg" = xyes; then
   AC_MSG_CHECKING([for jpeglib.h])
   AC_TRY_CPP(
[#include <stdio.h>
#undef PACKAGE
#undef VERSION
#undef HAVE_STDLIB_H
#include <jpeglib.h>],
      available_jpeg=yes,
      available_jpeg=no)
   AC_MSG_RESULT($available_jpeg)
   if test "x$available_jpeg" = xyes; then
     JPEG_LIBS="$JPEG_LIBS -ljpeg"
   fi
fi
CFLAGS="$ac_save_CFLAGS"
LIBS="$ac_save_LIBS"

if test "x$available_jpeg" != xyes; then
   AC_MSG_ERROR(libjpeg needed)
fi

AM_CONDITIONAL(ENABLE_JPEG, test "x$available_jpeg" = xyes)
AC_SUBST(JPEG_CFLAGS)
AC_SUBST(JPEG_LIBS)

enable_jpeg=$available_jpeg


dnl
dnl debugging
dnl
AC_ARG_ENABLE(debug, [  --enable-debug          Turn on debugging [default=no]],,enable_debug=no)

if test "x$enable_debug" = xyes; then
  DEBUG_CFLAGS="-g"
  if test "x$GCC" = xyes; then
    DEBUG_CFLAGS="$DEBUG_CFLAGS -Wall"
  fi
  AC_DEFINE(DEBUG,1,[Define this if you want to Debug])
else
  CFLAGS="`echo $CFLAGS | sed 's/-g//g'`"
  DEBUG_CFLAGS=""
fi
AC_SUBST(DEBUG_CFLAGS)


dnl
dnl Some infos:
dnl
echo "xsystem35 summary:"
echo "------------------"

dnl Audio
echo " * audio output:"
if test "x$have_ossaudio" = xyes; then
    echo "    - oss (Open Sound System)"
fi
if test "x$have_sunaudio" = xyes; then
    echo "    - sun ()"
fi
if test "x$no_alsa" != xyes; then
  if test "x$have_alsa05" = xyes; then
    echo "    - alsa05 (Alsa 0.5.x)"
  fi
  if test "x$have_alsa09" = xyes; then
    echo "    - alsa09 (Alsa 0.9.x)"
  fi
fi
if test "x$no_esd" != xyes; then
  echo "    - esd (Enlightened Sound Daemon)"
fi
echo ""

dnl CDROM
echo " * cdrom output:"
if test "x$enable_cd_linux" = xyes; then
  echo "    - Linux|Solaris ioctl"
fi
if test "x$enable_cd_bsd" = xyes; then
  echo "    - *BSD ioctl"
fi
if test "x$enable_cd_irix" = xyes; then
  echo "    - Irix cdrom library"
fi
if test "x$enable_cd_mp3" = xyes; then
  echo "    - External player (wav|mp3|ogg...)"
fi
echo ""

dnl MIDI
echo " * midi output:"
if test "x$enable_midi_raw" = xyes; then
  echo "    - raw midi interface"
fi
if test "x$enable_midi_seq" = xyes; then
  echo "    - OSS sequencer interface"
fi
if test "x$enable_midi_extp" = xyes; then
  echo "    - External MIDI Player"
fi
echo ""

dnl font
echo " * font device:"
if test "x$have_sdl" = xyes; then
  if test "x$enable_gtk" = xyes; then
    echo "    - gtk(gdk) font"
  fi
else
  echo "    - x11 font"
fi
if test "x$have_ttf" = xyes; then
  echo "    - old truetype font library"
fi
if test "x$no_ft" != xyes; then
  echo "    - freetype2 library"
fi
echo ""

echo "----"

AC_SUBST(EXTRACFLAGS)
AC_SUBST(EXTRALIBS)
AC_SUBST(EXTRALDFLAGS)
AC_CONFIG_FILES([
po/Makefile.in
Makefile
xsystem35.spec
modules/ShArray/Makefile
modules/ShCalc/Makefile
modules/ShString/Makefile
modules/ShPort/Makefile
modules/nDEMO/Makefile
modules/nDEMOE/Makefile
modules/NIGHTDLL/Makefile
modules/oujimisc/Makefile
modules/RandMT/Makefile
modules/Math/Makefile
modules/Gpx/Makefile
modules/MsgSkip/Makefile
modules/ShSound/Makefile
modules/ShGraph/Makefile
modules/dDemo/Makefile
modules/oDEMO/Makefile
modules/SACT/Makefile
modules/tDemo/Makefile
modules/eDemo/Makefile
modules/eeDemo/Makefile
modules/Confirm/Makefile
modules/AliceLogo/Makefile
modules/NightDemonDemo/Makefile
modules/lib/Makefile
modules/Makefile
src/Makefile
macros/Makefile
])
AC_OUTPUT
