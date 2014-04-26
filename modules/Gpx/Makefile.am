pkglib_LTLIBRARIES = Gpx.la

Gpx_la_LDFLAGS = -module -export-dynamic -avoid-version
Gpx_la_LIBADD   = ../lib/libgraph.la @GLIB_LIBS@
Gpx_la_SOURCES = Gpx.c \
	effectcopy.c

AM_CFLAGS = @DEBUG_CFLAGS@ @GLIB_CFLAGS@

INCLUDES = -I$(top_builddir) -I$(top_srcdir) \
	-I$(top_builddir)/src -I$(top_srcdir)/src \
	-I$(top_builddir)/libltdl -I$(top_srcdir)/libltdl \
	-I$(top_builddir)/modules/lib -I$(top_srcdir)/modules/lib \
	-I$(top_builddir)/modules/Gpx -I$(top_srcdir)/modules/Gpx
