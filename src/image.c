/*
 * image.c  image操作
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
/* $Id: image.c,v 1.40 2003/08/30 21:29:16 chikama Exp $ */

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "portab.h"
#include "system.h"
#include "image.h"
#include "cg.h"
#include "config.h"
#include "nact.h"
#include "graphicsdevice.h"
#include "alpha_plane.h"
#include "ags.h"

/* private variables */
static Pallet xpal[256];   /* pal & pixel 値 */

/* fader */
static int fadeX[16] = {0,2,2,0,1,3,3,1,1,3,3,1,0,2,2,0};
static int fadeY[16] = {0,2,0,2,1,3,1,3,0,2,0,2,1,3,1,3};

/* private methods */
static void trans_index2pixels(int depth, int lv, cgdata *cg);
static int  trans_index2pixel(int depth, int i);
static void image_drawLine8(agsurface_t *dib, int x0, int y0, int x1, int y1, int col);
static void image_drawRectangle8(agsurface_t *dib, int x, int y, int w, int h, int col);
static void image_fillRectangle8(agsurface_t *dib, int x, int y, int w, int h, int col);
static void image_copyArea8(agsurface_t *dib, int sx, int sy, int w, int h, int dx, int dy);
static void image_getGlyphImage8to8(agsurface_t *dib, agsurface_t *glyph, int dx, int dy, int col);
static void image_copyAreaSP8(agsurface_t *dib, BYTE *sdata, BYTE *ddata, int width, int height, int sp);

#include "image15.c"
#include "image16.c"
#include "image24.c"
#include "image24p.c"


static void (*draw_image24_from_data)(agsurface_t *, cgdata *, int, int, int, int);
static void (*draw_sprite16_from_data)(agsurface_t *, cgdata *, int, int, int, int);
static void (*draw_image16_from_data)(agsurface_t *, cgdata *, int, int, int, int);
static void (*draw_line)(agsurface_t*, int, int, int, int, int); 
static void (*draw_rectangle)(agsurface_t*, int, int, int, int, int);
static void (*fill_rectangle)(agsurface_t*, int, int, int, int, int);
static void (*copy_area)(agsurface_t*, int, int, int, int, int, int);
static void (*copy_area_ablend_sda)(agsurface_t *, BYTE *, BYTE *, int, int, BYTE *, int);
static void (*copy_area_ablend_sd)(agsurface_t *, BYTE *, BYTE *, int, int, int);
static void (*copy_area_alevel)(agsurface_t *, BYTE *, BYTE *, int, int, int);
static void (*copy_area_wlevel)(agsurface_t *, BYTE *, BYTE *, int, int, int);
static void (*copy_from_alpha)(agsurface_t *, BYTE *, BYTE *, int, int, ALPHA_DIB_COPY_TYPE);
static void (*copy_to_alpha)(agsurface_t *, BYTE *, BYTE *, int, int, ALPHA_DIB_COPY_TYPE);
static void (*wrap_color)(agsurface_t *, BYTE *, int, int, int, int); 
static void (*copy_area_sprite)(agsurface_t *, BYTE *, BYTE *, int, int, int);
static void (*draw_antialiased_pattern)(agsurface_t *, agsurface_t *, int, int, int, int, int);

/*
 * dib の depth に応じた関数の設定
 *   depth: dib depth
*/
void image_setdepth(int depth) {
	switch(depth) {
	case 8:
		draw_line = image_drawLine8;
		draw_rectangle = image_drawRectangle8;
		fill_rectangle = image_fillRectangle8;
		copy_area = image_copyArea8;
		copy_area_sprite = image_copyAreaSP8;
		break;
	case 15:
		// draw_image24_from_data = drawImage24_fromData15;
		draw_sprite16_from_data = drawSprite16_fromData15;
		draw_image16_from_data = drawImage16_fromData15;
		draw_line = image_drawLine16;
		draw_rectangle = image_drawRectangle16;
		fill_rectangle = image_fillRectangle16;
		copy_area = image_copyArea16;
		copy_area_ablend_sda = image_copyAreaSP16_shadow15;
		copy_area_ablend_sd = image_copyAreaSP16_alphaBlend15;
		copy_area_alevel = image_copyAreaSP16_alphaLevel15;
		copy_area_wlevel = image_copyAreaSP16_whiteLevel15;
		copy_from_alpha = image_copy_from_alpha15;
		copy_to_alpha = image_copy_to_alpha15;
		wrap_color = image_wrapColor15;
		copy_area_sprite = image_copyAreaSP16_transparent16;
		draw_antialiased_pattern = image_draw_antialiased_pattern15;
		break;
	case 16:
		draw_image24_from_data = drawImage24_fromData16;
		draw_sprite16_from_data = drawSprite16_fromData16;
		draw_image16_from_data = drawImage16_fromData16;
		draw_line = image_drawLine16;
		draw_rectangle = image_drawRectangle16;
		fill_rectangle = image_fillRectangle16;
		copy_area = image_copyArea16;
		copy_area_ablend_sda = image_copyAreaSP16_shadow16;
		copy_area_ablend_sd = image_copyAreaSP16_alphaBlend16;
		copy_area_alevel = image_copyAreaSP16_alphaLevel16;
		copy_area_wlevel = image_copyAreaSP16_whiteLevel16;
		copy_from_alpha = image_copy_from_alpha16;
		copy_to_alpha = image_copy_to_alpha16;
		wrap_color = image_wrapColor16;
		copy_area_sprite = image_copyAreaSP16_transparent16;
		draw_antialiased_pattern = image_draw_antialiased_pattern16;
		break;
	case 24:
	case 32:
		//draw_image24_from_data = drawImage24_fromData32;
		draw_sprite16_from_data = drawSprite16_fromData24;
		draw_image16_from_data = drawImage16_fromData24;
		draw_line = image_drawLine24;
		draw_rectangle = image_drawRectangle24;
		fill_rectangle = image_fillRectangle24;
		copy_area = image_copyArea24;
		copy_area_ablend_sda = image_copyAreaSP16_shadow24;
		copy_area_ablend_sd = image_copyAreaSP16_alphaBlend24;
		copy_area_alevel = image_copyAreaSP16_alphaLevel24;
		copy_area_wlevel = image_copyAreaSP16_whiteLevel24;
		copy_from_alpha = image_copy_from_alpha24;
		copy_to_alpha = image_copy_to_alpha24;
		wrap_color = image_wrapColor24;
		copy_area_sprite = image_copyAreaSP16_transparent24;
		draw_antialiased_pattern = image_draw_antialiased_pattern24;
		break;
	default:
		break;
	}
}


