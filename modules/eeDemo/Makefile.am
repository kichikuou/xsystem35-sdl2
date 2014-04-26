pkglib_LTLIBRARIES = eeDemo.la

eeDemo_la_LDFLAGS = -module -export-dynamic -avoid-version
eeDemo_la_SOURCES = eeDemo.c

AM_CFLAGS = @DEBUG_CFLAGS@

INCLUDES = -I$(top_builddir) -I$(top_srcdir) \
	-I$(top_builddir)/src -I$(top_srcdir)/src \
	-I$(top_builddir)/libltdl -I$(top_srcdir)/libltdl \
	-I$(top_builddir)/modules/eeDemo -I$(top_srcdir)/modules/eeDemo
