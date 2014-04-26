/*
 * image15.c  image¡‡∫Ó(15bpp)
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
/* $Id: image15.c,v 1.16 2006/04/21 16:40:48 chikama Exp $ */

#include <stdio.h>
#include "portab.h"

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
	
	yld = trans_index2pixel(dib->depth, col);
	
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

static void image_copy_from_alpha15(agsurface_t *dib, BYTE *sdata, BYTE *ddata, int w, int h, ALPHA_DIB_COPY_TYPE flag) {
	int x, y;
	BYTE *yls;
	WORD *yld;
	
	switch(flag) {
	case TO_16H:
		for (y = 0; y < h; y++) {
			yls = (BYTE *)(sdata + y * dib->width);
			yld = (WORD *)(ddata + y * dib->bytes_per_line);
			for (x = 0; x < w; x++) {
				/*  *yld = */ 
				yld++; yls++;
			}
		}
		break;
	case TO_16L:
		for (y = 0; y < h; y++) {
			yls = (BYTE *)(sdata + y * dib->width);
			yld = (WORD *)(ddata + y * dib->bytes_per_line);
			for (x = 0; x < w; x++) {
				/* *yld = (WORD)(*yls) | (*yld & 0xff00); */
				yld++; yls++;
			}
		}
		break;
	case TO_24R:
		for (y = 0; y < h; y++) {
			yls = (BYTE *)(sdata + y * dib->width);
			yld = (WORD *)(ddata + y * dib->bytes_per_line);
			for (x = 0; x < w; x++) {
				*yld = PIX15(*yls, PIXG15(*yld), PIXB15(*yld));
				yld++; yls++;
			}
		}
		break;
	case TO_24G:
		for (y = 0; y < h; y++) {
			yls = (BYTE *)(sdata + y * dib->width);
			yld = (WORD *)(ddata + y * dib->bytes_per_line);
			for (x = 0; x < w; x++) {
				*yld = PIX15(PIXR15(*yls), *yls, PIXB15(*yld));
				yld++; yls++;
			}
		}
		break;
	case TO_24B:
		for (y = 0; y < h; y++) {
			yls = (BYTE *)(sdata + y * dib->width);
			yld = (WORD *)(ddata + y * dib->bytes_per_line);
			for (x = 0; x < w; x++) {
				*yld = PIX15(PIXR15(*yld), PIXG15(*yld), *yls);
				yld++; yls++;
			}
		}
		break;
	default:
		break;
	}
}

static void image_copy_to_alpha15(agsurface_t *dib, BYTE *sdata, BYTE *ddata, int w, int h, ALPHA_DIB_COPY_TYPE flag) {
	int x, y;
	BYTE *yld;
	WORD *yls;
	
	switch(flag) {
	case FROM_16H:
		for (y = 0; y < h; y++) {
			yls = (WORD *)(sdata + y * dib->bytes_per_line);
			yld = (BYTE *)(ddata + y * dib->width);
			for (x = 0; x < w; x++) {
				*yld = (BYTE)(PIX16(PIXR15(*yls), PIXG15(*yls), PIXB15(*yls)) >> 8);
				yld++; yls++;
			}
		}
		break;
	case FROM_16L:
		for (y = 0; y < h; y++) {
			yls = (WORD *)(sdata + y * dib->bytes_per_line);
			yld = (BYTE *)(ddata + y * dib->width);
			for (x = 0; x < w; x++) {
				*yld = (BYTE)(PIX16(PIXR15(*yls), PIXG15(*yls), PIXB15(*yls)));
					yld++; yls++;
			}
		}
		break;
	case FROM_24R:
		for (y = 0; y < h; y++) {
			yls = (WORD *)(sdata + y * dib->bytes_per_line);
			yld = (BYTE *)(ddata + y * dib->width);
			for (x = 0; x < w; x++) {
				*yld = (BYTE)PIXR15(*yls);
				yld++; yls++;
			}
		}
		break;
	case FROM_24G:
		for (y = 0; y < h; y++) {
			yls = (WORD *)(sdata + y * dib->bytes_per_line);
			yld = (BYTE *)(ddata + y * dib->width);
			for (x = 0; x < w; x++) {
				*yld = (BYTE)PIXG15(*yls);
				yld++; yls++;
			}
		}
		break;
	case FROM_24B:
		for (y = 0; y < h; y++) {
			yls = (WORD *)(sdata + y * dib->bytes_per_line);
			yld = (BYTE *)(ddata + y * dib->width);
			for (x = 0; x < w; x++) {
				*yld = (BYTE)PIXB15(*yls);
				yld++; yls++;
			}
		}
		break;
	default:
		break;
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
