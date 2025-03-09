/*
 * sdl_darw.c  SDL draw to surface
 *
 * Copyright (C) 2000-     Fumihiko Murata       <fmurata@p1.tcnet.ne.jp>
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
/* $Id: sdl_draw.c,v 1.13 2003/01/25 01:34:50 chikama Exp $ */

#include "config.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <SDL.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "portab.h"
#include "system.h"
#include "sdl_core.h"
#include "sdl_private.h"
#include "scheduler.h"
#include "font.h"
#include "ags.h"
#include "image.h"
#include "nact.h"
#include "debugger.h"

static void sdl_pal_check(void) {
	if (nact->ags.pal_changed) {
		nact->ags.pal_changed = false;
		sdl_setPalette(nact->ags.pal, 0, 256);
	}
}

static Uint32 palette_color(uint8_t c) {
	if (main_surface->format->BitsPerPixel == 8)
		return c;
	return SDL_MapRGB(main_surface->format, sdl_palette->colors[c].r, sdl_palette->colors[c].g, sdl_palette->colors[c].b);
}

void sdl_updateScreen(void) {
	if (!sdl_dirty)
		return;
	SDL_RenderClear(sdl_renderer);
	SDL_RenderCopy(sdl_renderer, sdl_texture, NULL, NULL);
	SDL_RenderPresent(sdl_renderer);
	sdl_dirty = false;
}

uint32_t sdl_getTicks(void) {
	return SDL_GetTicks();
}

void sdl_sleep(int msec) {
	sdl_updateScreen();
	dbg_onsleep();
#ifdef __EMSCRIPTEN__
	emscripten_sleep(msec);
#else
	SDL_Delay(msec);
#endif
	scheduler_on_event(SCHEDULER_EVENT_SLEEP);
}

#ifdef __EMSCRIPTEN__
EM_JS(void, wait_vsync, (void), {
	// We need a `return` here for JSPI support (ASYNCIFY=2).
	return Asyncify.handleSleep(function(wakeUp) {
		window.requestAnimationFrame(function() {
			wakeUp();
		});
	});
});
#endif

void sdl_wait_vsync() {
	sdl_updateScreen();
	dbg_onsleep();
#ifdef __EMSCRIPTEN__
	wait_vsync();
#else
	SDL_Delay(16);
#endif
	scheduler_on_event(SCHEDULER_EVENT_SLEEP);
}

/* off-screen の指定領域を Main Window へ転送 */
void sdl_updateArea(MyRectangle *rect, MyPoint *dst) {
	SDL_Rect sw = {0, 0, main_surface->w, main_surface->h};
	SDL_Rect dw = {0, 0, view_w, view_h};
	SDL_Rect sr = {rect->x, rect->y, rect->w, rect->h};
	SDL_Rect dr = {dst->x, dst->y, rect->w, rect->h};
	if (!ags_clipCopyRect(&sw, &dw, &sr.x, &sr.y, &dr.x, &dr.y, &sr.w, &sr.h))
		return;
	dr.w = sr.w;
	dr.h = sr.h;

	SDL_Surface *sf;
	SDL_LockTextureToSurface(sdl_texture, &dr, &sf);
	SDL_BlitSurface(main_surface, &sr, sf, NULL);
	SDL_UnlockTexture(sdl_texture);

	sdl_dirty = true;
}

/* 全画面更新 */
void sdl_updateAll(MyRectangle *view_rect) {
	sdl_updateArea(view_rect, &(MyPoint){0, 0});
}

/* Color の複数個指定 */
void sdl_setPalette(Color *pal, int first, int count) {
	SDL_Color colors[256];
	for (int i = 0; i < count; i++) {
		colors[i].r = pal[first + i].r;
		colors[i].g = pal[first + i].g;
		colors[i].b = pal[first + i].b;
		colors[i].a = 255;
	}
	SDL_SetPaletteColors(sdl_palette, colors, first, count);
}

/* 矩形の描画 */
void sdl_drawRectangle(int x, int y, int w, int h, uint8_t c) {
	sdl_pal_check();
	Uint32 col = palette_color(c);

	SDL_Rect rect = {x, y, w, 1};
	SDL_FillRect(main_surface, &rect, col);
	
	rect = (SDL_Rect){x, y, 1, h};
	SDL_FillRect(main_surface, &rect, col);
	
	rect = (SDL_Rect){x, y + h - 1, w, 1};
	SDL_FillRect(main_surface, &rect, col);
	
	rect = (SDL_Rect){x + w - 1, y, 1, h};
	SDL_FillRect(main_surface, &rect, col);
}

/* 矩形塗りつぶし */
void sdl_fillRectangle(int x, int y, int w, int h, uint8_t c) {
	sdl_pal_check();
	
	SDL_Rect rect = {x, y, w, h};
	SDL_FillRect(main_surface, &rect, palette_color(c));
}

void sdl_fillRectangleRGB(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b) {
	if (main_surface->format->BitsPerPixel == 8)
		return;

	SDL_Rect rect = {x, y, w, h};
	SDL_FillRect(main_surface, &rect, SDL_MapRGB(main_surface->format, r, g, b));
}

void sdl_fillCircle(int left, int top, int diameter, uint8_t c) {
	diameter &= ~1;
	if (diameter <= 0)
		return;

	Uint32 col = palette_color(c);

	// This draws a circle that is pixel-identical to System3.9's grDrawFillCircle.
	for (int y = 0; y < diameter; y++) {
		int dy = diameter - 2*y;
		int dx = diameter - 1;
		for (int x = 0; 2*x < diameter; x++) {
			if (dy*dy + dx*dx <= diameter*diameter) {
				SDL_Rect rect = {left + x, top + y, diameter - 2*x, 1};
				SDL_FillRect(main_surface, &rect, col);
				break;
			}
			dx -= 2;
		}
	}
}

