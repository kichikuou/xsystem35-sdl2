pkglib_LTLIBRARIES = oujimisc.la

oujimisc_la_LDFLAGS = -module -export-dynamic -avoid-version
oujimisc_la_SOURCES = oujimisc.c

AM_CFLAGS = @DEBUG_CFLAGS@

INCLUDES = -I$(top_builddir) -I$(top_srcdir) \
	-I$(top_builddir)/src -I$(top_srcdir)/src \
	-I$(top_builddir)/libltdl -I$(top_srcdir)/libltdl \
	-I$(top_builddir)/modules/oujimisc -I$(top_srcdir)/modules/oujimisc
