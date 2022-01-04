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
#include "config.h"

#include <stdlib.h>
#include <string.h>
#include "portab.h"
#include "system.h"
#include "image.h"
#include "cg.h"
#include "config.h"
#include "nact.h"
#include "sdl_core.h"
#include "alpha_plane.h"
#include "ags.h"

static void (*copy_from_alpha)(agsurface_t *, BYTE *, BYTE *, int, int, ALPHA_DIB_COPY_TYPE);
static void (*copy_to_alpha)(agsurface_t *, BYTE *, BYTE *, int, int, ALPHA_DIB_COPY_TYPE);

/******************************************************************************/
/* private methods  image操作 16bpp                                           */
/******************************************************************************/

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

/******************************************************************************/
/* private methods  image操作 24/32bpp                                        */
/******************************************************************************/

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


/*
 * dib の depth に応じた関数の設定
 *   depth: dib depth
*/
void image_setdepth(int depth) {
	switch(depth) {
	case 8:
		break;
	case 16:
		copy_from_alpha = image_copy_from_alpha16;
		copy_to_alpha = image_copy_to_alpha16;
		break;
	case 24:
	case 32:
		copy_from_alpha = image_copy_from_alpha24;
		copy_to_alpha = image_copy_to_alpha24;
		break;
	default:
		break;
	}
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

/* 16bitCGの ALPHALEVELを指定 */
BYTE *changeImage16AlphaLevel(cgdata *cg) {
	WORD *new_pic = malloc(sizeof(WORD) * cg->width * cg->height), *new_pic_;
	WORD *pic = (WORD *)cg->pic;
	int   pixels = cg->width * cg->height;
	
	new_pic_ = new_pic;

	while (pixels--) {
		*new_pic = ALPHALEVEL16(*pic, cg->alphalevel);
		new_pic++; pic++;
	}
	return (BYTE *)new_pic_;
}
