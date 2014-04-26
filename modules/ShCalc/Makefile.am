pkglib_LTLIBRARIES = ShCalc.la

ShCalc_la_LDFLAGS = -module -export-dynamic -avoid-version
ShCalc_la_SOURCES = ShCalc.c
ShCalc_la_LIBADD = @GLIB_LIBS@

AM_CFLAGS = @DEBUG_CFLAGS@ @GLIB_CFLAGS@

INCLUDES = -I$(top_builddir) -I$(top_srcdir) \
	-I$(top_builddir)/src -I$(top_srcdir)/src \
	-I$(top_builddir)/libltdl -I$(top_srcdir)/libltdl \
	-I$(top_builddir)/modules/ShCalc -I$(top_srcdir)/modules/ShCalc


