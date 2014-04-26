pkglib_LTLIBRARIES = ShGraph.la

ShGraph_la_LDFLAGS = -module -export-dynamic -avoid-version
ShGraph_la_SOURCES = ShGraph.c

AM_CFLAGS = @DEBUG_CFLAGS@

INCLUDES = -I$(top_builddir) -I$(top_srcdir) \
	-I$(top_builddir)/src -I$(top_srcdir)/src \
	-I$(top_builddir)/libltdl -I$(top_srcdir)/libltdl \
	-I$(top_builddir)/modules/ShGraph -I$(top_srcdir)/modules/ShGraph