static void trans_index2pixels(int depth, int lv, cgdata *cg) {
	Pallet256 *pal;
	int i, i_st = 0, i_ed = 256, r, g, b;
	
	if (cg == NULL) {
		pal = nact->sys_pal;
	} else {
		pal = cg->pal;
		if (cg->type == ALCG_VSP) {
			i_st = (cg->vsp_bank << 4);
			i_ed = i_st + 16;
		}
	}
	switch(depth) {
	case 8:
		for (i = i_st; i < i_ed; i++) {
			xpal[i].r = pal->red  [i];
			xpal[i].g = pal->green[i];
			xpal[i].b = pal->blue [i];
			xpal[i].pixel = i;
		}
		break;
	case 15:
		if (lv == 255) {
			for (i = i_st; i < i_ed; i++) {
				xpal[i].pixel = PIX15(pal->red[i], pal->green[i], pal->blue[i]);
			}
		} else if (lv < 255) {
			for (i = i_st; i < i_ed; i++) {
				xpal[i].pixel = PIX15((pal->red  [i]*lv)>>8,
						      (pal->green[i]*lv)>>8,
						      (pal->blue [i]*lv)>>8);
			}
		} else {
			lv -= 255;
			for (i = i_st; i < i_ed; i++) {
				r = pal->red[i];
				g = pal->green[i];
				b = pal->blue[i];
				xpal[i].pixel = PIX15((((255-r)*lv)>>8)+r,
						      (((255-g)*lv)>>8)+g,
						      (((255-b)*lv)>>8)+b);
			}
		}
		break;
	case 16:
		if (lv == 255) {
			for (i = i_st; i < i_ed; i++) {
				xpal[i].pixel = PIX16(pal->red[i], pal->green[i], pal->blue[i]);
			}
		} else if (lv < 255) {
			for (i = i_st; i < i_ed; i++) {
				xpal[i].pixel = PIX16((pal->red  [i]*lv)>>8,
						      (pal->green[i]*lv)>>8,
						      (pal->blue [i]*lv)>>8);
			}
		} else {
			lv -= 255;
			for (i = i_st; i < i_ed; i++) {
				r = pal->red[i];
				g = pal->green[i];
				b = pal->blue[i];
				xpal[i].pixel = PIX16((((255-r)*lv)>>8)+r,
						      (((255-g)*lv)>>8)+g,
						      (((255-b)*lv)>>8)+b);
			}
		}
		break;
	case 24:
	case 32:
		if (lv == 255) {
			for (i = i_st; i < i_ed; i++) {
				xpal[i].pixel = PIX24(pal->red[i], pal->green[i], pal->blue[i]);
			} 
		} else if (lv < 255) {
			for (i = i_st; i < i_ed; i++) {
				xpal[i].pixel = PIX24((pal->red   [i]*lv)>>8,
						      (pal->green [i]*lv)>>8,
						      (pal->blue  [i]*lv)>>8);
			} 
		} else {
			lv -= 255;
			for (i = i_st; i < i_ed; i++) {
				r = pal->red[i];
				g = pal->green[i];
				b = pal->blue[i];
				xpal[i].pixel = PIX24((((255-r)*lv)>>8)+r,
						      (((255-g)*lv)>>8)+g,
						      (((255-b)*lv)>>8)+b);
			}
		}
		break;
	default:
		WARNING("Unknown depth\n");
		break;
	}
	nact->sys_pal_changed = FALSE;
}

static int trans_index2pixel(int depth, int i) {
	Pallet256 *pal = nact->sys_pal;
	
	switch(depth) {
	case 8:
		return i;
	case 15:
		return PIX15(pal->red[i], pal->green[i], pal->blue[i]);
	case 16:
		return PIX16(pal->red[i], pal->green[i], pal->blue[i]);
	case 24:
	case 32:
		return PIX24(pal->red[i], pal->green[i], pal->blue[i]);
	default:
		WARNING("Unknown depth\n");
		return i;
	}
}

