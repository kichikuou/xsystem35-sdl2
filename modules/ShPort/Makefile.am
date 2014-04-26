pkglib_LTLIBRARIES = ShPort.la

ShPort_la_LDFLAGS = -module -export-dynamic -avoid-version
ShPort_la_SOURCES = ShPort.c
ShPort_la_LIBADD = @GLIB_LIBS@

AM_CFLAGS = @DEBUG_CFLAGS@ @GLIB_CFLAGS@

INCLUDES = -I$(top_builddir) -I$(top_srcdir) \
	-I$(top_builddir)/src -I$(top_srcdir)/src \
	-I$(top_builddir)/libltdl -I$(top_srcdir)/libltdl \
	-I$(top_builddir)/modules/ShPort -I$(top_srcdir)/modules/ShPort
