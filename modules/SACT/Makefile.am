pkglib_LTLIBRARIES = SACT.la

SACT_la_LDFLAGS = -module -export-dynamic -avoid-version
SACT_la_LIBADD  = ../lib/libgraph.la ../lib/libdrawtext.la @GLIB_LIBS@
SACT_la_SOURCES = \
	SACT.c \
	sactcg.c \
	sactsound.c \
	sactbgm.c \
	sacttimer.c \
	sactstring.c \
	sactcrypto.c \
	sactchart.c \
	sactamask.c \
	sactlog.c \
	sprite.c \
	sprite_event.c \
	sprite_tevent.c \
	sprite_move.c \
	sprite_anime.c \
	sprite_get.c \
	sprite_put.c \
	sprite_switch.c \
	sprite_sel.c \
	sprite_msg.c \
	sprite_draw.c \
	sprite_update.c \
	sprite_eupdate.c \
	sprite_quake.c \
	sprite_keywait.c \
	screen_quake.c \
	sprite_xmenu.c


AM_CFLAGS = @DEBUG_CFLAGS@ @GLIB_CFLAGS@

INCLUDES = -I$(top_builddir) -I$(top_srcdir) \
	-I$(top_builddir)/src -I$(top_srcdir)/src \
	-I$(top_builddir)/libltdl -I$(top_srcdir)/libltdl \
	-I$(top_builddir)/modules/lib -I$(top_srcdir)/modules/lib \
	-I$(top_builddir)/modules/SACT -I$(top_srcdir)/modules/SACT
