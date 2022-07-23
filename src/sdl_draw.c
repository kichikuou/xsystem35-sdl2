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
#include "font.h"
#include "ags.h"
#include "image.h"
#include "nact.h"
#include "debugger.h"

static void sdl_pal_check(void) {
	if (nact->ags.pal_changed) {
		nact->ags.pal_changed = FALSE;
		sdl_setPalette(nact->ags.pal, 0, 256);
	}
}

static Uint32 palette_color(BYTE c) {
	if (sdl_dib->format->BitsPerPixel == 8)
		return c;
	return SDL_MapRGB(sdl_dib->format, sdl_col[c].r, sdl_col[c].g, sdl_col[c].b);
}

void sdl_updateScreen(void) {
	if (!sdl_dirty)
		return;
	SDL_UpdateTexture(sdl_texture, NULL, sdl_display->pixels, sdl_display->pitch);
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
}

#ifdef __EMSCRIPTEN__
EM_JS(void, wait_vsync, (void), {
	Asyncify.handleSleep(function(wakeUp) {
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
}

/* off-screen の指定領域を Main Window へ転送 */
void sdl_updateArea(MyRectangle *src, MyPoint *dst) {
	SDL_Rect rect_d = {dst->x, dst->y, src->w, src->h};
	
	SDL_BlitSurface(sdl_dib, src, sdl_display, &rect_d);
	
	sdl_dirty = TRUE;
}

/* 全画面更新 */
void sdl_updateAll(MyRectangle *view_rect) {
	SDL_Rect rect = {0, 0, view_w, view_h};
	
	SDL_BlitSurface(sdl_dib, view_rect, sdl_display, &rect);

	sdl_dirty = TRUE;
}

/* Color の複数個指定 */
void sdl_setPalette(Palette256 *pal, int src, int cnt) {
	int i;
	
	for (i = 0; i < cnt; i++) {
		sdl_col[src + i].r = pal->red  [src + i];
		sdl_col[src + i].g = pal->green[src + i];
		sdl_col[src + i].b = pal->blue [src + i];
	}
	
	if (sdl_dib->format->BitsPerPixel == 8) {
		SDL_SetPaletteColors(sdl_dib->format->palette, sdl_col, src, cnt);
	}
}

/* 矩形の描画 */
void sdl_drawRectangle(int x, int y, int w, int h, BYTE c) {
	sdl_pal_check();
	Uint32 col = palette_color(c);

	SDL_Rect rect = {x, y, w, 1};
	SDL_FillRect(sdl_dib, &rect, col);
	
	rect = (SDL_Rect){x, y, 1, h};
	SDL_FillRect(sdl_dib, &rect, col);
	
	rect = (SDL_Rect){x, y + h - 1, w, 1};
	SDL_FillRect(sdl_dib, &rect, col);
	
	rect = (SDL_Rect){x + w - 1, y, 1, h};
	SDL_FillRect(sdl_dib, &rect, col);
}

/* 矩形塗りつぶし */
void sdl_fillRectangle(int x, int y, int w, int h, BYTE c) {
	sdl_pal_check();
	
	SDL_Rect rect = {x, y, w, h};
	SDL_FillRect(sdl_dib, &rect, palette_color(c));
}

void sdl_fillRectangleRGB(int x, int y, int w, int h, BYTE r, BYTE g, BYTE b) {
	if (sdl_dib->format->BitsPerPixel == 8)
		return;

	SDL_Rect rect = {x, y, w, h};
	SDL_FillRect(sdl_dib, &rect, SDL_MapRGB(sdl_dib->format, r, g, b));
}

void sdl_fillCircle(int left, int top, int diameter, BYTE c) {
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
				SDL_FillRect(sdl_dib, &rect, col);
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
		SDL_BlitSurface(sdl_dib, &r_src, sdl_dib, &r_dst);
	}
}

/*
 * dib に指定のパレット sp を抜いてコピー
 */
void sdl_copyAreaSP(int sx, int sy, int w, int h, int dx, int dy, BYTE sp) {
	sdl_pal_check();

	Uint32 col = sp;
	if (sdl_dib->format->BitsPerPixel > 8) {
		col = SDL_MapRGB(sdl_dib->format,
				sdl_col[sp].r & 0xf8,
				sdl_col[sp].g & 0xfc,
				sdl_col[sp].b & 0xf8);
	}
	
	SDL_SetColorKey(sdl_dib, SDL_TRUE, col);
	
	SDL_Rect r_src = {sx, sy, w, h};
	SDL_Rect r_dst = {dx, dy, w, h};
	
	SDL_BlitSurface(sdl_dib, &r_src, sdl_dib, &r_dst);
	SDL_SetColorKey(sdl_dib, SDL_FALSE, 0);
}

void sdl_drawImage8_fromData(cgdata *cg, int dx, int dy, int sprite_color) {
	int w = cg->width;
	int h = cg->height;
	SDL_Surface *s = SDL_CreateRGBSurfaceWithFormatFrom(
		cg->pic, w, h, 8, w, SDL_PIXELFORMAT_INDEX8);
	
	sdl_pal_check();
	
	if (sdl_dib->format->BitsPerPixel > 8 && cg->pal) {
		int i, i_st = 0, i_end = 256;
		SDL_Color *c = s->format->palette->colors;
		BYTE *r = cg->pal->red, *g = cg->pal->green, *b = cg->pal->blue;
		
		if (cg->type == ALCG_VSP) {
			i_st  = (cg->vsp_bank << 4);
			i_end = i_st + 16;
			c += i_st;
		}
		for (i = i_st; i < i_end; i++) {
			c->r = *(r++);
			c->g = *(g++);
			c->b = *(b++);
			c++;
		}
	} else {
		memcpy(s->format->palette->colors, sdl_col, sizeof(SDL_Color) * 256);
	}
	
	if (sprite_color != -1)
		SDL_SetColorKey(s, SDL_TRUE, sprite_color);
	
	SDL_Rect r_dst = {dx, dy, w, h};
	
	SDL_BlitSurface(s, NULL, sdl_dib, &r_dst);
	SDL_FreeSurface(s);
}

/* 直線描画 */
void sdl_drawLine(int x1, int y1, int x2, int y2, BYTE c) {
	sdl_pal_check();
	
	SDL_Renderer *renderer = SDL_CreateSoftwareRenderer(sdl_dib);
	SDL_SetRenderDrawColor(renderer, sdl_col[c].r, sdl_col[c].g, sdl_col[c].b, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
	SDL_DestroyRenderer(renderer);
}

#define TYPE ___BYTE
#include "flood.h"
#undef TYPE
#define TYPE ___WORD
#include "flood.h"
#undef TYPE
#define TYPE ___DWORD
#include "flood.h"
#undef TYPE

SDL_Rect sdl_floodFill(int x, int y, int c) {
	Uint32 col = palette_color(c);

	switch (sdl_dib->format->BytesPerPixel) {
	case 1:
		return sdl_floodFill___BYTE(x, y, col);
	case 2:
		return sdl_floodFill___WORD(x, y, col);
	case 4:
		return sdl_floodFill___DWORD(x, y, col);
	default:
		WARNING("sdl_floodFill: unsupported DIB format\n");
		return (SDL_Rect){};
	}
}

SDL_Surface *com2surface(agsurface_t *s) {
	return SDL_CreateRGBSurfaceFrom(
		s->pixel, s->width, s->height, s->depth, s->bytes_per_line, 0, 0, 0, 0);
}

int sdl_nearest_color(int r, int g, int b) {
	int i, col, mind = INT_MAX;
	for (i = 0; i < 256; i++) {
		int dr = r - sdl_col[i].r;
		int dg = g - sdl_col[i].g;
		int db = b - sdl_col[i].b;
		int d = dr*dr*30 + dg*dg*59 + db*db*11;
		if (d < mind) {
			mind = d;
			col = i;
		}
	}
	return col;
}

SDL_Rect sdl_drawString(int x, int y, const char *str_utf8, BYTE col) {
	sdl_pal_check();
	return font_draw_glyph(x, y, str_utf8, col);
}

/*
 * 指定範囲にパレット col を rate の割合で重ねる CK1
 */
void sdl_wrapColor(int sx, int sy, int w, int h, BYTE c, int rate) {
	SDL_Surface *s = SDL_CreateRGBSurface(0, w, h, sdl_dib->format->BitsPerPixel, 0, 0, 0, 0);
	assert(s->format->BitsPerPixel > 8);

	SDL_Rect r_src = {0, 0, w, h};
	SDL_FillRect(s, &r_src, palette_color(c));
	
	SDL_SetSurfaceBlendMode(s, SDL_BLENDMODE_BLEND);
	SDL_SetSurfaceAlphaMod(s, rate);
	SDL_Rect r_dst = {sx, sy, w, h};
	SDL_BlitSurface(s, &r_src, sdl_dib, &r_dst);
	SDL_FreeSurface(s);
}
