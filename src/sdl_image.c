/*
 * sdl_image.c  image操作 for SDL
 *
 * Copyright (C) 2000-   Fumihiko Murata <fmurata@p1.tcnet.ne.jp>
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
/* $Id: sdl_image.c,v 1.23 2003/07/20 19:30:16 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <SDL.h>

#include "portab.h"
#include "system.h"
#include "sdl_core.h"
#include "sdl_private.h"
#include "cg.h"
#include "nact.h"
#include "alpha_plane.h"
#include "image.h"

void sdl_FlipSurfaceHorizontal(SDL_Surface *s) {
#if SDL_VERSION_ATLEAST(3, 2, 0)
	SDL_FlipSurface(s, SDL_FLIP_HORIZONTAL);
#else
	uint8_t *p = s->pixels;
	int bpp = s->format->BytesPerPixel;
	uint8_t tmp[4];
	for (int y = 0; y < s->h; y++) {
		uint8_t *p1 = p;
		uint8_t *p2 = p + (s->w - 1) * bpp;
		for (int x = 0; x < s->w / 2; x++) {
			memcpy(tmp, p1, bpp);
			memcpy(p1, p2, bpp);
			memcpy(p2, tmp, bpp);
			p1 += bpp;
			p2 -= bpp;
		}
		p += s->pitch;
	}
#endif
}

void sdl_FlipSurfaceVertical(SDL_Surface *s) {
#if SDL_VERSION_ATLEAST(3, 2, 0)
	SDL_FlipSurface(s, SDL_FLIP_VERTICAL);
#else
	uint8_t *p1 = s->pixels;
	uint8_t *p2 = s->pixels + (s->h - 1) * s->pitch;
	uint8_t *tmp = malloc(s->pitch);
	for (int y = 0; y < s->h / 2; y++) {
		memcpy(tmp, p1, s->pitch);
		memcpy(p1, p2, s->pitch);
		memcpy(p2, tmp, s->pitch);
		p1 += s->pitch;
		p2 -= s->pitch;
	}
	free(tmp);
#endif
}

// Scaled copy in main_surface
void sdl_scaledCopyArea(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, int mirror) {
	SDL_Rect src_rect = {sx, sy, sw, sh};
	SDL_Rect dst_rect = {dx, dy, dw, dh};
	SDL_BlitScaled(main_surface, &src_rect, main_surface, &dst_rect);
	if (mirror) {
		SDL_IntersectRect(&dst_rect, &(SDL_Rect){0, 0, main_surface->w, main_surface->h}, &dst_rect);
		SDL_Surface *view = sdl_createSurfaceView(main_surface, dst_rect.x, dst_rect.y, dst_rect.w, dst_rect.h);
		if (mirror & 1)
			sdl_FlipSurfaceVertical(view);
		if (mirror & 2)
			sdl_FlipSurfaceHorizontal(view);
		SDL_FreeSurface(view);
	}
}


/*
 * dibに16bitCGの描画
 */
