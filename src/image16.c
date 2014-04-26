/*
 * image16.c  image¡‡∫Ó(16bpp)
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
/* $Id: image16.c,v 1.20 2003/05/03 03:44:27 chikama Exp $ */

#include "config.h"
#include "portab.h"

static void fadeOut16(agsurface_t *dst, int lv, int col) {
	WORD *yld;
	int x, y;
	
	for (y = 0; y < dst->height; y+=4) {
		yld = (WORD *)(dst->pixel + (y + fadeY[lv]) * dst->bytes_per_line);
		for (x = 0; x < dst->width; x+=4) {
			*(yld + fadeX[lv]) = col;
			yld+=4;
		}
	}
}

static void fadeIn16(agsurface_t *src, agsurface_t *dst, int lv) {
	WORD *yls, *yld;
	int x, y;
	
	for (y = 0; y < src->height; y+=4) {
		yls = (WORD *)(src->pixel + (y + fadeY[lv]) * src->bytes_per_line);
		yld = (WORD *)(dst->pixel + (y + fadeY[lv]) * dst->bytes_per_line);
		for (x = 0; x < src->width; x+=4) {
			*(yld + fadeX[lv]) = *(yls + fadeX[lv]);
			yls+=4; yld+=4;
		}
	}
}

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

static void image_drawLine16(agsurface_t *dib, int x0, int y0, int x1, int y1, int col) {
	int dx = abs(x0 - x1), dy = abs(y0 - y1);
	
	if (dx == 0) {
		int i = min(y0, y1), d = dib->bytes_per_line / 2;
		WORD *p = (WORD *)GETOFFSET_PIXEL(dib, x0, i);

		for (i = 0; i < dy; i++) {
			*p = col;
			p += d;
		}
		
	} else if (dy == 0) {
		int i = min(x0, x1), j;
		WORD *p = (WORD *)GETOFFSET_PIXEL(dib, i, y0);
		
		for (j = 0; j < dx; j++) {
			*(p++) = col;
		}
		
	} else if (dx == dy) {
		int i;
		WORD *p;
		
		if (x0 < x1) {
			p = (WORD *)GETOFFSET_PIXEL(dib, x0, y0);
			if (y0 < y1) {
				dy =  dib->bytes_per_line + dib->bytes_per_pixel;
			} else {
				dy = -dib->bytes_per_line + dib->bytes_per_pixel;
			}
		} else {
			p = (WORD *)GETOFFSET_PIXEL(dib, x1, y1);
			if (y0 < y1) {
				dy = -dib->bytes_per_line + dib->bytes_per_pixel;
			} else {
				dy =  dib->bytes_per_line + dib->bytes_per_pixel;
			}
		}
		dy /= 2;
		for (i = 0; i < dx; i++) {
			*p = col;
			p += dy;
		}
	} else {
		int i, d1, d2, ds, dd, imax;
		WORD *p;

		if (dx < dy) {
			d1 = dib->bytes_per_line;
			if (y0 > y1) {
				p  = (WORD *)GETOFFSET_PIXEL(dib, x1, y1);
				d2 = dib->bytes_per_pixel * (x0 < x1 ? -1 : 1);
			} else {
				p  = (WORD *)GETOFFSET_PIXEL(dib, x0, y0);
				d2 = dib->bytes_per_pixel * (x0 < x1 ? 1 : -1);
			}
			ds   = dx;
			imax = dy;
		} else {
			d1 = dib->bytes_per_pixel;
			if (x0 > x1) {
				p  = (WORD *)GETOFFSET_PIXEL(dib, x1, y1);
				d2 = dib->bytes_per_line * (y0 < y1 ? -1 : 1);
			} else {
				p  = (WORD *)GETOFFSET_PIXEL(dib, x0, y0);
				d2 = dib->bytes_per_line * (y0 < y1 ? 1 : -1);
			}
			ds   = dy;
			imax = dx;
		}
		dd = 0;
		d1 /= 2;
		d2 /= 2;
		for (i = 0; i < imax; i++) {
			*p = col;
			p  += d1;
			dd += ds;
			if (dd > imax) {
				p  += d2;
				dd -= imax;
			}
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

static void image_fillRectangle16(agsurface_t *dib, int x, int y, int w, int h, int col) {
	BYTE *_dst, *dst = GETOFFSET_PIXEL(dib, x, y);
	int i;

	_dst = dst;
	
	for (i = 0; i < w; i++) {
		*((WORD *)dst + i) = col;
	}
	
	for (i = 0; i < h -1; i++) {
		dst += dib->bytes_per_line;
		memcpy(dst, _dst, w * 2);
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
	
	yld = trans_index2pixel(dib->depth, col);
	
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
	WORD yla = trans_index2pixel(dib->depth, pal);
	
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

static void image_copy_from_alpha16(agsurface_t *dib, BYTE *sdata, BYTE *ddata, int w, int h, ALPHA_DIB_COPY_TYPE flag) {
	int x, y;
	BYTE *yls;
	WORD *yld;
	
	switch(flag) {
	case TO_16H:
		for (y = 0; y < h; y++) {
			yls = (BYTE *)(sdata + y * dib->width);
			yld = (WORD *)(ddata + y * dib->bytes_per_line);
			for (x = 0; x < w; x++) {
				*yld = (WORD)(*yls << 8) | (*yld & 0xff);
				yld++; yls++;
			}
		}
		break;
	case TO_16L:
		for (y = 0; y < h; y++) {
			yls = (BYTE *)(sdata + y * dib->width);
			yld = (WORD *)(ddata + y * dib->bytes_per_line);
			for (x = 0; x < w; x++) {
				*yld = (WORD)(*yls) | (*yld & 0xff00);
				yld++; yls++;
			}
		}
		break;
	case TO_24R:
		for (y = 0; y < h; y++) {
			yls = (BYTE *)(sdata + y * dib->width);
			yld = (WORD *)(ddata + y * dib->bytes_per_line);
			for (x = 0; x < w; x++) {
				*yld = PIX16(*yls, PIXG16(*yld), PIXB16(*yld));
				yld++; yls++;
			}
		}
		break;
	case TO_24G:
		for (y = 0; y < h; y++) {
			yls = (BYTE *)(sdata + y * dib->width);
			yld = (WORD *)(ddata + y * dib->bytes_per_line);
			for (x = 0; x < w; x++) {
				*yld = PIX16(PIXR16(*yls), *yls, PIXB16(*yld));
				yld++; yls++;
			}
		}
		break;
	case TO_24B:
		for (y = 0; y < h; y++) {
			yls = (BYTE *)(sdata + y * dib->width);
			yld = (WORD *)(ddata + y * dib->bytes_per_line);
			for (x = 0; x < w; x++) {
				*yld = PIX16(PIXR16(*yld), PIXG16(*yld), *yls);
				yld++; yls++;
			}
		}
		break;
	default:
		break;
	}
}

static void image_copy_to_alpha16(agsurface_t *dib, BYTE *sdata, BYTE *ddata, int w, int h, ALPHA_DIB_COPY_TYPE flag) {
	int x, y;
	BYTE *yld;
	WORD *yls;
	
	switch(flag) {
	case FROM_16H:
		for (y = 0; y < h; y++) {
			yls = (WORD *)(sdata + y * dib->bytes_per_line);
			yld = (BYTE *)(ddata + y * dib->width);
			for (x = 0; x < w; x++) {
				*yld = (BYTE)(*yls >> 8);
				yld++; yls++;
			}
		}
		break;
	case FROM_16L:
		for (y = 0; y < h; y++) {
			yls = (WORD *)(sdata + y * dib->bytes_per_line);
			yld = (BYTE *)(ddata + y * dib->width);
			for (x = 0; x < w; x++) {
				*yld = (BYTE)(*yls);
				yld++; yls++;
			}
		}
		break;
	case FROM_24R:
		for (y = 0; y < h; y++) {
			yls = (WORD *)(sdata + y * dib->bytes_per_line);
			yld = (BYTE *)(ddata + y * dib->width);
			for (x = 0; x < w; x++) {
				*yld = PIXR16(*yls);
				yld++; yls++;
			}
		}
		break;
	case FROM_24G:
		for (y = 0; y < h; y++) {
			yls = (WORD *)(sdata + y * dib->bytes_per_line);
			yld = (BYTE *)(ddata + y * dib->width);
			for (x = 0; x < w; x++) {
				*yld = PIXG16(*yls);
				yld++; yls++;
			}
		}
		break;
	case FROM_24B:
		for (y = 0; y < h; y++) {
			yls = (WORD *)(sdata + y * dib->bytes_per_line);
			yld = (BYTE *)(ddata + y * dib->width);
			for (x = 0; x < w; x++) {
				*yld = PIXB16(*yls);
				yld++; yls++;
			}
		}
		break;
	default:
		break;
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