/* 16bitCGの ALPHALEVELを指定 */
static WORD *changeImageAlphaLevel(cgdata *cg) {
	WORD *new_pic = g_new(WORD, cg->width * cg->height), *new_pic_;
	WORD *pic = (WORD *)cg->pic;
	int   pixels = cg->width * cg->height;
	
	new_pic_ = new_pic;

	while (pixels--) {
		*new_pic = RGB_ALPHALEVEL16(*pic, cg->alphalevel);
		new_pic++; pic++;
	}
	return new_pic_;
}


/* 色の拡大 */
void image_expandPixel(agsurface_t *img_src, agsurface_t *img_dst, int lv) {
	static int prelv = 0;
	if (img_src->depth != 8) return;
	
	if (nact->sys_pal_changed || prelv != lv) trans_index2pixels(img_dst->depth, lv, NULL);
	
	switch(img_dst->bytes_per_pixel) {
	case 2:
		expandPixel8to16(img_src, img_dst);
		break;
	case 3:
		expandPixel8to24p(img_src, img_dst);
		break;
	case 4:
		expandPixel8to24(img_src, img_dst);
		break;
	default:
		break;
	}
	prelv = lv;
}

/*
   fade out for 16/24/32
*/
void image_fadeOut(agsurface_t *img, int lv, int col) {
	switch(img->bytes_per_pixel) {
	case 2:
		fadeOut16(img, lv, col); break;
	case 3:
		fadeOut24p(img, lv, col); break;
	case 4:
		fadeOut24(img, lv, col); break;
	default:
		break;
	}
}

/*
   fade in for 16/24/32
*/
void image_fadeIn(agsurface_t *src, agsurface_t *dst, int lv) {
	switch(dst->bytes_per_pixel) {
	case 2:
		fadeIn16(src, dst, lv); break;
	case 3:
		fadeIn24p(src, dst, lv); break;
	case 4:
		fadeIn24(src, dst, lv); break;
	default:
		break;
	}
}

/*
 * 拡大・縮小コピー
 */
void image_scaledCopyArea(agsurface_t *src, agsurface_t *dst, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, int mirror) {
	float    a1, a2, xd, yd;
	int      *row, *col;
	int      x, y;
	BYTE    *sdata, *ddata;
	agsurface_t *srccpy = NULL;
	
	/* src が NULL の場合,領域が重なる場合を考えて copy を作る */
	if (src == NULL) {
		srccpy = image_saveRegion(dst, sx, sy, sw, sh);
		src = srccpy;
		sx = sy = 0;
	}
	
	sdata = GETOFFSET_PIXEL(src, sx, sy);
	ddata = GETOFFSET_PIXEL(dst, dx, dy);
	
	a1  = (float)sw / (float)dw;
	a2  = (float)sh / (float)dh;
	// src width と dst width が同じときに問題があるので+1
	row = g_new0(int, dw+1);
	// 1おおきくして初期化しないと col[dw-1]とcol[dw]が同じになる
	// 可能性がある。
	col = g_new0(int, dh+1);

	if (mirror & 1) {
		/* 上下反転 added by  tajiri@wizard */
		for (yd = sh - a2, y = 0; y < dh; y++) {
			col[y] = yd; yd -= a2;
		}
	} else {
		for (yd = 0.0, y = 0; y < dh; y++) {
			col[y] = yd; yd += a2;
		}
	}
	if (mirror & 2) {
		/* 左右反転 added by  tajiri@wizard */
		for (xd = sw - a1, x = 0; x < dw; x++) {
			row[x] = xd; xd -= a1;
		}
	} else {
		for (xd = 0.0, x = 0; x < dw; x++) {
			row[x] = xd; xd += a1;
		}
	}
	
#define SCALEDCOPYAREA(type) {                                          \
	int x, y;                                                       \
	type *sl, *dl;                                                  \
	BYTE *_sl, *_dl;                                                \
	for (y = 0; y < dh; y++) {                                      \
		sl = (type *)(sdata + *(y + col) * src->bytes_per_line);\
		dl = (type *)(ddata +   y        * dst->bytes_per_line);\
		for (x = 0; x < dw; x++) {                              \
			*(dl + x) = *(sl + *(row + x));                 \
		}                                                       \
		_dl = (BYTE *)dl;                                       \
		while(*(col + y) == *(col + y + 1)) {                   \
			_sl = _dl;                                      \
			_dl += dst->bytes_per_line;                     \
			memcpy(_dl, _sl, dw * sizeof(type));            \
			y++;                                            \
		}                                                       \
	}}
	
	switch(dst->depth) {
	case 8:	
		SCALEDCOPYAREA(BYTE); break;
	case 15:
	case 16:
		SCALEDCOPYAREA(WORD); break;
	case 24:
	case 32:
		SCALEDCOPYAREA(DWORD); break;
	default:
		break;
	}
	
	g_free(row);
	g_free(col);
	
	if (srccpy != NULL) {
		image_delRegion(srccpy);
	}
}

/*
 * dibに8bitCGの描画
 */
