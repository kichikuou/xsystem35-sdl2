/*
 * xcore_image.c  image操作 for X11
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
static Palette xpal[256];   /* pal & pixel 値 */

/* private methods */
static void trans_index2pixels(int depth, int lv, cgdata *cg);

/******************************************************************************/
/* private methods  image操作 8bpp                                            */
/******************************************************************************/

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

/******************************************************************************/
/* private methods  image操作 15bpp                                           */
/******************************************************************************/

static void drawImage16_fromData15(agsurface_t *dib, cgdata *cg, int dx, int dy, int w, int h) {
	int  x, y;
	WORD *pic_src = (WORD *)(cg->pic + cg->data_offset);
	BYTE *pic_dst = GETOFFSET_PIXEL(dib, dx, dy);
	WORD  pic16;
	WORD *yl;
	
	for (y = 0; y < h; y++) {
		yl = (WORD *)(pic_dst + y * dib->bytes_per_line);
		for (x = 0; x < w; x++) {
			pic16 = *pic_src;
			*yl = PIX15(RGB_PIXR16(pic16), RGB_PIXG16(pic16), RGB_PIXB16(pic16));
			(WORD *)yl ++; pic_src++;
		}
		pic_src += (cg->width - w);
	}
}

static void drawSprite16_fromData15(agsurface_t *dib, cgdata *cg, int dx, int dy, int w, int h) {
	int  x, y;
	WORD pic16;
	WORD *pic_src   = (WORD *)(cg->pic + cg->data_offset);
	BYTE *pic_dst   = GETOFFSET_PIXEL(dib, dx, dy);
	BYTE *pic_dst_a = GETOFFSET_ALPHA(dib, dx, dy);
	WORD *yl, pic15;
	BYTE *ya;
	
	for (y = 0; y < h; y++) {
		yl = (WORD *)(pic_dst   + y * dib->bytes_per_line);
		ya = (BYTE *)(pic_dst_a + y * dib->width);
		for (x = 0; x < w; x++) {
			pic16 = *pic_src;
			pic15 = PIX15(RGB_PIXR16(pic16),RGB_PIXG16(pic16),RGB_PIXB16(pic16));
			if (*ya == 255) {
				*yl = pic15;
			} else if (*ya > 0) {
				*yl= ALPHABLEND15(pic15, *yl, *ya);
			}
			yl ++; ya++; pic_src++;
		}
		pic_src += (cg->width - w);
	}
}

static void image_wrapColor15(agsurface_t *dib, BYTE *dst, int w, int h, int col, int rate) {
	int x, y;
	WORD *yls, yld;
	
	yld = image_index2pixel(dib->depth, col);
	
	for (y = 0; y < h; y++) {
		yls = (WORD *)(dst + y * dib->bytes_per_line);
		for (x = 0; x < w; x++) {
			*yls = ALPHABLEND15(yld, *yls, rate);
			yls++;
		}
	}
}

static void image_copyAreaSP16_shadow15(agsurface_t *dib, BYTE *sdata, BYTE *ddata, int w, int h, BYTE *adata, int lv) {
	int x, y;
	WORD *yls, *yld;
	BYTE *yla;

	for (y = 0; y < h; y++) {
		yls = (WORD *)(sdata + y * dib->bytes_per_line);
		yld = (WORD *)(ddata + y * dib->bytes_per_line);
		yla = (BYTE *)(adata + y * dib->width);
		for (x = 0; x < w; x++) {
			*yld = ALPHABLEND15(*yls, *yld, (*yla * lv) / 255);
			yls++; yld++; yla++;
		}
	}
}

static void image_copyAreaSP16_alphaBlend15(agsurface_t *dib, BYTE *sdata, BYTE *ddata, int w, int h, int lv) {
	int x, y;
	WORD *yls, *yld;
	
	for (y = 0; y < h; y++) {
		yls = (WORD *)(sdata + y * dib->bytes_per_line);
		yld = (WORD *)(ddata + y * dib->bytes_per_line);
		for (x = 0; x < w; x++) {
			*yld = ALPHABLEND15(*yls, *yld, lv);
			yls++; yld++;
		}
	}
}

