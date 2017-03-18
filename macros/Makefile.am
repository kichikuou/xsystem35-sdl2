## Please update this variable if any new macros are created

MACROS= \
  aclocal-include.m4 \
  argz.m4 \
  codeset.m4 \
  freetype2.m4 \
  gettext.m4 \
  glibc21.m4 \
  gtk-2.0.m4 \
  iconv.m4 \
  intdiv0.m4 \
  inttypes_h.m4 \
  inttypes.m4 \
  inttypes-pri.m4 \
  isc-posix.m4 \
  lcmessage.m4 \
  lib-ld.m4 \
  lib-link.m4 \
  lib-prefix.m4 \
  libtool.m4 \
  ltdl.m4 \
  lt~obsolete.m4 \
  ltoptions.m4 \
  ltsugar.m4 \
  ltversion.m4 \
  ogg.m4 \
  pkg.m4 \
  progtest.m4 \
  sdl2.m4 \
  stdint_h.m4 \
  uintmax_t.m4 \
  ulonglong.m4 \
  vorbis.m4


EXTRA_DIST=$(MACROS)
MAINTAINERCLEANFILES=macros.dep

@MAINT@macros.dep: Makefile.am
@MAINT@ @echo '$$(top_srcdir)/aclocal.m4: $(MACROS:%=macros/%)' > $@