void image_drawImage8_fromData(agsurface_t *dib, cgdata *cg, int dx, int dy, int w, int h) {
#define DRAWSPRITEFROMDATA8(type) {                              \
	int  x, y;                                               \
	BYTE *pic_src = cg->pic + cg->data_offset;               \
	BYTE *pic_dst = GETOFFSET_PIXEL(dib, dx, dy);               \
	type *yl;                                                \
	for (y = 0; y < h; y++) {                                \
		yl = (type *)(pic_dst + y * dib->bytes_per_line);\
		for (x = 0; x < w; x++) {                        \
			if (*pic_src != (BYTE)cg->spritecolor) { \
				*yl = xpal[(*pic_src)].pixel;    \
			}                                        \
			yl++; pic_src++;                         \
		}                                                \
		pic_src += (cg->width - w);                      \
	}}
#define DRAWIMAGEFROMDATA8(type) {                               \
	int   x, y;                                              \
	BYTE *pic_src = cg->pic + cg->data_offset;               \
	BYTE *pic_dst = GETOFFSET_PIXEL(dib, dx, dy);               \
	type *yl;                                                \
	for (y = 0; y <h; y++) {                                 \
		yl = (type *)(pic_dst + y * dib->bytes_per_line);\
		for (x = 0; x < w; x++) {                        \
			*yl = xpal[(*pic_src)].pixel;            \
			yl++; pic_src++;                         \
		}                                                \
		pic_src += (cg->width - w);                      \
	}}
	
	if (cg->spritecolor >= 0) {
		switch(dib->depth) {
		case 8:
		{
			int  x, y;
			BYTE *pic_src = cg->pic + cg->data_offset;
			BYTE *pic_dst = GETOFFSET_PIXEL(dib, dx, dy);
			BYTE *yl;
			for (y = 0; y < h; y++) {
				yl = (BYTE *)(pic_dst + y * dib->bytes_per_line);
				for (x = 0; x < w; x++) {
					if (*pic_src != (BYTE)cg->spritecolor) {
						*yl = (*pic_src);
					}
					yl++; pic_src++;
				}
				pic_src += (cg->width - w);
			}
			break;
		}
		case 15:
		case 16:
			if (nact->sys_pal_changed)
				trans_index2pixels(dib->depth, cg->alphalevel, cg);
			DRAWSPRITEFROMDATA8(WORD); break;
		case 24:
		case 32:
			if (nact->sys_pal_changed)
				trans_index2pixels(dib->depth, cg->alphalevel, cg);
			DRAWSPRITEFROMDATA8(DWORD); break;
		default:
			break;
		}
	} else {
		switch(dib->depth) {
		case 8:
		{
			int   y;
			BYTE* pic_src = cg->pic + cg->data_offset;
			BYTE* pic_dst = GETOFFSET_PIXEL(dib, dx, dy);
			
			for (y = 0; y < h; y++) {
				memcpy(pic_dst, pic_src, w);
				pic_src += cg->width;
				pic_dst += dib->bytes_per_line;
			}
			break;
		}
		case 15:
		case 16:
			if (nact->sys_pal_changed)
				trans_index2pixels(dib->depth, cg->alphalevel, cg);
			DRAWIMAGEFROMDATA8(WORD); break;
		case 24:
		case 32:
			if (nact->sys_pal_changed)
				trans_index2pixels(dib->depth, cg->alphalevel, cg);
			DRAWIMAGEFROMDATA8(DWORD); break;
		default:
			break;
		}
	}
}

/*
 * dibに16bitCGの描画
 */
void image_drawImage16_fromData(agsurface_t *dib, cgdata *cg, int x, int y, int w, int h) {
	WORD *pic_save = NULL;
	
	/* set alpha Level */
	if (cg->alphalevel != 255) {
		pic_save = (WORD *)cg->pic;
		cg->pic = (BYTE *)changeImageAlphaLevel(cg);
	}
	
	if (cg->spritecolor >= 0) {
		draw_sprite16_from_data(dib, cg, x, y, w, h);
	} else {
		draw_image16_from_data(dib, cg, x, y, w, h);
	}
	
	if (cg->alphalevel != 255) {
		g_free(cg->pic);
		cg->pic = (BYTE *)pic_save;
	}
}

/*
 * dib に線を描画
 */
void image_drawLine(agsurface_t *dib, int x0, int y0, int x1, int y1, int col) {
	draw_line(dib, x0, y0, x1, y1, col);
}

/*
 * dib に矩形を描画
 */
void image_drawRectangle(agsurface_t *dib, int x, int y, int w, int h, int col) {
	draw_rectangle(dib, x, y, w, h, col);
}

/*
 * dib に矩形塗りつぶしを描画
 */
void image_fillRectangle(agsurface_t *dib, int x, int y, int w, int h, int col) {
	fill_rectangle(dib, x, y, w, h, col);
}

void image_fillRectangleNeg(agsurface_t *dib, int x, int y, int w, int h, int col) {
	fill_rectangle(dib, x, y, w, h, -1 ^ trans_index2pixel(dib->depth, col));
}

/*
 * dib の指定領域をコピー
 */
void image_copyArea(agsurface_t *dib, int sx, int sy, int w, int h, int dx, int dy) {
	copy_area(dib, sx, sy, w, h, dx, dy);
}

