/*
 * font_ttf.c  access to ttf font device
 *
 * Copyright (C) 2000-   Fumihiko Murata <fmurata@p1.tcnet.ne.jp>
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
/* $Id: font_ttf.c,v 1.6 2002/09/18 13:16:22 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <glib.h>
#include <string.h>

#ifdef FREETYPE_HAVE_DIR
#include <freetype/freetype.h>
#include <freetype/ftxerr18.h>
#else
#include <freetype.h>
#include <ftxerr18.h>
#endif

#include "portab.h"
#include "system.h"
#include "font.h"
#include "ags.h"
#include "s2utbl.h"

typedef struct {
	int                fnn;
	int                size, ysize;
	TT_Face            face;
	TT_Face_Properties pp;
	TT_Glyph           glyph;
	TT_Instance        inst;
	TT_Matrix          mat;
	TT_Raster_Map      r;
} ttfont;

typedef struct {
	int etype;   /* font char map ... sjis:0, unicode:1 */
	TT_CharMap map;
} ttmap;

typedef struct {
	int      size;
	int      type;
	ttfont   *id;
} FontTable;

#define FONTTABLEMAX 256
static FontTable fonttbl[FONTTABLEMAX];
static int       fontcnt = 0;
static TT_Engine eng;
static ttfont   *fontset;
static ttmap     fontmaps[4];
static TT_Byte rpal_na[5]={   0,    0, 0xff, 0xff, 0xff};
static TT_Byte rpal_an[5]={0x20, 0x3f, 0xc0, 0xf0, 0xff};

#define GLYPH_PIXMAP_WIDTH  800  /* 文字イメージを取得する為のPixmapの大きさ */
#define GLYPH_PIXMAP_HEIGHT 150

static agsurface_t  img_glyph;

static FONT *this;

