pkglib_LTLIBRARIES = ShString.la

ShString_la_LDFLAGS = -module -export-dynamic -avoid-version
ShString_la_SOURCES = ShString.c

AM_CFLAGS = @DEBUG_CFLAGS@

INCLUDES = -I$(top_builddir) -I$(top_srcdir) \
	-I$(top_builddir)/src -I$(top_srcdir)/src \
	-I$(top_builddir)/libltdl -I$(top_srcdir)/libltdl \
	-I$(top_builddir)/modules/ShString -I$(top_srcdir)/modules/ShString
