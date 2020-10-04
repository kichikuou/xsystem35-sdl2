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

/* fader */
static const int fadeX[16] = {0,2,2,0,1,3,3,1,1,3,3,1,0,2,2,0};
static const int fadeY[16] = {0,2,0,2,1,3,1,3,0,2,0,2,1,3,1,3};

static void (*copy_from_alpha)(agsurface_t *, BYTE *, BYTE *, int, int, ALPHA_DIB_COPY_TYPE);
static void (*copy_to_alpha)(agsurface_t *, BYTE *, BYTE *, int, int, ALPHA_DIB_COPY_TYPE);

/******************************************************************************/
/* private methods  image操作 16bpp                                           */
/******************************************************************************/

static void fadeOut16(SDL_Surface *dst, int lv, int col) {
	WORD *yld;
	int x, y;
	
	for (y = 0; y < dst->h; y += 4) {
		yld = PIXEL_AT(dst, 0, y + fadeY[lv]);
		for (x = 0; x < dst->w; x += 4) {
			*(yld + fadeX[lv]) = col;
			yld += 4;
		}
	}
}

static void fadeIn16(SDL_Surface *src, SDL_Surface *dst, int lv) {
	WORD *yls, *yld;
	int x, y;
	
	for (y = 0; y < src->h; y += 4) {
		yls = PIXEL_AT(src, 0, y + fadeY[lv]);
		yld = PIXEL_AT(dst, 0, y + fadeY[lv]);
		for (x = 0; x < src->w; x += 4) {
			*(yld + fadeX[lv]) = *(yls + fadeX[lv]);
			yls += 4; yld += 4;
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

/******************************************************************************/
/* private methods  image操作 packed 24bpp                                    */
/******************************************************************************/

static void fadeOut24p(SDL_Surface *dst, int lv, int col) {
	BYTE *yld;
	int x, y;
	
	for (y = 0; y < dst->h; y += 4) {
		yld = PIXEL_AT(dst, 0, y + fadeY[lv]);
		for (x = 0; x < dst->w; x += 4) {
			*(yld + fadeX[lv])    = 
			*(yld + fadeX[lv] +1) =
			*(yld + fadeX[lv] +2) = (BYTE)col;
			yld += (4*3);
		}
	}
}

static void fadeIn24p(SDL_Surface *src, SDL_Surface *dst, int lv) {
	DWORD *yls;
	BYTE  *yld;
	int x, y;
	
	for (y = 0; y < src->h; y += 4) {
		yls = PIXEL_AT(src, 0, y + fadeY[lv]);
		yld = PIXEL_AT(dst, 0, y + fadeY[lv]);
		for (x = 0; x < src->w; x += 4) {
			*(yld + fadeX[lv]   ) = PIXB24(*(yls + fadeX[lv]));
			*(yld + fadeX[lv] +1) = PIXG24(*(yls + fadeX[lv]));
			*(yld + fadeX[lv] +2) = PIXR24(*(yls + fadeX[lv]));
			yls += 4; yld += (4*3);
		}
	}
}

/******************************************************************************/
/* private methods  image操作 24/32bpp                                        */
/******************************************************************************/

static void fadeOut24(SDL_Surface *dst, int lv, int col) {
	DWORD *yld;
	int x, y;
	
	for (y = 0; y < dst->h; y += 4) {
		yld = PIXEL_AT(dst, 0, y + fadeY[lv]);
		for (x = 0; x < dst->w; x += 4) {
			*(yld + fadeX[lv]) = col;
			yld += 4;
		}
	}
}

static void fadeIn24(SDL_Surface *src, SDL_Surface *dst, int lv) {
	DWORD *yls, *yld;
	int x, y;
	
	for (y = 0; y < src->h; y += 4) {
		yls = PIXEL_AT(src, 0, y + fadeY[lv]);
		yld = PIXEL_AT(dst, 0, y + fadeY[lv]);
		for (x = 0; x < src->w; x += 4) {
			*(yld + fadeX[lv]) = *(yls + fadeX[lv]);
			yls += 4; yld += 4;
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

/*
   fade out for 16/24/32
*/
void image_fadeOut(SDL_Surface *img, int lv, int col) {
	switch(img->format->BytesPerPixel) {
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
void image_fadeIn(SDL_Surface *src, SDL_Surface *dst, int lv) {
	switch(dst->format->BytesPerPixel) {
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

/* モザイク */

void image_Mosaic(SDL_Surface *dib, int sx, int sy, int w, int h, int dx, int dy, int slice) {
	SDL_Rect r = {.h = slice};
	for (int y = 0; y < h; y += slice) {
		if (y + slice > h)
			r.h = h - y;
		r.w = slice;
		for (int x = 0; x < w; x += slice) {
			void *src = PIXEL_AT(dib, sx + x, sy + y);
			Uint32 color;
			switch (dib->format->BytesPerPixel) {
			case 1: color = *(BYTE *)src; break;
			case 2: color = *(WORD *)src; break;
			default: color = *(DWORD *)src; break;
			}
			r.x = dx + x;
			r.y = dy + y;
			if (r.x + slice > w)
				r.w = w - x;
			SDL_FillRect(dib, &r, color);
		}
	}
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