static void font_insert(int size, int type, ttfont *fontset) {
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

static void font_ttf_sel_font(int type, int size) {
	FontTable *tbl;


	if (NULL == (tbl = font_lookup(size, type))) {
		char *name;
		ttfont *fs;
		TT_Error err;
		int i, map_sjis = 0, map_uni = 0;
		TT_UShort platform, encoding;
		TT_Instance_Metrics imt;
		
		if (type > 3) type = FONT_GOTHIC;
		
		name = this->name[type];
		
		fs = g_new0(ttfont, 1);
		fs->fnn  = type;
		fs->size = size;
		
		err = TT_Open_Face(eng, name, &fs->face);
		NOTICE("TTF open %s size %d\n", name, size);
		if (err) {
			WARNING("%s is not found: %s\n", name, TT_ErrToString18(err));
			g_free(fs);
			return;
		}
		
		TT_Get_Face_Properties(fs->face, &fs->pp);
		fs->ysize = (fs->pp.header->yMax * size) / (fs->pp.header->yMax - fs->pp.header->yMin + 1);
		
		err = TT_New_Glyph(fs->face, &fs->glyph);
		if (err) {
			WARNING("TTfont %s couldn't create glyph container: %s\n",name, TT_ErrToString18(err));
			g_free(fs);
			return;
		}
		err = TT_New_Instance(fs->face, &fs->inst);
		err = TT_Set_Instance_Resolutions(fs->inst, 76, 76);
		err = TT_Set_Instance_CharSize(fs->inst, size*64);
		
		/* character map is sjis or unicode */
		if (fontmaps[type].etype < 0) {
			for (i = 0; i < TT_Get_CharMap_Count(fs->face); i++) {
				TT_Get_CharMap_ID(fs->face, i, &platform, &encoding);
				if (platform == TT_PLATFORM_MICROSOFT) {
					if (encoding == TT_MS_ID_SJIS) map_sjis = i + 1;
					if (encoding == TT_MS_ID_UNICODE_CS) map_uni = i + 1;
				}
			}
			if (map_sjis) {
				fontmaps[type].etype = 0;
				TT_Get_CharMap(fs->face, map_sjis -1, &fontmaps[type].map);
			} else if (map_uni) {
				fontmaps[type].etype = 1;
				TT_Get_CharMap(fs->face, map_uni -1, &fontmaps[type].map);
			}
		}
		
		TT_Get_Instance_Metrics(fs->inst, &imt);
		fs->r.rows = size;
		fs->r.width = size;
		fs->r.flow = TT_Flow_Up;
		fs->r.cols = size + 3;
		fs->r.size = fs->r.rows * fs->r.cols;
		fs->r.bitmap = g_malloc((int)fs->r.size);
		font_insert(size, type, fs);
		fontset = fs;
	} else {
		fontset = tbl->id;
	}
}

static void pixmap2comimg(BYTE *src, int x, int y, int w, int h, int src_bpl) {
	int yy;
	BYTE *dst = GETOFFSET_PIXEL(&img_glyph, x, y + h - 2); /* must be -1 XXX */
	
	for (yy = 0; yy < h; yy++) {
		memcpy(dst, src, w);
		src += src_bpl;
		dst -= img_glyph.bytes_per_line;
	}
}

static void clear_canvas(void) {
	memset(img_glyph.pixel, 0, GLYPH_PIXMAP_WIDTH * GLYPH_PIXMAP_HEIGHT);
}

static void *font_ttf_get_glyph(unsigned char *msg) {
	TT_UShort code;
	guint16 w = 0;
	int ww, hh;
	int index;
	TT_Glyph_Metrics mt;
	TT_Error err;
	int x = 0;
	
	boolean isHalfWidth = FALSE;   /* half width character ? */
	
	clear_canvas();

	if (this->antialiase_on) {
		TT_Set_Raster_Gray_Palette(eng, rpal_an);
	} else {
		TT_Set_Raster_Gray_Palette(eng, rpal_na);
	}
	
	while(*msg) {
		if (fontmaps[fontset->fnn].etype) {
			/* sjis to unicode */
			if (*msg >= 0xa0 && *msg <= 0xdf) {
				code = 0xff60 + *(msg++) - 0xa0;
				isHalfWidth = TRUE;
			} else if( *msg&0x80 ) {
				code = s2u[msg[0] - 0x80][msg[1] - 0x40];
				msg+=2;
				isHalfWidth = FALSE;
			} else {
				code = *(msg++);
				isHalfWidth = TRUE;
			}
		} else {
			if (*msg >= 0xa0 && *msg <= 0xdf) {
				code = *(msg++);
				isHalfWidth = TRUE;
			} else if (*msg & 0x80) {
				code = (msg[0] << 8) + msg[1];
				msg += 2;
				isHalfWidth = FALSE;
			} else {
				code = *(msg++);
				isHalfWidth = TRUE;
			}
		}
		
		index = TT_Char_Index(fontmaps[fontset->fnn].map, code);
		if (!(err = TT_Load_Glyph(fontset->inst, fontset->glyph, index, TTLOAD_DEFAULT)) ) {
			TT_Get_Glyph_Metrics(fontset->glyph, &mt);
			
			memset(fontset->r.bitmap, 0, fontset->r.size);
			TT_Get_Glyph_Pixmap(fontset->glyph, &fontset->r,
					    -mt.bbox.xMin, -mt.bbox.yMin);
			
			ww = (mt.bbox.xMax - mt.bbox.xMin) / 64;
			if (ww > fontset->r.cols) ww = fontset->size;
			
			hh = (mt.bbox.yMax - mt.bbox.yMin) / 64;
			if (hh > fontset->r.rows) hh = fontset->size;
			
			pixmap2comimg(fontset->r.bitmap,
				      x + mt.bbox.xMin/64, fontset->size - mt.bbox.yMax/64,
				      ww, hh, fontset->r.cols);
			
			if (isHalfWidth) {
				x += (fontset->size / 2);
				w += (fontset->size / 2);
			} else {
				x += fontset->size;
				w += fontset->size;
			}
		}
	}
	
	img_glyph.width  = x;
	img_glyph.height = fontset->size;
	
	return &img_glyph;
}

static int font_ttf_draw_glyph(int x, int y, unsigned char *str, int col) {
	return 0;
}

static boolean drawable() {
	return FALSE;
}

FONT *font_ttf_new() {
	int err, i;
	FONT *f = g_new(FONT, 1);
	
	f->sel_font   = font_ttf_sel_font;
	f->get_glyph  = font_ttf_get_glyph;
	f->draw_glyph = font_ttf_draw_glyph;
	f->self_drawable = drawable;
	f->antialiase_on = FALSE;
	
	for (i = 0; i < 4; i++) {
		fontmaps[i].etype = -1;
	}
	err = TT_Init_FreeType(&eng);
#if 0
	if (err >= 0) {
		TT_Set_Raster_Gray_Palette(eng, rpal);
	}
#endif
	
	img_glyph.width  = GLYPH_PIXMAP_WIDTH;
	img_glyph.height = GLYPH_PIXMAP_HEIGHT;
	img_glyph.bytes_per_line  = GLYPH_PIXMAP_WIDTH;
	img_glyph.depth           = 8;
	img_glyph.bytes_per_pixel = 1;
	img_glyph.pixel = g_malloc(GLYPH_PIXMAP_WIDTH * GLYPH_PIXMAP_HEIGHT);
	
	this = f;
	
	NOTICE("FontDevice freetype 1.x\n");
	
	return f;
}
