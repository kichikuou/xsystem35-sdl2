pkglib_LTLIBRARIES = ShArray.la

ShArray_la_LDFLAGS = -module -export-dynamic -avoid-version
ShArray_la_SOURCES = ShArray.c

AM_CFLAGS = @DEBUG_CFLAGS@

INCLUDES = -I$(top_builddir) -I$(top_srcdir) \
	-I$(top_builddir)/src -I$(top_srcdir)/src \
	-I$(top_builddir)/libltdl -I$(top_srcdir)/libltdl \
	-I$(top_builddir)/modules/ShArray -I$(top_srcdir)/modules/ShArray