/* 領域コピー */
void sdl_copyArea(int sx, int sy, int w, int h, int dx, int dy) {
	if (sx == dx && sy == dy)
		return;

	SDL_Rect r_src = {sx, sy, w, h};
	SDL_Rect r_dst = {dx, dy, w, h};

	SDL_Rect intersect;
	if (SDL_IntersectRect(&r_src, &r_dst, &intersect)) {
		void* region = sdl_saveRegion(sx, sy, w, h);
		sdl_restoreRegion(region, dx, dy);
	} else {
		SDL_BlitSurface(main_surface, &r_src, main_surface, &r_dst);
	}
}

static void sdl_dib_sprite_copy(SDL_Surface *dst, int sx, int sy, int w, int h, int dx, int dy, uint8_t sp) {
	sdl_pal_check();

	Uint32 col = palette_color(sp);
	SDL_SetColorKey(main_surface, SDL_TRUE, col);
	
	SDL_Rect r_src = {sx, sy, w, h};
	SDL_Rect r_dst = {dx, dy, w, h};
	
	SDL_BlitSurface(main_surface, &r_src, dst, &r_dst);
	SDL_SetColorKey(main_surface, SDL_FALSE, 0);
}

/*
 * dib に指定のパレット sp を抜いてコピー
 */
void sdl_copyAreaSP(int sx, int sy, int w, int h, int dx, int dy, uint8_t sp) {
	sdl_dib_sprite_copy(main_surface, sx, sy, w, h, dx, dy, sp);
}

SDL_Surface *sdl_dib_to_surface_colorkey(int x, int y, int w, int h, int sp) {
	SDL_Surface *s = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_ARGB8888);
	SDL_FillRect(s, NULL, 0);
	sdl_dib_sprite_copy(s, x, y, w, h, 0, 0, sp);
	return s;
}

void sdl_drawImage8(cgdata *cg, int dx, int dy, int sprite_color) {
	int w = cg->width;
	int h = cg->height;
	SDL_Surface *s = SDL_CreateRGBSurfaceWithFormatFrom(
		cg->pic, w, h, 8, w, SDL_PIXELFORMAT_INDEX8);
	
	sdl_pal_check();
	
	if (main_surface->format->BitsPerPixel > 8 && cg->pal) {
		SDL_Color *c = s->format->palette->colors;
		for (int i = 0; i < 256; i++) {
			c->r = cg->pal[i].r;
			c->g = cg->pal[i].g;
			c->b = cg->pal[i].b;
			c++;
		}
	} else {
		SDL_SetSurfacePalette(s, sdl_palette);
	}
	
	if (sprite_color != -1)
		SDL_SetColorKey(s, SDL_TRUE, sprite_color);
	
	SDL_Rect r_dst = {dx, dy, w, h};
	
	SDL_BlitSurface(s, NULL, main_surface, &r_dst);
	SDL_FreeSurface(s);
}

/* 直線描画 */
void sdl_drawLine(int x1, int y1, int x2, int y2, uint8_t c) {
	sdl_pal_check();
	
	SDL_Renderer *renderer = SDL_CreateSoftwareRenderer(main_surface);
	SDL_SetRenderDrawColor(renderer, sdl_palette->colors[c].r, sdl_palette->colors[c].g, sdl_palette->colors[c].b, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
	SDL_DestroyRenderer(renderer);
}

#define TYPE uint8_t
#include "flood.h"
#undef TYPE
#define TYPE uint16_t
#include "flood.h"
#undef TYPE
#define TYPE uint32_t
#include "flood.h"
#undef TYPE

SDL_Rect sdl_floodFill(int x, int y, int c) {
	Uint32 col = palette_color(c);

	switch (main_surface->format->BytesPerPixel) {
	case 1:
		return sdl_floodFill_uint8_t(x, y, col);
	case 2:
		return sdl_floodFill_uint16_t(x, y, col);
	case 4:
		return sdl_floodFill_uint32_t(x, y, col);
	default:
		WARNING("sdl_floodFill: unsupported DIB format");
		return (SDL_Rect){};
	}
}

int sdl_nearest_color(int r, int g, int b) {
	int i, col, mind = INT_MAX;
	for (i = 0; i < 256; i++) {
		int dr = r - sdl_palette->colors[i].r;
		int dg = g - sdl_palette->colors[i].g;
		int db = b - sdl_palette->colors[i].b;
		int d = dr*dr*30 + dg*dg*59 + db*db*11;
		if (d < mind) {
			mind = d;
			col = i;
		}
	}
	return col;
}

SDL_Rect sdl_drawString(int x, int y, const char *str_utf8, uint8_t col) {
	sdl_pal_check();
	return font_draw_glyph(x, y, str_utf8, col);
}

/*
 * 指定範囲にパレット col を rate の割合で重ねる CK1
 */
void sdl_wrapColor(int sx, int sy, int w, int h, uint8_t c, int rate) {
	SDL_Surface *s = SDL_CreateRGBSurfaceWithFormat(0, w, h, main_surface->format->BitsPerPixel, main_surface->format->format);
	assert(s->format->BitsPerPixel > 8);

	SDL_Rect r_src = {0, 0, w, h};
	SDL_FillRect(s, &r_src, palette_color(c));
	
	SDL_SetSurfaceBlendMode(s, SDL_BLENDMODE_BLEND);
	SDL_SetSurfaceAlphaMod(s, rate);
	SDL_Rect r_dst = {sx, sy, w, h};
	SDL_BlitSurface(s, &r_src, main_surface, &r_dst);
	SDL_FreeSurface(s);
}