static void image_copyAreaSP16_alphaLevel15(agsurface_t *dib, BYTE *sdata, BYTE *ddata, int w, int h, int lv) {
	int x, y;
	WORD *yls, *yld;
	
	for (y = 0; y < h; y++) {
		yls = (WORD *)(sdata + y * dib->bytes_per_line);
		yld = (WORD *)(ddata + y * dib->bytes_per_line);
		for (x = 0; x < w; x++) {
			*yld = ALPHALEVEL15(*yls, lv);
			yls++; yld++;
		}
	}
}

static void image_copyAreaSP16_whiteLevel15(agsurface_t *dib, BYTE *sdata, BYTE *ddata, int w, int h, int lv) {
	int x, y;
	WORD *yls, *yld;
	
	for (y = 0; y < h; y++) {
		yls = (WORD *)(sdata + y * dib->bytes_per_line);
		yld = (WORD *)(ddata + y * dib->bytes_per_line);
		for (x = 0; x < w; x++) {
			*yld = WHITELEVEL15(*yls, lv);
			yls++; yld++;
		}
	}
}

void image_draw_antialiased_pattern15(agsurface_t *dib, agsurface_t *pattern, int dx, int dy, int dw, int dh, int col) {
	int   x, y;
	BYTE *dst = GETOFFSET_PIXEL(dib, dx, dy);
	BYTE *src = (BYTE *)pattern->pixel;
	WORD *yd;
	BYTE *ys;
	
	for (y = 0; y < dh; y++) {
		ys = (BYTE *)(src + y * pattern->bytes_per_line);
		yd = (WORD *)(dst + y * dib->bytes_per_line);
		for (x = 0; x < dw; x++) {
			if (*ys != 0) {
				*yd = ALPHABLEND15(col, *yd, (BYTE)*ys);
			}
			ys++; yd++;
		}
	}
}

/******************************************************************************/
/* private methods  image操作 16bpp                                           */
/******************************************************************************/

static void drawImage24_fromData16(agsurface_t *dib, cgdata *cg, int dx, int dy, int w, int h) {
	int x, y;
	BYTE* pic_src = (BYTE *)(cg->pic + cg->data_offset);
	BYTE* pic_dst = GETOFFSET_PIXEL(dib, dx, dy);
	WORD *yl;
	
	for (y = 0; y < h; y++) {
		yl = (WORD *)(pic_dst + y * dib->bytes_per_line);
		for (x = 0; x < w; x++) {
			BYTE r, g, b;
			r = *pic_src;
			g = *(pic_src+1);
			b = *(pic_src+2);
			*yl = PIX16(r,g,b);
			yl ++; pic_src+=3;
		}
		pic_src += (cg->width - w);
	}
}

static void drawImage16_fromData16(agsurface_t *dib, cgdata *cg, int dx, int dy, int w, int h) {
	int  y;
	WORD* pic_src = (WORD *)(cg->pic + cg->data_offset);
	BYTE* pic_dst = GETOFFSET_PIXEL(dib, dx, dy);
	
	for (y = 0; y < h; y++) {
		memcpy(pic_dst, pic_src, w * 2);
		pic_src += cg->width;
		pic_dst += dib->bytes_per_line;
	}
}

static void drawSprite16_fromData16(agsurface_t *dib, cgdata *cg, int dx, int dy, int w, int h) {
	int  x, y;
	WORD pic16;
	WORD *pic_src   = (WORD *)(cg->pic + cg->data_offset);
	BYTE *pic_dst   = GETOFFSET_PIXEL(dib, dx, dy);
	BYTE *pic_dst_a = GETOFFSET_ALPHA(dib, dx, dy);
	WORD *yl;
	BYTE *ya;
	
	for (y = 0; y < h; y++) {
		yl = (WORD *)(pic_dst   + y * dib->bytes_per_line);
		ya = (BYTE *)(pic_dst_a + y * dib->width);
		for (x = 0; x < w; x++) {
			pic16 = *pic_src;
			if (*ya == 255) {
				*yl = pic16;
			} else if (*ya > 0) {
				*yl= ALPHABLEND16(pic16, *yl, *ya);
			}
			yl++; ya++; pic_src++;
		}
		pic_src += (cg->width - w);
	}
}

