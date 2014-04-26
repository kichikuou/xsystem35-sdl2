pkglib_LTLIBRARIES = Confirm.la

Confirm_la_LDFLAGS = -module -export-dynamic -avoid-version
Confirm_la_SOURCES = Confirm.c
Confirm_la_LIBADD  = 

AM_CFLAGS = @DEBUG_CFLAGS@

INCLUDES = -I$(top_builddir) -I$(top_srcdir) \
	-I$(top_builddir)/src -I$(top_srcdir)/src \
	-I$(top_builddir)/modules/lib -I$(top_srcdir)/modules/lib \
	-I$(top_builddir)/libltdl -I$(top_srcdir)/libltdl \
	-I$(top_builddir)/modules/Confirm -I$(top_srcdir)/modules/Confirm
