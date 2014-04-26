pkglib_LTLIBRARIES = Math.la

Math_la_LDFLAGS = -module -export-dynamic -avoid-version
Math_la_SOURCES = Math.c

AM_CFLAGS = @DEBUG_CFLAGS@

INCLUDES = -I$(top_builddir) -I$(top_srcdir) \
	-I$(top_builddir)/src -I$(top_srcdir)/src \
	-I$(top_builddir)/libltdl -I$(top_srcdir)/libltdl \
	-I$(top_builddir)/modules/Math -I$(top_srcdir)/modules/Math