static void expandPixel8to16(agsurface_t *src, agsurface_t *dst) {
	int x, y;
	WORD *yd;
	BYTE *ys;
	
	for (y = 0; y < src->height; y++) {
		ys = (BYTE *)(src->pixel + y * src->bytes_per_line);
		yd = (WORD *)(dst->pixel + y * dst->bytes_per_line);
		for (x = 0; x < src->width; x++) {
			*yd = xpal[*ys].pixel;
			ys++; yd++;
		}
	}
}


static void image_drawRectangle16(agsurface_t *dib, int x, int y, int w, int h, int col) {
	BYTE *dst = GETOFFSET_PIXEL(dib, x, y);
	int i;
	
	/* top */
	for (i = 0; i < w; i++) {
		*((WORD *)dst + i) = col;
	}
	
	/* side */
	h-=2;
	for (i = 0; i < h; i++) {
		dst += dib->bytes_per_line;
		*((WORD *)dst)         = col;
		*((WORD *)dst + w - 1) = col;
	}
	
	/* bottom */
	dst += dib->bytes_per_line;
	for (i = 0; i < w; i++) {
		*((WORD *)dst + i) = col;
	}
}

static void image_copyArea16(agsurface_t *dib, int sx, int sy, int w, int h, int dx, int dy) {
	BYTE *src = GETOFFSET_PIXEL(dib, sx, sy);
	BYTE *dst = GETOFFSET_PIXEL(dib, dx, dy);
	
	if (sy <= dy && dy < (sy + h)) {
		src += (h-1) * dib->bytes_per_line;
		dst += (h-1) * dib->bytes_per_line;

		while (h--) {
			memmove(dst, src, w * 2);
			src -= dib->bytes_per_line;
			dst -= dib->bytes_per_line;
		}
	} else {
	        while(h--) {
			memmove(dst, src, w * 2);
			src += dib->bytes_per_line;
			dst += dib->bytes_per_line;
		}
	}
}			

static void image_getGlyphImage16to16(agsurface_t *dib, agsurface_t *glyph, int dx, int dy, int col) {
	int   x, y;
	BYTE *dst = GETOFFSET_PIXEL(dib, dx, dy);
	BYTE *src = (BYTE *)glyph->pixel;
	WORD *yd;
	WORD *ys;
	
	for (y = 0; y < glyph->height; y++) {
		ys = (WORD *)(src + y * glyph->bytes_per_line);
		yd = (WORD *)(dst + y * dib->bytes_per_line);
		for (x = 0; x < glyph->width; x++) {
			if (*ys != 0) {
				*yd = col;
			}
			ys++; yd++;
		}
	}
}

static void image_wrapColor16(agsurface_t *dib, BYTE *dst, int w, int h, int col, int rate) {
	int x, y;
	WORD *yls, yld;
	
	yld = image_index2pixel(dib->depth, col);
	
	if (nact->mmx_is_ok) {
//	if (0) {
#ifdef ENABLE_MMX
		int alpha = rate | rate << 8 | rate << 16 | rate << 24;
		ablend16_dpd(dst, yld, dst, alpha, w, h, dib->bytes_per_line, dib->bytes_per_line);
#endif
	} else {
		for (y = 0; y < h; y++) {
			yls = (WORD *)(dst + y * dib->bytes_per_line);
			for (x = 0; x < w; x++) {
				*yls = ALPHABLEND16(yld, *yls, rate);
				yls++;
			}
		}
	}
}

static void image_copyAreaSP16_shadow16(agsurface_t *dib, BYTE *sdata, BYTE *ddata, int w, int h, BYTE *adata, int lv) {
	int x, y;
	WORD *yls, *yld;
	BYTE *yla;

	if (nact->mmx_is_ok) {
//	if (0) {
#ifdef ENABLE_MMX
		ablend16_ppp(ddata, sdata, ddata, adata, w, h, dib->bytes_per_line, dib->bytes_per_line, dib->bytes_per_line, dib->width, lv);
#endif
	} else {
		for (y = 0; y < h; y++) {
			yls = (WORD *)(sdata + y * dib->bytes_per_line);
			yld = (WORD *)(ddata + y * dib->bytes_per_line);
			yla = (BYTE *)(adata + y * dib->width);
			for (x = 0; x < w; x++) {
				*yld = ALPHABLEND16(*yls, *yld, (*yla * lv) / 255);
				yls++; yld++; yla++;
			}
		}
	}
}

