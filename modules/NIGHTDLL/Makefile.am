pkglib_LTLIBRARIES = NIGHTDLL.la

NIGHTDLL_la_LDFLAGS = -module -export-dynamic -avoid-version
NIGHTDLL_la_LIBADD  = ../lib/libgraph.la ../lib/libdrawtext.la @GLIB_LIBS@
NIGHTDLL_la_SOURCES = \
	NIGHTDLL.c \
	nt_scenario.c \
	sactcg.c \
	sactstring.c \
	sprite.c \
	sprite_draw.c \
	sprite_update.c \
	sprite_eupdate.c \
	nt_event.c \
	nt_msg.c \
	nt_sel.c \
	nt_graph.c \
	nt_sound.c

AM_CFLAGS = @DEBUG_CFLAGS@ @GLIB_CFLAGS@

INCLUDES = -I$(top_builddir) -I$(top_srcdir) \
	-I$(top_builddir)/src -I$(top_srcdir)/src \
	-I$(top_builddir)/libltdl -I$(top_srcdir)/libltdl \
	-I$(top_builddir)/modules/lib -I$(top_srcdir)/modules/lib \
	-I$(top_builddir)/modules/NIGHTDLL -I$(top_srcdir)/modules/NIGHTDLL
