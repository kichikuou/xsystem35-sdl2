pkglib_LTLIBRARIES = nDEMO.la

nDEMO_la_LDFLAGS = -module -export-dynamic -avoid-version
nDEMO_la_SOURCES = nDEMO.c

AM_CFLAGS = @DEBUG_CFLAGS@

INCLUDES = -I$(top_builddir) -I$(top_srcdir) \
	-I$(top_builddir)/src -I$(top_srcdir)/src \
	-I$(top_builddir)/libltdl -I$(top_srcdir)/libltdl \
	-I$(top_builddir)/modules/nDEMO -I$(top_srcdir)/modules/nDEMO