static void image_copyAreaSP16_transparent16(agsurface_t *dib, BYTE *sdata, BYTE *ddata, int w, int h, int pal) {
	int x, y;
	WORD *yls, *yld;
	WORD yla = image_index2pixel(dib->depth, pal);
	
	for (y = 0; y < h; y++) {
		yls = (WORD *)(sdata + y * dib->bytes_per_line);
		yld = (WORD *)(ddata + y * dib->bytes_per_line);
		for (x = 0; x < w; x++) {
			if (*yls != yla) {
				*yld = *yls;
			}
			yls++; yld++;
		}
	}
}

static void image_copyAreaSP16_alphaBlend16(agsurface_t *dib, BYTE *sdata, BYTE *ddata, int w, int h, int lv) {
	int x, y;
	
	if (nact->mmx_is_ok) {
//	if (0) {
#ifdef ENABLE_MMX
		int alpha = lv | lv << 8 | lv << 16 | lv << 24;
		ablend16_ppd(ddata, sdata, ddata, alpha, w, h, dib->bytes_per_line, dib->bytes_per_line, dib->bytes_per_line);
#endif
	} else {
		WORD *yls, *yld;
		
		for (y = 0; y < h; y++) {
			yls = (WORD *)(sdata + y * dib->bytes_per_line);
			yld = (WORD *)(ddata + y * dib->bytes_per_line);
			for (x = 0; x < w; x++) {
				*yld = ALPHABLEND16(*yls, *yld, lv);
				yls++; yld++;
			}
		}
	}
}

static void image_copyAreaSP16_alphaLevel16(agsurface_t *dib, BYTE *sdata, BYTE *ddata, int w, int h, int lv) {
	int x, y;
	WORD *yls, *yld;

	if (nact->mmx_is_ok) {
//	if (0) {
#ifdef ENABLE_MMX
		int alpha;
		lv = 255 - lv;
		alpha = lv | lv << 8 | lv << 16 | lv << 24;
		ablend16_dpd(ddata, 0, sdata, alpha, w, h, dib->bytes_per_line, dib->bytes_per_line);
#endif
	} else {
		for (y = 0; y < h; y++) {
			yls = (WORD *)(sdata + y * dib->bytes_per_line);
			yld = (WORD *)(ddata + y * dib->bytes_per_line);
			for (x = 0; x < w; x++) {
				*yld = ALPHALEVEL16(*yls, lv);
				yls++; yld++;
			}
		}
	}
}

static void image_copyAreaSP16_whiteLevel16(agsurface_t *dib, BYTE *sdata, BYTE *ddata, int w, int h, int lv) {
	int x, y;
	WORD *yls, *yld;
	
	for (y = 0; y < h; y++) {
		yls = (WORD *)(sdata + y * dib->bytes_per_line);
		yld = (WORD *)(ddata + y * dib->bytes_per_line);
		for (x = 0; x < w; x++) {
			*yld = WHITELEVEL16(*yls, lv);
			yls++; yld++;
		}
	}
}


void image_draw_antialiased_pattern16(agsurface_t *dib, agsurface_t *pattern, int dx, int dy, int dw, int dh, int col) {
	int   x, y;
	BYTE *dst = GETOFFSET_PIXEL(dib, dx, dy);
	BYTE *src = (BYTE *)pattern->pixel;
	WORD *yd;
	BYTE *ys;
	
	for (y = 0; y < dh; y++) {
		ys = (BYTE *)(src + y * pattern->bytes_per_line);
		yd = (WORD *)(dst + y * dib->bytes_per_line);
		for (x = 0; x < dw; x++) {
			if (*ys != 0) {
				*yd = ALPHABLEND16(col, *yd, (BYTE)*ys);
			}
			ys++; yd++;
		}
	}
}

/******************************************************************************/
/* private methods  image操作 packed 24bpp                                    */
/******************************************************************************/

