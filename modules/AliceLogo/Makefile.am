pkglib_LTLIBRARIES = AliceLogo.la

AliceLogo_la_LDFLAGS = -module -export-dynamic -avoid-version
AliceLogo_la_SOURCES = AliceLogo.c
AliceLogo_la_LIBADD  = ../lib/libgraph.la ../lib/libalk.la

AM_CFLAGS = @DEBUG_CFLAGS@

INCLUDES = -I$(top_builddir) -I$(top_srcdir) \
	-I$(top_builddir)/src -I$(top_srcdir)/src \
	-I$(top_builddir)/modules/lib -I$(top_srcdir)/modules/lib \
	-I$(top_builddir)/libltdl -I$(top_srcdir)/libltdl \
	-I$(top_builddir)/modules/AliceLogo -I$(top_srcdir)/modules/AliceLogo
