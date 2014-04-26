pkglib_LTLIBRARIES = MsgSkip.la MSGSKIP.la

MsgSkip_la_LDFLAGS = -module -export-dynamic -avoid-version
MsgSkip_la_SOURCES = MsgSkip.c

MSGSKIP_la_LDFLAGS = -module -export-dynamic -avoid-version
MSGSKIP_la_SOURCES = MsgSkip.c

AM_CFLAGS = @DEBUG_CFLAGS@

INCLUDES = -I$(top_builddir) -I$(top_srcdir) \
	-I$(top_builddir)/src -I$(top_srcdir)/src \
	-I$(top_builddir)/libltdl -I$(top_srcdir)/libltdl \
	-I$(top_builddir)/modules/MsgSkip -I$(top_srcdir)/modules/MsgSkip