static void expandPixel8to24p(agsurface_t *src, agsurface_t *dst) {
	Palette256 *pal = nact->sys_pal;
	int   x, y;
	BYTE *yd;
	BYTE *ys;
	
	for (y = 0; y < src->height; y++) {
		ys = (BYTE *)(src->pixel + y * src->bytes_per_line);
		yd = (BYTE *)(dst->pixel + y * dst->bytes_per_line);
		for (x = 0; x < src->width; x++) {
			*yd = pal->blue[*ys];  yd++;
			*yd = pal->green[*ys]; yd++;
			*yd = pal->red[*ys];   yd++; ys++;
		}
	}
}

void image_trans_pixel_24to24p(agsurface_t *src, agsurface_t *dst) {
	int   x, y;
	BYTE  *yd;
	DWORD *ys;
	
	for (y = 0; y < src->height; y++) {
		ys = (DWORD *)(src->pixel +  y * src->bytes_per_line);
		yd = (BYTE  *)(dst->pixel +  y * dst->bytes_per_line);
		for (x = 0; x < src->width; x++) {
			*yd = PIXB24(*ys); yd++;
			*yd = PIXG24(*ys); yd++;
			*yd = PIXR24(*ys); yd++; ys++;
		}
	}
}

static void image_getGlyphImage24pto24(agsurface_t *dib, agsurface_t *glyph, int dx, int dy, int col) {
	int   x, y;
	BYTE *dst = GETOFFSET_PIXEL(dib, dx, dy);
	BYTE *src = (BYTE *)glyph->pixel;
	DWORD *yd;
	BYTE  *ys;
	
	for (y = 0; y < glyph->height; y++) {
		ys = (BYTE *)(src + y * glyph->bytes_per_line);
		yd = (DWORD *)(dst + y * dib->bytes_per_line);
		for (x = 0; x < glyph->width; x++) {
			if (*ys != 0) {
				*yd = col;
			}
			ys+=3; yd++;
		}
	}
}

/******************************************************************************/
/* private methods  image操作 24/32bpp                                        */
/******************************************************************************/

static void drawImage16_fromData24(agsurface_t *dib, cgdata *cg, int dx, int dy, int w, int h) {
	int    x, y;
	WORD   pic16;
	WORD  *pic_src = (WORD *)(cg->pic + cg->data_offset);
	BYTE  *pic_dst = GETOFFSET_PIXEL(dib, dx, dy);
	DWORD *yl;
	
	for (y = 0; y < h; y++) {
		yl = (DWORD *)(pic_dst + y * dib->bytes_per_line);
		for (x = 0; x < w; x++) {
			pic16 = *pic_src;
			*yl = PIX24(RGB_PIXR16(pic16),RGB_PIXG16(pic16),RGB_PIXB16(pic16));
			yl ++; pic_src++;
		}
		pic_src += (cg->width - w);
	}
}

static void drawSprite16_fromData24(agsurface_t *dib, cgdata *cg, int dx, int dy, int w, int h) {
	int    x, y;
	WORD   pic16;
	WORD  *pic_src   = (WORD *)(cg->pic + cg->data_offset);
	BYTE  *pic_dst   = GETOFFSET_PIXEL(dib, dx, dy);
	BYTE  *pic_dst_a = GETOFFSET_ALPHA(dib, dx, dy);
	DWORD *yl, pic24;
	BYTE  *ya;
	
	for (y = 0; y < h; y++) {
		yl = (DWORD *)(pic_dst   + y * dib->bytes_per_line);
		ya = (BYTE  *)(pic_dst_a + y * dib->width);
		for (x = 0; x < w; x++) {
			pic16 = *pic_src;
			pic24 = PIX24(RGB_PIXR16(pic16),RGB_PIXG16(pic16),RGB_PIXB16(pic16));
			if (*ya == 255) {
				*yl = pic24;
			} else if (*ya > 0) {
				*yl = ALPHABLEND24(pic24, *yl, *ya);
			}
			yl ++; ya++; pic_src++;
		}
		pic_src += (cg->width - w);
	}
}

