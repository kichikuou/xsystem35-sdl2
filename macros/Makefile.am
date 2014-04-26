## Please update this variable if any new macros are created

MACROS= \
  aclocal-include.m4 \
  ac_c_var_func.m4 \
  alsa.m4 \
  codeset.m4 \
  esd.m4 \
  freetype.m4 \
  freetype2.m4 \
  gtk.m4 \
  glib.m4 \
  libtool.m4 \
  sdl.m4 \
  pkg.m4 \
  vorbis.m4 \
  ogg.m4 \
  glib-2.0.m4 \
  gtk-2.0.m4 \
  ltdl.m4


EXTRA_DIST=$(MACROS)
MAINTAINERCLEANFILES=macros.dep

@MAINT@macros.dep: Makefile.am
@MAINT@ @echo '$$(top_srcdir)/aclocal.m4: $(MACROS:%=macros/%)' > $@
