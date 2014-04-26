pkglib_LTLIBRARIES = eDemo.la

eDemo_la_LDFLAGS = -module -export-dynamic -avoid-version
eDemo_la_SOURCES = eDemo.c

AM_CFLAGS = @DEBUG_CFLAGS@

INCLUDES = -I$(top_builddir) -I$(top_srcdir) \
	-I$(top_builddir)/src -I$(top_srcdir)/src \
	-I$(top_builddir)/libltdl -I$(top_srcdir)/libltdl \
	-I$(top_builddir)/modules/eDemo -I$(top_srcdir)/modules/eDemo