static void expandPixel8to24(agsurface_t *src, agsurface_t *dst) {
	int   x, y;
	DWORD *yd;
	BYTE  *ys;
	
	for (y = 0; y < src->height; y++) {
		ys = (BYTE  *)(src->pixel + y * src->bytes_per_line);
		yd = (DWORD *)(dst->pixel + y * dst->bytes_per_line);
		for (x = 0; x < src->width; x++) {
			*yd = xpal[*ys].pixel;
			ys++; yd++;
		}
	}
}

static void image_drawRectangle24(agsurface_t *dib, int x, int y, int w, int h, int col) {
	BYTE *dst = GETOFFSET_PIXEL(dib, x, y);
	int i;
	
	/* top */
	for (i = 0; i < w; i++) {
		*((DWORD *)dst + i) = col;
	}
	
	/* side */
	h-=2;
	for (i = 0; i < h; i++) {
		dst += dib->bytes_per_line;
		*((DWORD *)dst)         = col;
		*((DWORD *)dst + w - 1) = col;
	}
	
	/* bottom */
	dst += dib->bytes_per_line;
	for (i = 0; i < w; i++) {
 		*((DWORD *)dst + i) = col;
	}
}

static void image_copyArea24(agsurface_t *dib, int sx, int sy, int w, int h, int dx, int dy) {
	BYTE *src = GETOFFSET_PIXEL(dib, sx, sy);
	BYTE *dst = GETOFFSET_PIXEL(dib, dx, dy);
	
	if (sy <= dy && dy < (sy + h)) {
		src += (h-1) * dib->bytes_per_line;
		dst += (h-1) * dib->bytes_per_line;

		while (h--) {
 			memmove(dst, src, w * 4);
			src -= dib->bytes_per_line;
			dst -= dib->bytes_per_line;
		}
	} else {
	        while(h--) {
			memmove(dst, src, w * 4);
			src += dib->bytes_per_line;
			dst += dib->bytes_per_line;
		}
	}
}			

static void image_getGlyphImage24to24(agsurface_t *dib, agsurface_t *glyph, int dx, int dy, int col) {
	int   x, y;
	BYTE *dst = GETOFFSET_PIXEL(dib, dx, dy);
	BYTE *src = (BYTE *)glyph->pixel;
	DWORD *yd;
	DWORD *ys;
	
	for (y = 0; y < glyph->height; y++) {
		ys = (DWORD *)(src + y * glyph->bytes_per_line);
		yd = (DWORD *)(dst + y * dib->bytes_per_line);
		for (x = 0; x < glyph->width; x++) {
			if (*ys != 0) {
				*yd = col;
			}
			ys++; yd++;
		}
	}
}

static void image_wrapColor24(agsurface_t *dib, BYTE *dst, int w, int h, int col, int rate) {
	int x, y;
	DWORD *yls, yld;
	
	yld = image_index2pixel(dib->depth, col);
	
	for (y = 0; y < h; y++) {
		yls = (DWORD *)(dst + y * dib->bytes_per_line);
		for (x = 0; x < w; x++) {
			*yls = ALPHABLEND24(yld, *yls, rate);
			yls++;
		}
	}
}

static void image_copyAreaSP16_shadow24(agsurface_t *dib, BYTE *sdata, BYTE *ddata, int w, int h, BYTE *adata, int lv) {
	int x, y;
	DWORD *yls, *yld;
	BYTE *yla;

	for (y = 0; y < h; y++) {
		yls = (DWORD *)(sdata + y * dib->bytes_per_line);
		yld = (DWORD *)(ddata + y * dib->bytes_per_line);
		yla = (BYTE *)(adata + y * dib->width);
		for (x = 0; x < w; x++) {
			*yld = ALPHABLEND24(*yls, *yld, (*yla * lv) / 255);
			yls++; yld++; yla++;
		}
	}
}

static void image_copyAreaSP16_transparent24(agsurface_t *dib, BYTE *sdata, BYTE *ddata, int w, int h, int pal) {
	int x, y;
	DWORD *yls, *yld;
	DWORD yla = image_index2pixel(dib->depth, pal) & 0xf0f0f0;
	
	for (y = 0; y < h; y++) {
		yls = (DWORD *)(sdata + y * dib->bytes_per_line);
		yld = (DWORD *)(ddata + y * dib->bytes_per_line);
		for (x = 0; x < w; x++) {
			if ((*yls & 0xf0f0f0) != yla) {
				*yld = *yls;
			}
			yls++; yld++;
		}
	}
}