/*
 * 16bit専用の dib の指定領域コピー alphaつき
 */
void image_copyAreaSP16_shadow(agsurface_t *dib, int sx, int sy, int w, int h, int dx, int dy, int lv) {
	BYTE    *sdata = GETOFFSET_PIXEL(dib, sx, sy);
	BYTE    *ddata = GETOFFSET_PIXEL(dib, dx, dy);
	BYTE    *adata = GETOFFSET_ALPHA(dib, sx, sy);
	
	copy_area_ablend_sda(dib, sdata, ddata, w, h, adata, lv);
}

/*
 * 16bit専用の dib の指定領域コピー
 * srcのイメージを lvの明度でコピー CE 2001
 */
void image_copyAreaSP16_alphaBlend(agsurface_t *dib, int sx, int sy, int w, int h, int dx, int dy, int lv) {
	BYTE    *sdata = GETOFFSET_PIXEL(dib, sx, sy);
	BYTE    *ddata = GETOFFSET_PIXEL(dib, dx, dy);

	copy_area_ablend_sd(dib, sdata, ddata, w, h, lv);
}

void image_copyAreaSP16_alphaLevel(agsurface_t *dib, int sx, int sy, int w, int h, int dx, int dy, int lv) {
	BYTE    *sdata = GETOFFSET_PIXEL(dib, sx, sy);
	BYTE    *ddata = GETOFFSET_PIXEL(dib, dx, dy);
	
	copy_area_alevel(dib, sdata, ddata, w, h, lv);
}

void image_copyAreaSP16_whiteLevel(agsurface_t *dib, int sx, int sy, int w, int h, int dx, int dy, int lv) {
	BYTE    *sdata = GETOFFSET_PIXEL(dib, sx, sy);
	BYTE    *ddata = GETOFFSET_PIXEL(dib, dx, dy);

	copy_area_wlevel(dib, sdata, ddata, w, h, lv);
}

void image_copy_from_alpha(agsurface_t *dib, int sx, int sy, int w, int h, int dx, int dy, ALPHA_DIB_COPY_TYPE flag) {
	BYTE *sdata = GETOFFSET_ALPHA(dib, sx, sy);
	BYTE *ddata = GETOFFSET_PIXEL(dib, dx, dy);
	
	copy_from_alpha(dib, sdata, ddata, w, h, flag);
}

void image_copy_to_alpha(agsurface_t *dib, int sx, int sy, int w, int h, int dx, int dy, ALPHA_DIB_COPY_TYPE flag) {
	BYTE *sdata = GETOFFSET_PIXEL(dib, sx, sy);
	BYTE *ddata = GETOFFSET_ALPHA(dib, dx, dy);
	
	copy_to_alpha(dib, sdata, ddata, w, h, flag);
}


/*
 * 指定範囲にパレット col を rate の割合で重ねる CK1
 */
void image_wrapColor(agsurface_t *dib, int sx, int sy, int w, int h, int col, int rate) {
	BYTE *sdata = GETOFFSET_PIXEL(dib, sx, sy);
	
	wrap_color(dib, sdata, w, h, col, rate);
}

/*
 * dib のピクセル情報を取得
 */
void image_getPixel(agsurface_t *dib, int x, int y, Pallet *cell) {
	BYTE *dst = GETOFFSET_PIXEL(dib, x, y);

	switch(dib->depth) {
	case 8:
		cell->pixel = *dst; break;
	case 15:
		cell->r     = PIXR15(*(WORD *)dst);
		cell->g     = PIXG15(*(WORD *)dst);
		cell->b     = PIXB15(*(WORD *)dst);
		break;
	case 16:
		cell->r     = PIXR16(*(WORD *)dst);
		cell->g     = PIXG16(*(WORD *)dst);
		cell->b     = PIXB16(*(WORD *)dst);
		break;
	case 24:
	case 32:
		cell->r     = PIXR24(*(DWORD *)dst);
		cell->g     = PIXG24(*(DWORD *)dst);
		cell->b     = PIXB24(*(DWORD *)dst);
		break;
	default:
		break;
	}
}

/*
 * dib から領域の切り出し
 */
agsurface_t* image_saveRegion(agsurface_t *dib, int x, int y, int w, int h) {
	agsurface_t *i = g_new(agsurface_t ,1);
	int    j;
	BYTE *ys = GETOFFSET_PIXEL(dib, x, y);
	BYTE *yd;

	// printf("save region %d,%d,%d,%d\n", x, y, w, h);
	
	i->width          = w;
	i->height         = h;
	i->bytes_per_line = w * dib->bytes_per_pixel;
	i->bytes_per_pixel = dib->bytes_per_pixel;
	i->pixel = yd = g_new(char, w * h * dib->bytes_per_pixel);
	
	for (j = 0; j < h; j++) {
		memcpy(yd, ys, w * dib->bytes_per_pixel);
		yd += (w * dib->bytes_per_pixel);
		ys += dib->bytes_per_line;
	}
	return i;
}

/*
 * dib にセーブした領域を回復
 */
