noinst_LTLIBRARIES  = libgraph.la libalk.la libdrawtext.la

libgraph_la_LIBADD = @GLIB_LIBS@
libgraph_la_SOURCES = graph.c surface.c cg.c \
	graph_expandcolor_blend.c \
	graph_fillrect.c \
	graph_fillrect_amap.c \
	graph_fillrect_acolor.c \
	graph_rect.c \
	graph_copy.c \
	graph_copy_amap.c \
	graph_copy_bright.c \
	graph_copy_whiteout.c \
	graph_blend_amap.c \
	graph_blend_screen.c \
	graph_saturadd_amap.c \
	graph_draw_amap.c \
	graph_stretch.c \
	graph_cg.c \
	graph_buller.c \
	gre_blend_useamap.c \
	gre_blend.c \
	gre_blend_screen.c

libalk_la_SOURCES = alk.c 
libalk_la_LIBADD = @GLIB_LIBS@ 

libdrawtext_la_SOURCES = drawtext.c
libdrawtext_la_LIBADD = @GLIB_LIBS@

AM_CFLAGS = @DEBUG_CFLAGS@ @GLIB_CFLAGS@

INCLUDES = -I$(top_builddir) -I$(top_srcdir) \
	-I$(top_builddir)/src -I$(top_srcdir)/src \
	$(LTDLINCL) \
	-I$(top_builddir)/modules/lib -I$(top_srcdir)/modules/lib