static void image_copyAreaSP16_alphaBlend24(agsurface_t *dib, BYTE *sdata, BYTE *ddata, int w, int h, int lv) {
	int x, y;
	DWORD *yls, *yld;
	
	for (y = 0; y < h; y++) {
		yls = (DWORD *)(sdata + y * dib->bytes_per_line);
		yld = (DWORD *)(ddata + y * dib->bytes_per_line);
		for (x = 0; x < w; x++) {
			*yld = ALPHABLEND24(*yls, *yld, lv);
			yls++; yld++;
		}
	}
}

static void image_copyAreaSP16_alphaLevel24(agsurface_t *dib, BYTE *sdata, BYTE *ddata, int w, int h, int lv) {
	int x, y;
	DWORD *yls, *yld;
	
	for (y = 0; y < h; y++) {
		yls = (DWORD *)(sdata + y * dib->bytes_per_line);
		yld = (DWORD *)(ddata + y * dib->bytes_per_line);
		for (x = 0; x < w; x++) {
			*yld = ALPHALEVEL24(*yls, lv);
			yls++; yld++;
		}
	}
}

static void image_copyAreaSP16_whiteLevel24(agsurface_t *dib, BYTE *sdata, BYTE *ddata, int w, int h, int lv) {
	int x, y;
	DWORD *yls, *yld;
	
	for (y = 0; y < h; y++) {
		yls = (DWORD *)(sdata + y * dib->bytes_per_line);
		yld = (DWORD *)(ddata + y * dib->bytes_per_line);
		for (x = 0; x < w; x++) {
			*yld = WHITELEVEL24(*yls, lv);
			yls++; yld++;
		}
	}
}

void image_draw_antialiased_pattern24(agsurface_t *dib, agsurface_t *pattern, int dx, int dy, int dw, int dh, int col) {
	int   x, y;
	BYTE *dst = GETOFFSET_PIXEL(dib, dx, dy);
	BYTE  *src = (BYTE *)pattern->pixel;
	DWORD *yd;
	BYTE  *ys;

	for (y = 0; y < dh; y++) {
		ys = (BYTE  *)(src + y * pattern->bytes_per_line);
		yd = (DWORD *)(dst + y * dib->bytes_per_line);
		for (x = 0; x < dw; x++) {
			if (*ys) {
				*yd = ALPHABLEND24(col, *yd, (BYTE)*ys);
			}
			ys++; yd++;
		}
	}
}


static void (*draw_image24_from_data)(agsurface_t *, cgdata *, int, int, int, int);
static void (*draw_sprite16_from_data)(agsurface_t *, cgdata *, int, int, int, int);
static void (*draw_image16_from_data)(agsurface_t *, cgdata *, int, int, int, int);
static void (*draw_rectangle)(agsurface_t*, int, int, int, int, int);
static void (*copy_area)(agsurface_t*, int, int, int, int, int, int);
static void (*copy_area_ablend_sda)(agsurface_t *, BYTE *, BYTE *, int, int, BYTE *, int);
static void (*copy_area_ablend_sd)(agsurface_t *, BYTE *, BYTE *, int, int, int);
static void (*copy_area_alevel)(agsurface_t *, BYTE *, BYTE *, int, int, int);
static void (*copy_area_wlevel)(agsurface_t *, BYTE *, BYTE *, int, int, int);
static void (*wrap_color)(agsurface_t *, BYTE *, int, int, int, int); 
static void (*copy_area_sprite)(agsurface_t *, BYTE *, BYTE *, int, int, int);
static void (*draw_antialiased_pattern)(agsurface_t *, agsurface_t *, int, int, int, int, int);

