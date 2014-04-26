pkglib_LTLIBRARIES = nDEMOE.la

nDEMOE_la_LDFLAGS = -module -export-dynamic -avoid-version
nDEMOE_la_SOURCES = nDEMOE.c

AM_CFLAGS = @DEBUG_CFLAGS@

INCLUDES = -I$(top_builddir) -I$(top_srcdir) \
	-I$(top_builddir)/src -I$(top_srcdir)/src \
	-I$(top_builddir)/libltdl -I$(top_srcdir)/libltdl \
	-I$(top_builddir)/modules/nDEMOE -I$(top_srcdir)/modules/nDEMOE
