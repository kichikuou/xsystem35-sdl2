pkglib_LTLIBRARIES = ShSound.la

ShSound_la_LDFLAGS = -module -export-dynamic -avoid-version
ShSound_la_SOURCES = ShSound.c


AM_CFLAGS = @DEBUG_CFLAGS@

AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir) \
	-I$(top_builddir)/src -I$(top_srcdir)/src \
	-I$(top_builddir)/libltdl -I$(top_srcdir)/libltdl \
	-I$(top_builddir)/modules/ShSound -I$(top_srcdir)/modules/ShSound
