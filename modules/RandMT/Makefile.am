pkglib_LTLIBRARIES = RandMT.la

RandMT_la_LDFLAGS = -module -export-dynamic -avoid-version
RandMT_la_SOURCES = RandMT.c

AM_CFLAGS = @DEBUG_CFLAGS@

INCLUDES = -I$(top_builddir) -I$(top_srcdir) \
	-I$(top_builddir)/src -I$(top_srcdir)/src \
	-I$(top_builddir)/libltdl -I$(top_srcdir)/libltdl \
	-I$(top_builddir)/modules/RandMT -I$(top_srcdir)/modules/RandMT
