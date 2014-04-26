/*
 * image24.c  image¡‡∫Ó(24/32bpp)
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
/* $Id: image24.c,v 1.17 2006/04/21 16:40:48 chikama Exp $ */

#include <stdio.h>
#include "portab.h"

static void fadeOut24(agsurface_t *dst, int lv, int col) {
	DWORD *yld;
	int x, y;
	
	for (y = 0; y < dst->height; y+=4) {
		yld = (DWORD *)(dst->pixel + (y + fadeY[lv]) * dst->bytes_per_line);
		for (x = 0; x < dst->width; x+=4) {
			*(yld + fadeX[lv]) = col;
			yld+=4;
		}
	}
}

static void fadeIn24(agsurface_t *src, agsurface_t *dst, int lv) {
	DWORD *yls, *yld;
	int x, y;
	
	for (y = 0; y < src->height; y+=4) {
		yls = (DWORD *)(src->pixel + (y + fadeY[lv]) * src->bytes_per_line);
		yld = (DWORD *)(dst->pixel + (y + fadeY[lv]) * dst->bytes_per_line);
		for (x = 0; x < src->width; x+=4) {
			*(yld + fadeX[lv]) = *(yls + fadeX[lv]);
			yls+=4; yld+=4;
		}
	}
}

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

static void image_drawLine24(agsurface_t *dib, int x0, int y0, int x1, int y1, int col) {
	int dx = abs(x0 - x1), dy = abs(y0 - y1);
	
	if (dx == 0) {
		int i = min(y0, y1), d = dib->bytes_per_line / 4;
		DWORD *p = (DWORD *)GETOFFSET_PIXEL(dib, x0, i);
		
		for (i = 0; i < dy; i++) {
			*p = col;
			p += d;
		}
		
	} else if (dy == 0) {
		int i = min(x0, x1), j;
		DWORD *p = (DWORD *)GETOFFSET_PIXEL(dib, i, y0);
		
		for (j = 0; j < dx; j++) {
			*(p++) = col;
		}
		
	} else if (dx == dy) {
		int i;
		DWORD *p;
		
		if (x0 < x1) {
			p = (DWORD *)GETOFFSET_PIXEL(dib, x0, y0);
			if (y0 < y1) {
				dy =  dib->bytes_per_line + dib->bytes_per_pixel;
			} else {
				dy = -dib->bytes_per_line + dib->bytes_per_pixel;
			}
		} else {
			p = (DWORD *)GETOFFSET_PIXEL(dib, x1, y1);
			if (y0 < y1) {
				dy = -dib->bytes_per_line + dib->bytes_per_pixel;
			} else {
				dy =  dib->bytes_per_line + dib->bytes_per_pixel;
			}
		}
		dy /= 4;
		for (i = 0; i < dx; i++) {
			*p = col;
			p += dy;
		}
	} else {
		int i, d1, d2, ds, dd, imax;
		DWORD *p;

		if (dx < dy) {
			d1 = dib->bytes_per_line;
			if (y0 > y1) {
				p  = (DWORD *)GETOFFSET_PIXEL(dib, x1, y1);
				d2 = dib->bytes_per_pixel * (x0 < x1 ? -1 : 1);
			} else {
				p  = (DWORD *)GETOFFSET_PIXEL(dib, x0, y0);
				d2 = dib->bytes_per_pixel * (x0 < x1 ? 1 : -1);
			}
			ds   = dx;
			imax = dy;
		} else {
			d1 = dib->bytes_per_pixel;
			if (x0 > x1) {
				p  = (DWORD *)GETOFFSET_PIXEL(dib, x1, y1);
				d2 = dib->bytes_per_line * (y0 < y1 ? -1 : 1);
			} else {
				p  = (DWORD *)GETOFFSET_PIXEL(dib, x0, y0);
				d2 = dib->bytes_per_line * (y0 < y1 ? 1 : -1);
			}
			ds   = dy;
			imax = dx;
		}
		dd = 0;
		d1 /= 4;
		d2 /= 4;
		for (i = 0; i < imax; i++) {
			*p = col;
			p += d1;
			dd += ds;
			if (dd > imax) {
				p  += d2;
				dd -= imax;
			}
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

static void image_fillRectangle24(agsurface_t *dib, int x, int y, int w, int h, int col) {
	BYTE *_dst, *dst = GETOFFSET_PIXEL(dib, x, y);
	int i;

	_dst = dst;
	
	for (i = 0; i < w; i++) {
		*((DWORD *)dst + i) = col;
	}
	
	for (i = 0; i < h -1; i++) {
		dst += dib->bytes_per_line;
		memcpy(dst, _dst, w * 4);
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
	
	yld = trans_index2pixel(dib->depth, col);
	
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
	DWORD yla = trans_index2pixel(dib->depth, pal) & 0xf0f0f0;
	
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

static void image_copy_from_alpha24(agsurface_t *dib, BYTE *sdata, BYTE *ddata, int w, int h, ALPHA_DIB_COPY_TYPE flag) {
	int x, y;
	BYTE *yls;
	DWORD *yld;
	
	switch(flag) {
	case TO_16H:
		for (y = 0; y < h; y++) {
			yls = (BYTE *)(sdata + y * dib->width);
			yld = (DWORD *)(ddata + y * dib->bytes_per_line);
			for (x = 0; x < w; x++) {
				/*  *yld = */ 
				yld++; yls++;
			}
		}
		break;
	case TO_16L:
		for (y = 0; y < h; y++) {
			yls = (BYTE *)(sdata + y * dib->width);
			yld = (DWORD *)(ddata + y * dib->bytes_per_line);
			for (x = 0; x < w; x++) {
				/* *yld = (WORD)(*yls) | (*yld & 0xff00); */
				yld++; yls++;
			}
		}
		break;
	case TO_24R:
		for (y = 0; y < h; y++) {
			yls = (BYTE *)(sdata + y * dib->width);
			yld = (DWORD *)(ddata + y * dib->bytes_per_line);
			for (x = 0; x < w; x++) {
				*yld = PIX24(*yls, PIXG24(*yld), PIXB24(*yld));
				yld++; yls++;
			}
		}
		break;
	case TO_24G:
		for (y = 0; y < h; y++) {
			yls = (BYTE *)(sdata + y * dib->width);
			yld = (DWORD *)(ddata + y * dib->bytes_per_line);
			for (x = 0; x < w; x++) {
				*yld = PIX24(PIXR24(*yls), *yls, PIXB24(*yld));
				yld++; yls++;
			}
		}
		break;
	case TO_24B:
		for (y = 0; y < h; y++) {
			yls = (BYTE *)(sdata + y * dib->width);
			yld = (DWORD *)(ddata + y * dib->bytes_per_line);
			for (x = 0; x < w; x++) {
				*yld = PIX24(PIXR24(*yld), PIXG24(*yld), *yls);
				yld++; yls++;
			}
		}
		break;
	default:
		break;
	}
}

static void image_copy_to_alpha24(agsurface_t *dib, BYTE *sdata, BYTE *ddata, int w, int h, ALPHA_DIB_COPY_TYPE flag) {
	int x, y;
	BYTE *yld;
	DWORD *yls;
	
	switch(flag) {
	case FROM_16H:
		for (y = 0; y < h; y++) {
			yls = (DWORD *)(sdata + y * dib->bytes_per_line);
			yld = (BYTE  *)(ddata + y * dib->width);
			for (x = 0; x < w; x++) {
				*yld = (BYTE)(PIX16(PIXR24(*yls), PIXG24(*yls), PIXB24(*yls)) >> 8);
				yld++; yls++;
			}
		}
		break;
	case FROM_16L:
		for (y = 0; y < h; y++) {
			yls = (DWORD *)(sdata + y * dib->bytes_per_line);
			yld = (BYTE  *)(ddata + y * dib->width);
			for (x = 0; x < w; x++) {
				*yld = (BYTE)(PIX16(PIXR24(*yls), PIXG24(*yls), PIXB24(*yls)));
					yld++; yls++;
			}
		}
		break;
	case FROM_24R:
		for (y = 0; y < h; y++) {
			yls = (DWORD *)(sdata + y * dib->bytes_per_line);
			yld = (BYTE  *)(ddata + y * dib->width);
			for (x = 0; x < w; x++) {
				*yld = (BYTE)PIXR24(*yls);
				yld++; yls++;
			}
		}
		break;
	case FROM_24G:
		for (y = 0; y < h; y++) {
			yls = (DWORD *)(sdata + y * dib->bytes_per_line);
			yld = (BYTE  *)(ddata + y * dib->width);
			for (x = 0; x < w; x++) {
				*yld = (BYTE)PIXG24(*yls);
				yld++; yls++;
			}
		}
		break;
	case FROM_24B:
		for (y = 0; y < h; y++) {
			yls = (DWORD *)(sdata + y * dib->bytes_per_line);
			yld = (BYTE  *)(ddata + y * dib->width);
			for (x = 0; x < w; x++) {
				*yld = (BYTE)PIXB24(*yls);
				yld++; yls++;
			}
		}
		break;
	default:
		break;
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