void image_putRegion(agsurface_t *dib, agsurface_t *dst, int x, int y) {
	BYTE *yd = GETOFFSET_PIXEL(dib, x, y);
	BYTE *ys = GETOFFSET_PIXEL(dst, 0, 0);
	int   i;
	int width, height;
	
	width  = dst->width;
	height = dst->height;

	if (x + width  > dib->width)  width  = dib->width - x;
	if (y + height > dib->height) height = dib->height - y;

	// printf("dibinfo %d, %d\n", dib->width, dib->height);
	// printf("reginfo %d, %d\n", dst->width, dst->height);
	// printf("width,height %d, %d\n", width, height);
	
	for (i = 0; i < height; i++) {
		memcpy(yd, ys, width * dst->bytes_per_pixel);
		ys += (dst->width * dst->bytes_per_pixel);
		yd += dib->bytes_per_line;
	}
}

/*
 * dib に dstを描画後、後始末
 */
void image_restoreRegion(agsurface_t *dib, agsurface_t *dst, int x, int y) {
	image_putRegion(dib, dst, x, y);
	g_free(dst->pixel);
	g_free(dst);
}

/*
 * save した領域の解放
 */
void image_delRegion(agsurface_t *r) {
	g_free(r->pixel);
	g_free(r);
}


/*
 * dib にセーブした領域からコピー
 */
void image_copyRegion(agsurface_t *src, int sx, int sy, int w, int h, agsurface_t *dib, int dx, int dy) {
	int y;
	BYTE *sdata = GETOFFSET_PIXEL(src, sx, sy);
	BYTE *ddata = GETOFFSET_PIXEL(dib, dx, dy);
	
	for (y = 0; y < h; y++) {
		memcpy(ddata, sdata, w * dib->bytes_per_pixel);
		sdata += src->bytes_per_line;
		ddata += dib->bytes_per_line;
	}
}
						       

/*
 * dibに gpyphイメージを描画
 */
void image_getGlyphImage(agsurface_t *dib, agsurface_t *glyph, int dx, int dy, int col) {
	switch(glyph->bytes_per_pixel) {
	case 1:
		image_getGlyphImage8to8(dib, glyph, dx, dy, col); break;
	case 2:
		image_getGlyphImage16to16(dib, glyph, dx, dy, col); break;
	case 3:
		image_getGlyphImage24pto24(dib, glyph, dx, dy, col); break;
	case 4:
		image_getGlyphImage24to24(dib, glyph, dx, dy, col); break;
	default:
		break;
	}
}

/* 
 *  img    : 8bit DIB の先頭
 *  glyph  : 文字の書かれているイメージ (大きさ込) depth 15/16 
 *  dx, dy : 表示位置
 *  col    : 色
 */
static void image_getGlyphImage16to8(agsurface_t *dib, agsurface_t *glyph, int dx, int dy, int col) {
	int   x, y;
	BYTE *dst = GETOFFSET_PIXEL(dib, dx, dy);
	BYTE *src = GETOFFSET_PIXEL(glyph, 0, 0);
	BYTE *yd;
	WORD *ys;
	
	for (y = 0; y < glyph->height; y++) {
		ys = (WORD *)(src + y * glyph->bytes_per_line);
		yd = dst + y * dib->bytes_per_line;
		for (x = 0; x < glyph->width; x++) {
			if (*ys != 0) {
				*yd = col;
			}
			ys++; yd++;
		}
	}
}

static void image_getGlyphImage24to8(agsurface_t *dib, agsurface_t *glyph, int dx, int dy, int col) {
	int   x, y;
	BYTE *dst = GETOFFSET_PIXEL(dib, dx, dy);
	BYTE *src = GETOFFSET_PIXEL(glyph, 0, 0);
	BYTE *yd;
	DWORD *ys;
	
	for (y = 0; y < glyph->height; y++) {
		ys = (DWORD *)(src + y * glyph->bytes_per_line);
		yd = dst + y * dib->bytes_per_line;
		for (x = 0; x < glyph->width; x++) {
			if (*ys == 1) {
				*yd = col;
			}
			ys++; yd++;
		}
	}
}

static void image_getGlyphImage24pto8(agsurface_t *dib, agsurface_t *glyph, int dx, int dy, int col) {
	int   x, y;
	BYTE *dst = GETOFFSET_PIXEL(dib, dx, dy);
	BYTE *src = GETOFFSET_PIXEL(glyph, 0, 0);
	BYTE *yd;
	BYTE *ys;
	
	for (y = 0; y < glyph->height; y++) {
		ys = (BYTE *)(src + y * glyph->bytes_per_line);
		yd = dst + y * dib->bytes_per_line;
		for (x = 0; x < glyph->width; x++) {
			if (*ys == 1) {
				*yd = col;
			}
			ys+=3; yd++;
		}
	}
}

void image_getGlyphImageNto8(agsurface_t *dib, agsurface_t *glyph, int dx, int dy, int col) {
	switch(glyph->bytes_per_pixel) {
	case 2:
		image_getGlyphImage16to8(dib, glyph, dx, dy, col); break;
	case 3:
		image_getGlyphImage24pto8(dib, glyph, dx, dy, col); break;
	case 4:
		image_getGlyphImage24to8(dib, glyph, dx, dy, col); break;
	}
}


/*
 * dib に指定のパレット sp を抜いてコピー
 */
