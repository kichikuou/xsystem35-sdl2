/*
 * font_x11.c  access to x11 font device
 *
 * Copyright (C) 1997-1998 Masaki Chikama (Wren) <chikama@kasumi.ipl.mech.nagoya-u.ac.jp>
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
*/
/* $Id: font_x11.c,v 1.11 2003/01/31 12:58:28 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <glib.h>

#include "portab.h"
#include "system.h"
#include "font.h"
#include "ags.h"
#include "xcore.h"
#include "xcore_private.h"
#include "eucsjis.h"
#include "antialiase.h"

/* fontset の為の情報 */
typedef struct {
	int      size;
	int      type;
	XFontSet id;
} FontTable;

#define FONTTABLEMAX 256
static FontTable fonttbl[FONTTABLEMAX];
static int       fontcnt = 0;
static XFontSet  fontset;
static int       font_ascent, font_descent;

/* Font Glyph を取得するためのPixmapとGC */
static Pixmap    pix_glyph;
static GC        gc_pix_glyph;
static agsurface_t  img_glyph;

static FONT *this;

#define GLYPH_PIXMAP_WIDTH  800  /* 文字イメージを取得する為のPixmapの大きさ */
#define GLYPH_PIXMAP_HEIGHT 150

static void font_insert(int size, int type, XFontSet fontset) {
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

static void font_x11_sel_font(int type, int size) {
	XFontStruct **font_structs;
	char        **font_names;
	int         i, num_fonts;
	FontTable   *tbl;
	
	if (NULL == (tbl = font_lookup(size, type))) {
		char  name[256];
		char **miss, *def;
		int    n_miss;
		XFontSet fs;
	
		if (type > 3) type = FONT_GOTHIC;
		
		/* set size */
		g_snprintf(name, sizeof(name), this->name[type], size, size);
		
		fs = XCreateFontSet(x11_display, name, &miss, &n_miss, &def);
		if (n_miss > 0) {
			for (i = 0; i < n_miss; i++) {
				WARNING("can't load font %s\n", miss[i]);
			}
			XFreeStringList(miss);
			SYSERROR("Font Load failed\n");
		}
		font_insert(size, type, fs);
		fontset = fs;
	} else {
		fontset = tbl->id;
	}
	
	/* 最大のフォントの高さの取得 */
	num_fonts = XFontsOfFontSet(fontset, &font_structs, &font_names);
	font_ascent = font_descent = 0;
	for (i = 0; i < num_fonts; i++) {
		font_ascent  = max(font_ascent, font_structs[i]->ascent);
		font_descent = max(font_descent, font_structs[i]->descent);
	}
}

static agsurface_t *get_drawn_glyph(const char *str, int w) {
	XImage   *src;
	agsurface_t *dst;
	
	XSetForeground(x11_display, gc_pix_glyph, 0);
	XFillRectangle(x11_display, pix_glyph, gc_pix_glyph, 0, 0, GLYPH_PIXMAP_WIDTH, GLYPH_PIXMAP_HEIGHT);
	
	XSetForeground(x11_display, gc_pix_glyph, 1);
	XmbDrawString(x11_display, pix_glyph, fontset, gc_pix_glyph, 0, font_ascent, str, strlen(str));
	
	src = XGetImage(x11_display, pix_glyph, 0, 0, w, font_ascent + font_descent, AllPlanes, ZPixmap);

	dst = g_new(agsurface_t, 1);

	dst->width          = w;
	dst->height         = font_ascent + font_descent;
	dst->bytes_per_line = src->bytes_per_line;
	dst->pixel          = (BYTE *)src->data;
	dst->bytes_per_pixel = x11_workinfo->cimg.bytes_per_pixel;

	src->data = NULL;
	XDestroyImage(src);
	
	return dst;
}

static void *font_x11_get_glyph(unsigned char *str) {
	agsurface_t *dst;
	int w;
	BYTE *conv;
	
	/* convert string code from sjis to euc (or LANG) */
	conv = sjis2lang(str);
	
	w = XmbTextEscapement(fontset, conv, strlen(conv)); 
	
	if (w == 0) {
		free(conv);
		return NULL;
	}
	
	dst = get_drawn_glyph(conv, w);
	image_get_glyph(dst, &img_glyph);

	if (this->antialiase_on) {
		aa_make(img_glyph.pixel, w, dst->height, img_glyph.bytes_per_line);
	}
	
	img_glyph.width  = dst->width;
	img_glyph.height = dst->height;
	
	free(dst->pixel);
	g_free(dst);
	free(conv);
	return &img_glyph;
}

static int font_x11_draw_glyph(int x, int y, unsigned char *str, int col) {
	int w;
	BYTE *conv;
	
	/* convert string code from sjis to euc (or LANG) */
	conv = sjis2lang(str);
	
	w = XmbTextEscapement(fontset, conv, strlen(conv)); 
	
	if (w == 0) {
		free(conv);
		return 0;
	}
	
	if (!x11_dibinfo->shared) {
		/* 一度 Pixmap に書いてからイメージを取得してDIBへ */
		agsurface_t *dst = get_drawn_glyph(conv, w);
		
		if (DIB_DEPTH == dib_depth_candidate) {
			image_getGlyphImage(DIB, dst, x, y, PAL2PIC(col));
		} else {
			image_getGlyphImageNto8(DIB, dst, x, y, col);
		}
		g_free(dst);
	} else {
		Xcore_setForeground(col);
		XmbDrawString(x11_display, x11_pixmap, fontset, x11_gc_pix, x, y + font_ascent, conv, strlen(conv));
		x11_needSync = TRUE;
	}
	return w;
}

static boolean drawable() {
	return !this->antialiase_on;
}


FONT *font_x11_new() {
	FONT *f = g_new(FONT, 1);
	
	f->sel_font   = font_x11_sel_font;
	f->get_glyph  = font_x11_get_glyph;
	f->draw_glyph = font_x11_draw_glyph;
	f->self_drawable = drawable;
	f->antialiase_on = FALSE;
	
	/* Glyph取得のためのPixmapとGCとagsurface_tを用意 */
	pix_glyph = XCreatePixmap(x11_display, x11_window,
				  GLYPH_PIXMAP_WIDTH, GLYPH_PIXMAP_HEIGHT, WIN_DEPTH);
	gc_pix_glyph = XCreateGC(x11_display, pix_glyph, 0, 0);
	
	img_glyph.width  = GLYPH_PIXMAP_WIDTH;
	img_glyph.height = GLYPH_PIXMAP_HEIGHT;
	img_glyph.bytes_per_line  = GLYPH_PIXMAP_WIDTH;
	img_glyph.depth           = 8;
	img_glyph.bytes_per_pixel = 1;
	img_glyph.pixel = g_malloc(GLYPH_PIXMAP_WIDTH * GLYPH_PIXMAP_HEIGHT);
	
	this = f;

	NOTICE("FontDevice X11\n");
	
	return f;
}
