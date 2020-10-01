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

static void (*draw_line)(agsurface_t*, int, int, int, int, int); 
static void (*fill_rectangle)(agsurface_t*, int, int, int, int, int);
static void (*copy_from_alpha)(agsurface_t *, BYTE *, BYTE *, int, int, ALPHA_DIB_COPY_TYPE);
static void (*copy_to_alpha)(agsurface_t *, BYTE *, BYTE *, int, int, ALPHA_DIB_COPY_TYPE);

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

static void image_fillRectangle8(agsurface_t *dib, int x, int y, int w, int h, int col) {
	BYTE *dst = GETOFFSET_PIXEL(dib, x, y);
	int i;
	
	for (i = 0; i < h; i++) {
		memset(dst, col, w);
		dst += dib->bytes_per_line;
	}
}

/******************************************************************************/
/* private methods  image操作 15bpp                                           */
/******************************************************************************/

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

/******************************************************************************/
/* private methods  image操作 16bpp                                           */
/******************************************************************************/

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

/******************************************************************************/
/* private methods  image操作 24/32bpp                                        */
/******************************************************************************/

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
		draw_line = image_drawLine8;
		fill_rectangle = image_fillRectangle8;
		break;
	case 15:
		draw_line = image_drawLine16;
		fill_rectangle = image_fillRectangle16;
		copy_from_alpha = image_copy_from_alpha15;
		copy_to_alpha = image_copy_to_alpha15;
		break;
	case 16:
		draw_line = image_drawLine16;
		fill_rectangle = image_fillRectangle16;
		copy_from_alpha = image_copy_from_alpha16;
		copy_to_alpha = image_copy_to_alpha16;
		break;
	case 24:
	case 32:
		draw_line = image_drawLine24;
		fill_rectangle = image_fillRectangle24;
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
 * dib に線を描画
 */
void image_drawLine(agsurface_t *dib, int x0, int y0, int x1, int y1, int col) {
	draw_line(dib, x0, y0, x1, y1, col);
}

/*
 * dib に矩形塗りつぶしを描画
 */
void image_fillRectangle(agsurface_t *dib, int x, int y, int w, int h, int col) {
	fill_rectangle(dib, x, y, w, h, col);
}

void image_fillRectangleNeg(agsurface_t *dib, int x, int y, int w, int h, int col) {
	fill_rectangle(dib, x, y, w, h, -1 ^ image_index2pixel(dib->depth, col));
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

int image_index2pixel(int depth, int i) {
	Palette256 *pal = nact->sys_pal;
	
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

/* 16bitCGの ALPHALEVELを指定 */
BYTE *changeImage16AlphaLevel(cgdata *cg) {
	WORD *new_pic = malloc(sizeof(WORD) * cg->width * cg->height), *new_pic_;
	WORD *pic = (WORD *)cg->pic;
	int   pixels = cg->width * cg->height;
	
	new_pic_ = new_pic;

	while (pixels--) {
		*new_pic = RGB_ALPHALEVEL16(*pic, cg->alphalevel);
		new_pic++; pic++;
	}
	return (BYTE *)new_pic_;
}
