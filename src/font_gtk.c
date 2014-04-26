/*
 * font_gtk.c  access to gtk(gdk) font device 
 *
 * Copyright (C) 2000-     Fumihiko Murata       <fmurata@p1.tcnet.ne.jp>
 *           (C) 1997-1998 Masaki Chikama (Wren) <chikama@kasumi.ipl.mech.nagoya-u.ac.jp>
 *               1998-                           <masaki-c@is.aist-nara.ac.jp>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * NOTE:
 *   sdl_core.c から移動。また、その際 sdl に依存しないようにした。
 *
*/
/* $Id: font_gtk.c,v 1.8 2003/01/25 01:34:50 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <glib.h>

#include "portab.h"
#include "system.h"
#include "font.h"
#include "ags.h"
#include "eucsjis.h"
#include "antialiase.h"
#include "image.h"

#ifndef GTK_CHECK_VERSION
#define GTK_CHECK_VERSION(major,minor,micro)    \
(GTK_MAJOR_VERSION > (major) || \
 (GTK_MAJOR_VERSION == (major) && GTK_MINOR_VERSION > (minor)) || \
 (GTK_MAJOR_VERSION == (major) && GTK_MINOR_VERSION == (minor) && \
  GTK_MICRO_VERSION >= (micro)))
#endif /* !GTK_CHECK_VERSION */

#if defined (GTK_CHECK_VERSION) && GTK_CHECK_VERSION (1,2,0)
#define GTKV12 
#else
#define GTKV10
#endif

/* fontset の為の情報 */
typedef struct {
	int      size;
	int      type;
	GdkFont  *id;
} FontTable;

#define FONTTABLEMAX 256
static FontTable fonttbl[FONTTABLEMAX];
static int       fontcnt = 0;
static GdkFont   *fontset;
static int       font_ascent, font_descent;

static GtkWidget *mainwin;
static int       gdk_depth = 0;
static agsurface_t img_glyph;

static FONT *this;

#define GLYPH_PIXMAP_WIDTH  800  /* 文字イメージを取得する為のPixmapの大きさ */
#define GLYPH_PIXMAP_HEIGHT 150

static void font_insert(int size, int type, GdkFont *fontset) {
	fonttbl[fontcnt].size = size;
	fonttbl[fontcnt].type = type;
	fonttbl[fontcnt].id   = fontset;
	
	if (fontcnt >= (FONTTABLEMAX -1)) {
		WARNING("Font table is full.\n");
	} else {
		fontcnt++;
	}
}

static FontTable *font_lookup(int size, int type) {
	int i;
	
	for (i = 0; i < fontcnt; i++) {
		if (fonttbl[i].size == size && fonttbl[i].type == type) { 
			return &fonttbl[i];
		}
	}
	return NULL;
}

static void font_gtk_sel_font(int type, int size) {
	FontTable *tbl;
	
	if (NULL == (tbl = font_lookup(size, type))) {
		char    name[256];
		
		if (type > 3) type = FONT_GOTHIC;
		
		g_snprintf(name, sizeof(name), this->name[type], size, size);
		
		fontset = gdk_fontset_load(name);
		if (fontset == NULL) {
			SYSERROR("can't load font %s\n", name);
		}
		font_insert(size, type, fontset);
	} else {
		fontset = tbl->id;
	}
	
	font_ascent  = fontset->ascent;
	font_descent = fontset->descent;
}

static void *font_gtk_get_glyph(unsigned char *str) {
	agsurface_t *dst;
	int h, w, l;
	BYTE *conv;
	
	GdkPixmap    *pix_gdk;
	GdkGC        *gc_gdk;
	GdkImage     *img_gdk;
	GdkColor     col_gdk;

	
	/* convert string code from sjis to euc (or LANG) */
	conv = sjis2lang(str);
	
	l = strlen(conv);
	w = gdk_text_width(fontset, conv, l);
	
	if (w == 0) {
		free(conv);
		return NULL;
	}
	
#ifdef GTKV12
	h = gdk_text_height(fontset, conv, l);
#else
	h = font_ascent + fontset->ascent;
#endif
	
	if (w > GLYPH_PIXMAP_WIDTH)  w = GLYPH_PIXMAP_WIDTH;
	if (h > GLYPH_PIXMAP_HEIGHT) h = GLYPH_PIXMAP_HEIGHT;
	
	pix_gdk = gdk_pixmap_new(mainwin->window, w, h, gdk_depth);
	gc_gdk  = gdk_gc_new(pix_gdk);

	/* color */
	col_gdk.pixel = 0;
	gdk_gc_set_foreground(gc_gdk, &col_gdk);
	// gdk_gc_set_background(gc_gdk, &col_gdk);
	gdk_draw_rectangle(pix_gdk, gc_gdk, TRUE, 0, 0, w, h);
	
	col_gdk.pixel = 1;
	gdk_gc_set_foreground(gc_gdk, &col_gdk);
	gdk_draw_text(pix_gdk, fontset, gc_gdk, 0, fontset->ascent, conv, l);
	
	gdk_gc_destroy(gc_gdk);
	img_gdk = gdk_image_get(&((GdkWindowPrivate *)pix_gdk)->window,
				0, 0, w, h);
	gdk_pixmap_unref(pix_gdk);

	dst = g_new(agsurface_t, 1);

	dst->width = w;
	dst->height = h;
	dst->bytes_per_pixel = (img_gdk->bpp+1)/8; /* むーん */
	dst->bytes_per_line  = img_gdk->bpl;
	dst->pixel            = img_gdk->mem;
	
	image_get_glyph(dst, &img_glyph);

	if (this->antialiase_on) {
		aa_make(img_glyph.pixel, w, dst->height, img_glyph.bytes_per_line);
	}
	
	img_glyph.width  = w;
	img_glyph.height = h;
	
	g_free(dst);
	g_free(conv);
	gdk_image_destroy(img_gdk);
	
	return &img_glyph;
}

static int font_gtk_draw_glyph(int x, int y, unsigned char *str, int col) {
	return 0;
}

static boolean drawable() {
	return FALSE;
}

FONT *font_gtk_new() {
	FONT *f = g_new(FONT, 1);
	
	f->sel_font   = font_gtk_sel_font;
	f->get_glyph  = font_gtk_get_glyph;
	f->draw_glyph = font_gtk_draw_glyph;
	f->self_drawable = drawable;
	f->antialiase_on = FALSE;

	mainwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gdk_depth = gdk_visual_get_best_depth();

	img_glyph.width  = GLYPH_PIXMAP_WIDTH;
	img_glyph.height = GLYPH_PIXMAP_HEIGHT;
	img_glyph.bytes_per_line  = GLYPH_PIXMAP_WIDTH;
	img_glyph.depth           = 8;
	img_glyph.bytes_per_pixel = 1;
	img_glyph.pixel = g_malloc(GLYPH_PIXMAP_WIDTH * GLYPH_PIXMAP_HEIGHT);
	
	this = f;
	
	NOTICE("FontDevice gtk\n");
	
	return f;
}
