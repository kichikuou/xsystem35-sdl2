/*
 * font_freetype2.c  access to ttf font device
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
/* $Id: font_freetype2.c,v 1.12 2004/10/31 04:18:06 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <string.h>

#include <ft2build.h>
#include FT_FREETYPE_H
//#include <freetype/freetype.h>

#include "portab.h"
#include "system.h"
#include "font.h"
#include "ags.h"
#include "utfsjis.h"

extern const unsigned short *s2u[];

typedef struct {
	int      size;
	int      type;
	FT_Face  face;
} FontTable;

#define FONTTABLEMAX 256
static FontTable fonttbl[FONTTABLEMAX];
static int       fontcnt = 0;
static FT_Library eng;
static FontTable *fontset;

#define GLYPH_PIXMAP_WIDTH  800  /* 文字イメージを取得する為のPixmapの大きさ */
#define GLYPH_PIXMAP_HEIGHT 150

static agsurface_t img_glyph;

static FONT *this;

static void pixmap2comimg(BYTE *src, int x, int y, int w, int h, int src_bpl);
static void pixmapmono2comimg(BYTE *src, int x, int y, int w, int h, int src_bpl);

static boolean select_charmap(FT_Face f, int type) {
	int i;
	
	for (i = 0; i < f->num_charmaps; i++) {
		FT_CharMap map = f->charmaps[i];
		if (map->encoding == ft_encoding_unicode) {
			FT_Select_Charmap(f, ft_encoding_unicode);
			return TRUE;
		}
	}
	return FALSE;
}

