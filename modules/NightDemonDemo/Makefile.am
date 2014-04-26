pkglib_LTLIBRARIES = NightDemonDemo.la

NightDemonDemo_la_LDFLAGS = -module -export-dynamic -avoid-version
NightDemonDemo_la_SOURCES = NightDemonDemo.c ndd.c jpeg2surface.c
NightDemonDemo_la_LIBADD  = ../lib/libgraph.la ../lib/libalk.la @GLIB_LIBS@ @JPEG_LIBS@

AM_CFLAGS = @DEBUG_CFLAGS@ @GLIB_CFLAGS@ @JPEG_CFLAGS@

INCLUDES = -I$(top_builddir) -I$(top_srcdir) \
	-I$(top_builddir)/src -I$(top_srcdir)/src \
	-I$(top_builddir)/modules/lib -I$(top_srcdir)/modules/lib \
	-I$(top_builddir)/libltdl -I$(top_srcdir)/libltdl \
	-I$(top_builddir)/modules/NightDemonDemo -I$(top_srcdir)/modules/NightDemonDemo