void sdl_drawImage16_fromData(cgdata *cg, int dx, int dy, int brightness, bool alpha_blend) {
	int w = cg->width;
	int h = cg->height;

	if (cg->alpha) {
		uint8_t *a_src = cg->alpha;
		uint8_t *adata = GETOFFSET_ALPHA(sdl_dibinfo, dx, dy);

		for (int i = 0; i < h; i++) {
			memcpy(adata, a_src, w);
			adata += sdl_dibinfo->width;
			a_src += cg->width;
		}
	}

	SDL_Surface *s;
	if (cg->alpha && alpha_blend) {
		uint16_t *p_src = (uint16_t *)cg->pic;
		uint8_t *a_src = cg->alpha;
		s = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_ARGB8888);
		for (int y = 0; y < h; y++) {
			uint32_t *dst = (uint32_t *)(((uint8_t *)s->pixels) + y * s->pitch);
			for (int x = 0; x < w; x++) {
				*dst++ = rgb565_to_rgb888(*p_src++) | *a_src++ << 24;
			}
		}
	} else if (sdl_dibinfo->depth > 16) {
		// Convert to RGB888 by ourselves. SDL blit functions use a slightly
		// different mapping, so the color expanded by SDL may not match the
		// color key specified in the CX command.
		uint16_t *src = (uint16_t *)cg->pic;
		s = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_RGB888);
		for (int y = 0; y < h; y++) {
			uint32_t *dst = (uint32_t *)(((uint8_t *)s->pixels) + y * s->pitch);
			for (int x = 0; x < w; x++) {
				*dst++ = rgb565_to_rgb888(*src++);
			}
		}
	} else {
		s = SDL_CreateRGBSurfaceWithFormatFrom(
			cg->pic, cg->width, cg->height, 16, cg->width * 2, SDL_PIXELFORMAT_RGB565);
	}

	if (brightness != 255)
		SDL_SetSurfaceColorMod(s, brightness, brightness, brightness);

	SDL_Rect r_dst = {dx, dy, w, h};
	SDL_BlitSurface(s, NULL, main_surface, &r_dst);
	SDL_FreeSurface(s);
}

void sdl_drawImage24_fromData(cgdata *cg, int x, int y, int brightness) {
	if (cg->alpha) {
		// This function is called only for JPEG images, so this shouldn't happen.
		WARNING("sdl_drawImage24_fromData: unsupported format");
		return;
	}

	SDL_Surface *s = SDL_CreateRGBSurfaceWithFormatFrom(
		cg->pic, cg->width, cg->height, 24, cg->width * 3, SDL_PIXELFORMAT_RGB24);
	if (brightness != 255)
		SDL_SetSurfaceColorMod(s, brightness, brightness, brightness);

	SDL_Rect r_dst = {x, y, cg->width, cg->height};
	SDL_BlitSurface(s, NULL, main_surface, &r_dst);
	SDL_FreeSurface(s);
}

SDL_Surface *sdl_dib_to_surface_with_alpha(int x, int y, int w, int h) {
	SDL_Surface *s = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_ARGB8888);

	SDL_Rect r_src = {x, y, w, h};
	SDL_BlitSurface(main_surface, &r_src, s, NULL);

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	uint8_t *p_ds = s->pixels + 3;
#else
	uint8_t *p_ds = s->pixels;
#endif
	uint8_t *adata = GETOFFSET_ALPHA(sdl_dibinfo, x, y);
	for (int y = 0; y < h; y++) {
		uint8_t *p_src = adata;
		uint8_t *p_dst = p_ds;
		for (int x = 0; x < w; x++) {
			*p_dst = *(p_src++);
			p_dst += 4;
		}
		adata += sdl_dibinfo->width;
		p_ds  += s->pitch;
	}
	return s;
}

void sdl_copyAreaSP16_shadow(int sx, int sy, int w, int h, int dx, int dy, int lv) {
	SDL_Surface *s = sdl_dib_to_surface_with_alpha(sx, sy, w, h);
	if (lv < 255)
		SDL_SetSurfaceAlphaMod(s, lv);

	SDL_Rect r_dst = {dx, dy, w, h};
	SDL_BlitSurface(s, NULL, main_surface, &r_dst);
	SDL_FreeSurface(s);
}

void sdl_copyAreaSP16_alphaBlend(int sx, int sy, int w, int h, int dx, int dy, int lv) {
	SDL_Rect r_src = {sx, sy, w, h};
	SDL_Rect r_dst = {dx, dy, w, h};
	SDL_SetSurfaceBlendMode(main_surface, SDL_BLENDMODE_BLEND);
	SDL_SetSurfaceAlphaMod(main_surface, lv);
	SDL_BlitSurface(main_surface, &r_src, main_surface, &r_dst);
	SDL_SetSurfaceAlphaMod(main_surface, 255);
	SDL_SetSurfaceBlendMode(main_surface, SDL_BLENDMODE_NONE);
}