void image_copyAreaSP(agsurface_t *dib, int sx, int sy, int w, int h, int dx, int dy, int col) {
	BYTE    *sdata = GETOFFSET_PIXEL(dib, sx, sy);
	BYTE    *ddata = GETOFFSET_PIXEL(dib, dx, dy);

	copy_area_sprite(dib, sdata, ddata, w, h, col);
}

int image_index2pixel(int depth, int pal) {
	return trans_index2pixel(depth, pal);
}

void image_get_glyph(agsurface_t *gls, agsurface_t *gld) {
	BYTE *dst = GETOFFSET_PIXEL(gld, 0, 0);
	int x, y;
	int dw = gls->width, dh = gls->height;
	
	switch(gls->bytes_per_pixel) {
	case 1:
	{
		BYTE *src = GETOFFSET_PIXEL(gls, 0, 0);
		for (y = 0; y < dh; y++) {
			memcpy(dst, src, dw);
			src += gls->bytes_per_line;
			dst += gld->bytes_per_line;
		}
		break;
	}
	case 2:
	{
		WORD *src;
		for (y = 0; y < dh; y++) {
			src = (WORD *)(gls->pixel + y * gls->bytes_per_line);
			dst = (BYTE *)(gld->pixel + y * gld->bytes_per_line);
			for (x = 0; x < dw; x++) {
				*dst = (*src) ? 1 : 0;
				src++; dst++;
			}
		}
		break;
	}
	case 3:
	{
		BYTE *src;
		for (y = 0; y < dh; y++) {
			src = (BYTE *)(gls->pixel + y * gls->bytes_per_line);
			dst = (BYTE *)(gld->pixel + y * gld->bytes_per_line);
			for (x = 0; x < dw; x++) {
				*dst = (*src) ? 1 : 0;
				src+=3; dst++;
			}
		}
		break;
	}
	case 4:
	{
		DWORD *src;
		for (y = 0; y < dh; y++) {
			src = (DWORD *)(gls->pixel + y * gls->bytes_per_line);
			dst = (BYTE  *)(gld->pixel + y * gld->bytes_per_line);
			for (x = 0; x < dw; x++) {
				*dst = (*src) ? 1 : 0;
				src++; dst++;
			}
		}
		break;
	}
	}
}

void image_draw_antialiased_pattern(agsurface_t *dib, agsurface_t *pattern, int dx, int dy, int dw, int dh, int col) {
	draw_antialiased_pattern(dib, pattern, dx, dy, dw, dh, col);
}

void image_draw_pattern(agsurface_t *dib, agsurface_t *pattern, int dx, int dy, int dw, int dh, int col) {
#define DRAWPATTERN(type) {                                         \
	int   x, y;                                                 \
	BYTE  *dst = GETOFFSET_PIXEL(dib, dx, dy);                  \
	BYTE  *src = (BYTE *)pattern->pixel;                        \
	type *yd;                                                   \
	BYTE  *ys;                                                  \
	for (y = 0; y < dh; y++) {                                  \
		ys = (BYTE *)(src + y * pattern->bytes_per_line);   \
		yd = (type *)(dst + y * dib->bytes_per_line);       \
		for (x = 0; x < dw; x++) {                          \
			if (*ys) {                                  \
				*yd = col;                          \
			}                                           \
			ys++; yd++;                                 \
		}                                                   \
	}}
	
	switch(dib->depth) {
	case 8:
		DRAWPATTERN(BYTE); break;
	case 15:
	case 16:
		DRAWPATTERN(WORD); break;
	case 24:
	case 32:
		DRAWPATTERN(DWORD); break;
	}
}

/* モザイク */

void image_Mosaic(agsurface_t *dib, int sx, int sy, int w, int h, int dx, int dy, int slice) {
#define m_mozaic(type) {                                                      \
	type *p_ss = (type *)GETOFFSET_PIXEL(dib, sx, sy);                       \
	type *p_src;                                                          \
	int l = dib->bytes_per_line / dib->bytes_per_pixel * slice;           \
	for (y = 0; y < h; y += slice) {                                      \
		p_src = p_ss;                                                 \
		if ((y + slice) > h ) r.height = h - y;                       \
		r.width = slice;                                              \
		for (x = 0; x < w; x += slice) {                              \
			cl = *p_src;                                          \
			r.x = dx + x;                                         \
			r.y = dy + y;                                         \
			if ((r.x + slice) > w) r.width = w - x;               \
			fill_rectangle(dib, r.x, r.y, r.width, r.height, cl); \
			p_src += slice;                                       \
		}                                                             \
		p_ss += l;                                                    \
	}}
	
	int cl;
	int x,y;
	MyRectangle r;
	
	r.width=slice;
	r.height=slice;
	
	switch(dib->depth) {
	case 8:
		m_mozaic(BYTE);
		break;
	case 16:
		m_mozaic(WORD);
		break;
	case 24:
	case 32:
		m_mozaic(DWORD);
		break;
	}
}

/******************************************************************************/
/* private methods  image操作 8bpp                                            */
/******************************************************************************/