/*
 * dib の depth に応じた関数の設定
 *   depth: dib depth
*/
void xcore_image_setdepth(int depth) {
	switch(depth) {
	case 8:
		draw_rectangle = image_drawRectangle8;
		copy_area = image_copyArea8;
		copy_area_sprite = image_copyAreaSP8;
		break;
	case 15:
		// draw_image24_from_data = drawImage24_fromData15;
		draw_sprite16_from_data = drawSprite16_fromData15;
		draw_image16_from_data = drawImage16_fromData15;
		draw_rectangle = image_drawRectangle16;
		copy_area = image_copyArea16;
		copy_area_ablend_sda = image_copyAreaSP16_shadow15;
		copy_area_ablend_sd = image_copyAreaSP16_alphaBlend15;
		copy_area_alevel = image_copyAreaSP16_alphaLevel15;
		copy_area_wlevel = image_copyAreaSP16_whiteLevel15;
		wrap_color = image_wrapColor15;
		copy_area_sprite = image_copyAreaSP16_transparent16;
		draw_antialiased_pattern = image_draw_antialiased_pattern15;
		break;
	case 16:
		draw_image24_from_data = drawImage24_fromData16;
		draw_sprite16_from_data = drawSprite16_fromData16;
		draw_image16_from_data = drawImage16_fromData16;
		draw_rectangle = image_drawRectangle16;
		copy_area = image_copyArea16;
		copy_area_ablend_sda = image_copyAreaSP16_shadow16;
		copy_area_ablend_sd = image_copyAreaSP16_alphaBlend16;
		copy_area_alevel = image_copyAreaSP16_alphaLevel16;
		copy_area_wlevel = image_copyAreaSP16_whiteLevel16;
		wrap_color = image_wrapColor16;
		copy_area_sprite = image_copyAreaSP16_transparent16;
		draw_antialiased_pattern = image_draw_antialiased_pattern16;
		break;
	case 24:
	case 32:
		//draw_image24_from_data = drawImage24_fromData32;
		draw_sprite16_from_data = drawSprite16_fromData24;
		draw_image16_from_data = drawImage16_fromData24;
		draw_rectangle = image_drawRectangle24;
		copy_area = image_copyArea24;
		copy_area_ablend_sda = image_copyAreaSP16_shadow24;
		copy_area_ablend_sd = image_copyAreaSP16_alphaBlend24;
		copy_area_alevel = image_copyAreaSP16_alphaLevel24;
		copy_area_wlevel = image_copyAreaSP16_whiteLevel24;
		wrap_color = image_wrapColor24;
		copy_area_sprite = image_copyAreaSP16_transparent24;
		draw_antialiased_pattern = image_draw_antialiased_pattern24;
		break;
	default:
		break;
	}
}


static void trans_index2pixels(int depth, int lv, cgdata *cg) {
	Palette256 *pal;
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
	row = calloc(dw+1, sizeof(int));
	// 1おおきくして初期化しないと col[dw-1]とcol[dw]が同じになる
	// 可能性がある。
	col = calloc(dh+1, sizeof(int));

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
	
	free(row);
	free(col);
	
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
	BYTE *pic_save = NULL;
	
	/* set alpha Level */
	if (cg->alphalevel != 255) {
		pic_save = cg->pic;
		cg->pic = changeImage16AlphaLevel(cg);
	}
	
	if (cg->spritecolor >= 0) {
		draw_sprite16_from_data(dib, cg, x, y, w, h);
	} else {
		draw_image16_from_data(dib, cg, x, y, w, h);
	}
	
	if (cg->alphalevel != 255) {
		free(cg->pic);
		cg->pic = pic_save;
	}
}

/*
 * dib に矩形を描画
 */
void image_drawRectangle(agsurface_t *dib, int x, int y, int w, int h, int col) {
	draw_rectangle(dib, x, y, w, h, col);
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
void image_getPixel(agsurface_t *dib, int x, int y, Palette *cell) {
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
	agsurface_t *i = malloc(sizeof(agsurface_t));
	int    j;
	BYTE *ys = GETOFFSET_PIXEL(dib, x, y);
	BYTE *yd;

	// printf("save region %d,%d,%d,%d\n", x, y, w, h);
	
	i->width          = w;
	i->height         = h;
	i->bytes_per_line = w * dib->bytes_per_pixel;
	i->bytes_per_pixel = dib->bytes_per_pixel;
	i->pixel = yd = malloc(sizeof(char) * (w * h * dib->bytes_per_pixel));
	
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
	free(dst->pixel);
	free(dst);
}

/*
 * save した領域の解放
 */
void image_delRegion(agsurface_t *r) {
	free(r->pixel);
	free(r);
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
