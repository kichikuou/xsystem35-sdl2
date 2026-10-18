/*
 * Copyright (C) 2026 kichikuou <KichikuouChrome@gmail.com>
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

#include "sdl_compat.h"

#include <stdlib.h>
#include <string.h>

#if XSYSTEM35_SDL_VERSION == 2

#ifdef _WIN32
#include <SDL_syswm.h>
#endif

static void flip_horizontal(SDL_Surface *s) {
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
}

static bool flip_vertical(SDL_Surface *s) {
	uint8_t *p1 = s->pixels;
	uint8_t *p2 = (uint8_t *)s->pixels + (s->h - 1) * s->pitch;
	uint8_t *tmp = malloc(s->pitch);
	if (!tmp)
		return false;
	for (int y = 0; y < s->h / 2; y++) {
		memcpy(tmp, p1, s->pitch);
		memcpy(p1, p2, s->pitch);
		memcpy(p2, tmp, s->pitch);
		p1 += s->pitch;
		p2 -= s->pitch;
	}
	free(tmp);
	return true;
}

bool sdl_FlipSurface(SDL_Surface *surface, SDL_FlipMode flip) {
	switch (flip) {
	case SDL_FLIP_HORIZONTAL:
		flip_horizontal(surface);
		return true;
	case SDL_FLIP_VERTICAL:
		return flip_vertical(surface);
	default:
		return false;
	}
}

#ifdef _WIN32
void *sdl_GetWindowHWND(SDL_Window *window) {
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if (!SDL_GetWindowWMInfo(window, &info))
		return NULL;
	return info.info.win.window;
}
#endif

#else // XSYSTEM35_SDL_VERSION == 2

bool sdl_BlitSurfaceClipped(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
	SDL_Rect sr = srcrect ? *srcrect : (SDL_Rect){0, 0, src->w, src->h};
	SDL_Rect bounds = {0, 0, src->w, src->h};
	SDL_Rect r;
	if (!SDL_GetRectIntersection(&sr, &bounds, &r)) {
		dstrect->w = dstrect->h = 0;
		return true;
	}
	SDL_Rect d = {dstrect->x + r.x - sr.x, dstrect->y + r.y - sr.y, r.w, r.h};
	SDL_Rect clip;
	SDL_GetSurfaceClipRect(dst, &clip);
	if (!SDL_GetRectIntersection(&d, &clip, dstrect)) {
		dstrect->w = dstrect->h = 0;
		return true;
	}
	sr = (SDL_Rect){r.x + dstrect->x - d.x, r.y + dstrect->y - d.y, dstrect->w, dstrect->h};
	return SDL_BlitSurface(src, &sr, dst, dstrect);
}

#endif // XSYSTEM35_SDL_VERSION == 2