static void font_insert(int size, int type, FT_Face face) {
	fonttbl[fontcnt].size = size;
	fonttbl[fontcnt].type = type;
	fonttbl[fontcnt].face = face;
	
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

static void font_ttf_sel_font(int type, int size) {
	FontTable *tbl;
	
	// check too big size 
	if (size > (GLYPH_PIXMAP_HEIGHT - 10)) {
		size = GLYPH_PIXMAP_HEIGHT - 10;
	}
	
	if (NULL == (tbl = font_lookup(size, type))) {
		char *name;
		FT_Error err;
		FT_Face face;
		
		if (type > FONTTYPEMAX) type = FONT_GOTHIC;
		
		name = this->name[type];
		
		err = FT_New_Face(eng, name, this->face[type], &face);
		if (err) {
			WARNING("face %d is not found. retry 0", this->face[type]);
			err = FT_New_Face(eng, name, 0, &face);
		}
		NOTICE("TTF open %s size %d\n", name, size);
		if (err) {
			WARNING("%s is not found\n", name);
			return;
		}
		
		if (!select_charmap(face, type))
			SYSERROR("TTfont %s has no unicode charmap\n", name);
		
		err = FT_Set_Pixel_Sizes(face, 0, size);
		if (err) {
			WARNING("Font %s contains only fixed size(%d) font.\n",name, face->size);
			return;
		}
		
		font_insert(size, type, face);
		fontset = &fonttbl[fontcnt -1];
	} else {
		fontset = tbl;
	}
}

static void pixmap2comimg(BYTE *src, int x, int y, int w, int h, int src_bpl) {
	if (x < 0) {
		w += x;
		src -= x;
		x = 0;
	}
	BYTE *dst = GETOFFSET_PIXEL(&img_glyph, x, y);
	if (w <= 0) return;
	
	for (int yy = 0; yy < h; yy++) {
		memcpy(dst, src, w);
		src += src_bpl;
		dst += img_glyph.bytes_per_line;
	}
}

static void pixmapmono2comimg(BYTE *src, int x, int y, int w, int h, int src_bpl) {
	int i, xx, yy;
	int w1,w2;
	BYTE *dst = GETOFFSET_PIXEL(&img_glyph, x, y);
	if (w <= 0) return;
	
	w1 = w / 8;
	w2 = w % 8;
	for (yy = 0; yy < h; yy++) {
		for (xx = 0; xx < w1; xx++){
			BYTE ch = src[xx];
			for (i = 0; i < 8; i++){
				*(dst + i + xx * 8) = (ch & 0x80 ? 255 : 0);
				ch <<= 1;
			}
		}
		if (w2) {
			BYTE ch = src[w1];
			for (i = 0; i < w2; i++){
				*(dst + i + w1 * 8) = (ch & 0x80 ? 255 : 0);
				ch <<= 1;
			}
		}
		src += src_bpl;
		dst += img_glyph.bytes_per_line;
	}
}

static void clear_canvas(void) {
	memset(img_glyph.pixel, 0, GLYPH_PIXMAP_WIDTH * GLYPH_PIXMAP_HEIGHT);
}

static agsurface_t *font_ttf_get_glyph(const char *str_utf8) {
	FT_GlyphSlot   slot;
	FT_UShort      code;
	FT_Error       err;
	FT_Pixel_Mode  pixelmode;
	FT_Int         loadflag;
	int x = 0;
	
	if (fontset == NULL) return &img_glyph;
	
	clear_canvas();
	
	while (*str_utf8) {
		code = utf8_next_codepoint(&str_utf8);
		
		if (this->antialiase_on) {
			loadflag = FT_LOAD_RENDER;
			// if( 埋め込みビットマップを使いたくない時)
			//	loadglag |= FT_LOAD_NO_BITMAP;
		} else {
			loadflag = (FT_LOAD_RENDER | FT_LOAD_MONOCHROME);
		}
		
		err = FT_Load_Char(fontset->face, code, loadflag);
		/* gray scaleでもモノクロでもない埋め込みビットマップは使わず、
		   アウトラインフォントを使用 */
		pixelmode = fontset->face->glyph->bitmap.pixel_mode ;
		if (!err &&
		    pixelmode != ft_pixel_mode_mono &&
		    pixelmode != ft_pixel_mode_grays){
			WARNING("Not supported type embeded bitmap font!!!");
			err = FT_Load_Char(fontset->face, code, loadflag | FT_LOAD_NO_BITMAP);
			pixelmode = fontset->face->glyph->bitmap.pixel_mode;
		}
		
		if (err) continue;
		
		slot = fontset->face->glyph;
		
		// lazy check, but needed
		if (x + fontset->size > GLYPH_PIXMAP_WIDTH) {
			break;
		}
		
		if (pixelmode == ft_pixel_mode_grays) {
			pixmap2comimg(slot->bitmap.buffer,
				      x + slot->bitmap_left,
				      max(0, fontset->size * 0.9 - slot->metrics.horiBearingY/64),
				      slot->bitmap.width,
				      slot->bitmap.rows,
				      slot->bitmap.pitch);
		} else if (pixelmode == ft_pixel_mode_mono) {
			pixmapmono2comimg(slot->bitmap.buffer,
					  x + slot->bitmap_left,
					  max(0, fontset->size * 0.9 - slot->metrics.horiBearingY/64),
					  slot->bitmap.width,
					  slot->bitmap.rows,
					  slot->bitmap.pitch);
		}
		x += slot->metrics.horiAdvance/64;
	}
	
	img_glyph.width  = x;
	img_glyph.height = fontset->size;
	
	return &img_glyph;
}

static int font_ttf_draw_glyph(int x, int y, const char *str_utf8, int col) {
	return 0;
}

static boolean drawable() {
	return FALSE;
}

FONT *font_ft2_new() {
	int err;
	FONT *f = malloc(sizeof(FONT));
	
	f->sel_font   = font_ttf_sel_font;
	f->get_glyph  = font_ttf_get_glyph;
	f->draw_glyph = font_ttf_draw_glyph;
	f->self_drawable = drawable;
	f->antialiase_on = FALSE;
	
	err = FT_Init_FreeType(&eng);
	
	img_glyph.width  = GLYPH_PIXMAP_WIDTH;
	img_glyph.height = GLYPH_PIXMAP_HEIGHT;
	img_glyph.bytes_per_line  = GLYPH_PIXMAP_WIDTH;
	img_glyph.depth           = 8;
	img_glyph.bytes_per_pixel = 1;
	img_glyph.pixel = malloc(GLYPH_PIXMAP_WIDTH * GLYPH_PIXMAP_HEIGHT);
	
	this = f;
	
	NOTICE("FontDevice freetype2\n");
	
	return f;
}
