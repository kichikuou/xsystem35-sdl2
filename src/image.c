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
#include "alpha_plane.h"
#include "ags.h"

static void (*copy_from_alpha)(surface_t *, uint8_t *, uint8_t *, int, int, ALPHA_DIB_COPY_TYPE);
static void (*copy_to_alpha)(surface_t *, uint8_t *, uint8_t *, int, int, ALPHA_DIB_COPY_TYPE);

/******************************************************************************/
/* private methods  image操作 16bpp                                           */
/******************************************************************************/

static void image_copy_from_alpha16(surface_t *dib, uint8_t *sdata, uint8_t *ddata, int w, int h, ALPHA_DIB_COPY_TYPE flag) {
	int x, y;
	uint8_t *yls;
	uint16_t *yld;
	
	switch(flag) {
	case TO_16H:
		for (y = 0; y < h; y++) {
			yls = (uint8_t *)(sdata + y * dib->width);
			yld = (uint16_t *)(ddata + y * dib->sdl_surface->pitch);
			for (x = 0; x < w; x++) {
				*yld = (uint16_t)(*yls << 8) | (*yld & 0xff);
				yld++; yls++;
			}
		}
		break;
	case TO_16L:
		for (y = 0; y < h; y++) {
			yls = (uint8_t *)(sdata + y * dib->width);
			yld = (uint16_t *)(ddata + y * dib->sdl_surface->pitch);
			for (x = 0; x < w; x++) {
				*yld = (uint16_t)(*yls) | (*yld & 0xff00);
				yld++; yls++;
			}
		}
		break;
	case TO_24R:
		for (y = 0; y < h; y++) {
			yls = (uint8_t *)(sdata + y * dib->width);
			yld = (uint16_t *)(ddata + y * dib->sdl_surface->pitch);
			for (x = 0; x < w; x++) {
				*yld = PIX16(*yls, PIXG16(*yld), PIXB16(*yld));
				yld++; yls++;
			}
		}
		break;
	case TO_24G:
		for (y = 0; y < h; y++) {
			yls = (uint8_t *)(sdata + y * dib->width);
			yld = (uint16_t *)(ddata + y * dib->sdl_surface->pitch);
			for (x = 0; x < w; x++) {
				*yld = PIX16(PIXR16(*yls), *yls, PIXB16(*yld));
				yld++; yls++;
			}
		}
		break;
	case TO_24B:
		for (y = 0; y < h; y++) {
			yls = (uint8_t *)(sdata + y * dib->width);
			yld = (uint16_t *)(ddata + y * dib->sdl_surface->pitch);
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

static void image_copy_to_alpha16(surface_t *dib, uint8_t *sdata, uint8_t *ddata, int w, int h, ALPHA_DIB_COPY_TYPE flag) {
	int x, y;
	uint8_t *yld;
	uint16_t *yls;
	
	switch(flag) {
	case FROM_16H:
		for (y = 0; y < h; y++) {
			yls = (uint16_t *)(sdata + y * dib->sdl_surface->pitch);
			yld = (uint8_t *)(ddata + y * dib->width);
			for (x = 0; x < w; x++) {
				*yld = (uint8_t)(*yls >> 8);
				yld++; yls++;
			}
		}
		break;
	case FROM_16L:
		for (y = 0; y < h; y++) {
			yls = (uint16_t *)(sdata + y * dib->sdl_surface->pitch);
			yld = (uint8_t *)(ddata + y * dib->width);
			for (x = 0; x < w; x++) {
				*yld = (uint8_t)(*yls);
				yld++; yls++;
			}
		}
		break;
	case FROM_24R:
		for (y = 0; y < h; y++) {
			yls = (uint16_t *)(sdata + y * dib->sdl_surface->pitch);
			yld = (uint8_t *)(ddata + y * dib->width);
			for (x = 0; x < w; x++) {
				*yld = PIXR16(*yls);
				yld++; yls++;
			}
		}
		break;
	case FROM_24G:
		for (y = 0; y < h; y++) {
			yls = (uint16_t *)(sdata + y * dib->sdl_surface->pitch);
			yld = (uint8_t *)(ddata + y * dib->width);
			for (x = 0; x < w; x++) {
				*yld = PIXG16(*yls);
				yld++; yls++;
			}
		}
		break;
	case FROM_24B:
		for (y = 0; y < h; y++) {
			yls = (uint16_t *)(sdata + y * dib->sdl_surface->pitch);
			yld = (uint8_t *)(ddata + y * dib->width);
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

static void image_copy_from_alpha24(surface_t *dib, uint8_t *sdata, uint8_t *ddata, int w, int h, ALPHA_DIB_COPY_TYPE flag) {
	int x, y;
	uint8_t *yls;
	uint32_t *yld;
	
	switch(flag) {
	case TO_16H:
		for (y = 0; y < h; y++) {
			yls = (uint8_t *)(sdata + y * dib->width);
			yld = (uint32_t *)(ddata + y * dib->sdl_surface->pitch);
			for (x = 0; x < w; x++) {
				/*  *yld = */ 
				yld++; yls++;
			}
		}
		break;
	case TO_16L:
		for (y = 0; y < h; y++) {
			yls = (uint8_t *)(sdata + y * dib->width);
			yld = (uint32_t *)(ddata + y * dib->sdl_surface->pitch);
			for (x = 0; x < w; x++) {
				/* *yld = (uint16_t)(*yls) | (*yld & 0xff00); */
				yld++; yls++;
			}
		}
		break;
	case TO_24R:
		for (y = 0; y < h; y++) {
			yls = (uint8_t *)(sdata + y * dib->width);
			yld = (uint32_t *)(ddata + y * dib->sdl_surface->pitch);
			for (x = 0; x < w; x++) {
				*yld = PIX24(*yls, PIXG24(*yld), PIXB24(*yld));
				yld++; yls++;
			}
		}
		break;
	case TO_24G:
		for (y = 0; y < h; y++) {
			yls = (uint8_t *)(sdata + y * dib->width);
			yld = (uint32_t *)(ddata + y * dib->sdl_surface->pitch);
			for (x = 0; x < w; x++) {
				*yld = PIX24(PIXR24(*yls), *yls, PIXB24(*yld));
				yld++; yls++;
			}
		}
		break;
	case TO_24B:
		for (y = 0; y < h; y++) {
			yls = (uint8_t *)(sdata + y * dib->width);
			yld = (uint32_t *)(ddata + y * dib->sdl_surface->pitch);
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

static void image_copy_to_alpha24(surface_t *dib, uint8_t *sdata, uint8_t *ddata, int w, int h, ALPHA_DIB_COPY_TYPE flag) {
	int x, y;
	uint8_t *yld;
	uint32_t *yls;
	
	switch(flag) {
	case FROM_16H:
		for (y = 0; y < h; y++) {
			yls = (uint32_t *)(sdata + y * dib->sdl_surface->pitch);
			yld = (uint8_t  *)(ddata + y * dib->width);
			for (x = 0; x < w; x++) {
				*yld = (uint8_t)(PIX16(PIXR24(*yls), PIXG24(*yls), PIXB24(*yls)) >> 8);
				yld++; yls++;
			}
		}
		break;
	case FROM_16L:
		for (y = 0; y < h; y++) {
			yls = (uint32_t *)(sdata + y * dib->sdl_surface->pitch);
			yld = (uint8_t  *)(ddata + y * dib->width);
			for (x = 0; x < w; x++) {
				*yld = (uint8_t)(PIX16(PIXR24(*yls), PIXG24(*yls), PIXB24(*yls)));
					yld++; yls++;
			}
		}
		break;
	case FROM_24R:
		for (y = 0; y < h; y++) {
			yls = (uint32_t *)(sdata + y * dib->sdl_surface->pitch);
			yld = (uint8_t  *)(ddata + y * dib->width);
			for (x = 0; x < w; x++) {
				*yld = (uint8_t)PIXR24(*yls);
				yld++; yls++;
			}
		}
		break;
	case FROM_24G:
		for (y = 0; y < h; y++) {
			yls = (uint32_t *)(sdata + y * dib->sdl_surface->pitch);
			yld = (uint8_t  *)(ddata + y * dib->width);
			for (x = 0; x < w; x++) {
				*yld = (uint8_t)PIXG24(*yls);
				yld++; yls++;
			}
		}
		break;
	case FROM_24B:
		for (y = 0; y < h; y++) {
			yls = (uint32_t *)(sdata + y * dib->sdl_surface->pitch);
			yld = (uint8_t  *)(ddata + y * dib->width);
			for (x = 0; x < w; x++) {
				*yld = (uint8_t)PIXB24(*yls);
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

void image_copy_from_alpha(surface_t *dib, int sx, int sy, int w, int h, int dx, int dy, ALPHA_DIB_COPY_TYPE flag) {
	uint8_t *sdata = GETOFFSET_ALPHA(dib, sx, sy);
	uint8_t *ddata = GETOFFSET_PIXEL(dib, dx, dy);
	
	copy_from_alpha(dib, sdata, ddata, w, h, flag);
}

void image_copy_to_alpha(surface_t *dib, int sx, int sy, int w, int h, int dx, int dy, ALPHA_DIB_COPY_TYPE flag) {
	uint8_t *sdata = GETOFFSET_PIXEL(dib, sx, sy);
	uint8_t *ddata = GETOFFSET_ALPHA(dib, dx, dy);
	
	copy_to_alpha(dib, sdata, ddata, w, h, flag);
}