static void image_drawLine8(agsurface_t *dib, int x0, int y0, int x1, int y1, int col) {
	int dx = abs(x0 - x1), dy = abs(y0 - y1);
	BYTE *p;
	
	if (dx == 0) {
		int i = min(y0, y1);
		p = GETOFFSET_PIXEL(dib, x0, i);
		
		for (i = 0; i < dy; i++) {
			*p = col;
			p += dib->bytes_per_line;
		}
		
	} else if (dy == 0) {
		int i = min(x0, x1);
		p = GETOFFSET_PIXEL(dib, i, y0);
		memset(p, col, dx);
		
	} else if (dx == dy) {
		int i;
		if (x0 < x1) {
			p = GETOFFSET_PIXEL(dib, x0, y0);
			if (y0 < y1) {
				dy =  dib->bytes_per_line + dib->bytes_per_pixel;
			} else {
				dy = -dib->bytes_per_line + dib->bytes_per_pixel;
			}
		} else {
			p = GETOFFSET_PIXEL(dib, x1, y1);
			if (y0 < y1) {
				dy = -dib->bytes_per_line + dib->bytes_per_pixel;
			} else {
				dy =  dib->bytes_per_line + dib->bytes_per_pixel;
			}
		}
		for (i = 0; i < dx; i++) {
			*p = col;
			p += dy;
		}
	} else {
		int i, d1, d2, ds, dd, imax;

		if (dx < dy) {
			d1 = dib->bytes_per_line;
			if (y0 > y1) {
				p  = GETOFFSET_PIXEL(dib, x1, y1);
				d2 = dib->bytes_per_pixel * (x0 < x1 ? -1 : 1);
			} else {
				p  = GETOFFSET_PIXEL(dib, x0, y0);
				d2 = dib->bytes_per_pixel * (x0 < x1 ? 1 : -1);
			}
			ds   = dx;
			imax = dy;
		} else {
			d1 = dib->bytes_per_pixel;
			if (x0 > x1) {
				p  = GETOFFSET_PIXEL(dib, x1, y1);
				d2 = dib->bytes_per_line * (y0 < y1 ? -1 : 1);
			} else {
				p  = GETOFFSET_PIXEL(dib, x0, y0);
				d2 = dib->bytes_per_line * (y0 < y1 ? 1 : -1);
			}
			ds   = dy;
			imax = dx;
		}
		dd = 0;
		for (i = 0; i < imax; i++) {
			*p  = col;
			p  += d1;
			dd += ds;
			if (dd > imax) {
				p += d2;
				dd -= imax;
			}
		}
	}
}

static void image_drawRectangle8(agsurface_t *dib, int x, int y, int w, int h, int col) {
	BYTE *dst = GETOFFSET_PIXEL(dib, x, y);
	int i;

	/* top */
	memset(dst, col, w);

	/* side */
	h-=2;
	for (i = 0; i < h; i++) {
		dst += dib->bytes_per_line;
		*dst = col;
		*(dst + w - 1) = col;
	}
	
	/* bottom */
	dst += dib->bytes_per_line;
	memset(dst, col, w);
}

static void image_fillRectangle8(agsurface_t *dib, int x, int y, int w, int h, int col) {
	BYTE *dst = GETOFFSET_PIXEL(dib, x, y);
	int i;
	
	for (i = 0; i < h; i++) {
		memset(dst, col, w);
		dst += dib->bytes_per_line;
	}
}

static void image_copyArea8(agsurface_t *dib, int sx, int sy, int w, int h, int dx, int dy) {
	BYTE *src = GETOFFSET_PIXEL(dib, sx, sy);
	BYTE *dst = GETOFFSET_PIXEL(dib, dx, dy);
	
	if (sy <= dy && dy < (sy + h)) {
		src += (h-1) * dib->bytes_per_line;
		dst += (h-1) * dib->bytes_per_line;

		while (h--) {
 			memmove(dst, src, w);
			src -= dib->bytes_per_line;
			dst -= dib->bytes_per_line;
		}
	} else {
	        while(h--) {
			memmove(dst, src, w);
			src += dib->bytes_per_line;
			dst += dib->bytes_per_line;
		}
	}
}			

static void image_getGlyphImage8to8(agsurface_t *dib, agsurface_t *glyph, int dx, int dy, int col) {
	int   x, y;
	BYTE *dst = GETOFFSET_PIXEL(dib, dx, dy);
	BYTE *src = GETOFFSET_PIXEL(glyph, 0, 0);
	BYTE *yd;
	BYTE *ys;
	
	for (y = 0; y < glyph->height; y++) {
		ys = src + y * glyph->bytes_per_line;
		yd = dst + y * dib->bytes_per_line;
		for (x = 0; x < glyph->width; x++) {
			if (*ys != 0) {
				*yd = col;
			}
			ys++; yd++;
		}
	}
}

static void image_copyAreaSP8(agsurface_t *dib, BYTE *sdata, BYTE *ddata, int width, int height, int sp) {
	int  x, y;
	BYTE *pix_dst, *pix_src;
	
	for (y = 0; y < height; y++) {
		pix_src = (BYTE *)(sdata + y * dib->bytes_per_line);
		pix_dst = (BYTE *)(ddata + y * dib->bytes_per_line);
		for (x = 0; x < width; x++) {
			if (*pix_src != (BYTE)sp) {
				*pix_dst = *pix_src;
			}
			pix_src++;
			pix_dst++;
		}
	}
}