void sdl_copyAreaSP16_alphaLevel(int sx, int sy, int w, int h, int dx, int dy, int lv) {
	SDL_Rect r_src = {sx, sy, w, h};
	SDL_Rect r_dst = {dx, dy, w, h};
	SDL_SetSurfaceBlendMode(main_surface, SDL_BLENDMODE_BLEND);
	SDL_SetSurfaceAlphaMod(main_surface, lv);
	SDL_FillRect(main_surface, &r_dst, 0);
	SDL_BlitSurface(main_surface, &r_src, main_surface, &r_dst);
	SDL_SetSurfaceAlphaMod(main_surface, 255);
	SDL_SetSurfaceBlendMode(main_surface, SDL_BLENDMODE_NONE);
}

void sdl_copy_from_alpha(int sx, int sy, int w, int h, int dx, int dy, ALPHA_DIB_COPY_TYPE flag) {
	image_copy_from_alpha(nact->ags.dib, sx, sy, w, h, dx, dy, flag);
}

void sdl_copy_to_alpha(int sx, int sy, int w, int h, int dx, int dy, ALPHA_DIB_COPY_TYPE flag) {
	image_copy_to_alpha(nact->ags.dib, sx, sy, w, h, dx, dy, flag);
}

/*
 * dib のピクセル情報を取得
 */
void sdl_getPixel(int x, int y, Palette *cell) {
	uint8_t *p = PIXEL_AT(main_surface, x, y);
	if (main_surface->format->BitsPerPixel == 8) {
		cell->pixel = *p;
	} else {
		Uint32 cl=0;
		
		switch (main_surface->format->BytesPerPixel) {
		case 2:
			{
				unsigned short *pp = (unsigned short *)p;
				cl = *pp;
			}
			break;
		case 3:
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
			cl = (p[2]<<16)+(p[1]<<8) + p[0];
#else
			cl = (p[0]<<16)+(p[1]<<8) + p[2];
#endif
			break;
		case 4:
			{
				Uint32 *pp = (Uint32 *)p;
				cl = *pp;
			}
			break;
		}
		SDL_GetRGB(cl, main_surface->format, &cell->r, &cell->g, &cell->b);
	}
}

/*
 * dib から領域の切り出し
 */
void* sdl_saveRegion(int x, int y, int w, int h) {
	SDL_Surface *s = SDL_CreateRGBSurfaceWithFormat(0, w, h, main_surface->format->BitsPerPixel, main_surface->format->format);
	if (main_surface->format->BitsPerPixel == 8)
		memcpy(s->format->palette->colors, main_surface->format->palette->colors,
		       sizeof(SDL_Color) * main_surface->format->palette->ncolors);
	SDL_Rect r_src = {x, y, w, h};
	SDL_Rect r_dst = {0, 0, w, h};
	SDL_BlitSurface(main_surface, &r_src, s, &r_dst);
	return s;
}

/*
 * セーブした領域を破棄
 */
void sdl_delRegion(void *psrc) {
	SDL_FreeSurface((SDL_Surface *)psrc);
}

/*
 * dib にセーブした領域を回復
 */
void sdl_putRegion(void *psrc, int x, int y) {
	SDL_Surface *src = (SDL_Surface *)psrc;
	
	SDL_Rect r_src = {0, 0, src->w, src->h};
	SDL_Rect r_dst = {x, y, src->w, src->h};

	SDL_BlitSurface(src, &r_src, main_surface, &r_dst);
}

/*
 * dib に dstを描画後、後始末
 */
void sdl_restoreRegion(void *psrc, int x, int y) {
	SDL_Surface *src = (SDL_Surface *)psrc;
	sdl_putRegion(src, x ,y);
	SDL_FreeSurface(src);
}
