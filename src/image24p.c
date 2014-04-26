/*
 * image24p.c  image¡‡∫Ó(packed 24bpp)
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
/* $Id: image24p.c,v 1.4 2006/04/21 16:40:48 chikama Exp $ */

#include "portab.h"
#include "image.h"

static void expandPixel8to24p(agsurface_t *src, agsurface_t *dst) {
	Pallet256 *pal = nact->sys_pal;
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

static void fadeOut24p(agsurface_t *dst, int lv, int col) {
	BYTE *yld;
	int x, y;
	
	for (y = 0; y < dst->height; y+=4) {
		yld = (BYTE *)(dst->pixel + (y + fadeY[lv]) * dst->bytes_per_line);
		for (x = 0; x < dst->width; x+=4) {
			*(yld + fadeX[lv])    = 
			*(yld + fadeX[lv] +1) =
			*(yld + fadeX[lv] +2) = (BYTE)col;
			yld += (4*3);
		}
	}
}

static void fadeIn24p(agsurface_t *src, agsurface_t *dst, int lv) {
	DWORD *yls;
	BYTE  *yld;
	int x, y;
	
	for (y = 0; y < src->height; y+=4) {
		yls = (DWORD *)(src->pixel + (y + fadeY[lv]) * src->bytes_per_line);
		yld = (BYTE  *)(dst->pixel + (y + fadeY[lv]) * dst->bytes_per_line);
		for (x = 0; x < src->width; x+=4) {
			*(yld + fadeX[lv]   ) = PIXB24(*(yls + fadeX[lv]));
			*(yld + fadeX[lv] +1) = PIXG24(*(yls + fadeX[lv]));
			*(yld + fadeX[lv] +2) = PIXR24(*(yls + fadeX[lv]));
			yls+=4; yld+=(4*3);
		}
	}
}
