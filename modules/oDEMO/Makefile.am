pkglib_LTLIBRARIES = oDEMO.la

oDEMO_la_LDFLAGS = -module -export-dynamic -avoid-version
oDEMO_la_SOURCES = oDEMO.c

AM_CFLAGS = @DEBUG_CFLAGS@

INCLUDES = -I$(top_builddir) -I$(top_srcdir) \
	-I$(top_builddir)/src -I$(top_srcdir)/src \
	-I$(top_builddir)/libltdl -I$(top_srcdir)/libltdl \
	-I$(top_builddir)/modules/oDEMO -I$(top_srcdir)/modules/oDEMO
