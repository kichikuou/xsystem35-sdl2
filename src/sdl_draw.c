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

static const int fadeX[16] = {0,2,2,0,1,3,3,1,1,3,3,1,0,2,2,0};
static const int fadeY[16] = {0,2,0,2,1,3,1,3,0,2,0,2,1,3,1,3};

static int fadestep[256] =
{0,1,3,4,6,7,9,10,12,14,15,17,18,20,21,23,25,26,28,29,31,32,34,36,37,39,40,
 42,43,45,46,48,49,51,53,54,56,57,59,60,62,63,65,66,68,69,71,72,74,75,77,78,
 80,81,83,84,86,87,89,90,92,93,95,96,97,99,100,102,103,105,106,108,109,110,
 112,113,115,116,117,119,120,122,123,124,126,127,128,130,131,132,134,135,136,
 138,139,140,142,143,144,146,147,148,149,151,152,153,155,156,157,158,159,161,
 162,163,164,166,167,168,169,170,171,173,174,175,176,177,178,179,181,182,183,
 184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,
 203,204,205,206,207,208,209,210,211,211,212,213,214,215,216,217,217,218,219,
 220,221,221,222,223,224,225,225,226,227,227,228,229,230,230,231,232,232,233,
 234,234,235,235,236,237,237,238,238,239,239,240,241,241,242,242,243,243,244,
 244,244,245,245,246,246,247,247,247,248,248,249,249,249,250,250,250,251,251,
 251,251,252,252,252,252,253,253,253,253,254,254,254,254,254,254,255,255,255,
 255,255,255,255,255,255,255,255,255,255,255};

static void sdl_pal_check(void) {
	if (nact->sys_pal_changed) {
		nact->sys_pal_changed = FALSE;
		sdl_setPalette(nact->sys_pal, 0, 256);
	}
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
static void sdl_updateAll() {
	SDL_Rect rect = {0, 0, view_w, view_h};
	
	SDL_BlitSurface(sdl_dib, &sdl_view, sdl_display, &rect);

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

	Uint32 col = (sdl_dib->format->BitsPerPixel == 8) ? c
		: SDL_MapRGB(sdl_dib->format, sdl_col[c].r, sdl_col[c].g, sdl_col[c].b);

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

	Uint32 col = (sdl_dib->format->BitsPerPixel == 8) ? c
		: SDL_MapRGB(sdl_dib->format, sdl_col[c].r, sdl_col[c].g, sdl_col[c].b);
	
	SDL_FillRect(sdl_dib, &rect, col);
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

void sdl_drawImage8_fromData(cgdata *cg, int dx, int dy, int w, int h) {
	SDL_Surface *s = SDL_CreateRGBSurface(0, w, h, 8, 0, 0, 0, 0);
	SDL_LockSurface(s);

#if 0  /* for broken cg */
	if (s->pitch == s->w) {
		memcpy(s->pixels, cg->pic, w * h);
	} else 
#endif
	{
		int i = h;
		BYTE *p_src = (cg->pic + cg->data_offset), *p_dst = s->pixels;
		
		while (i--) {
			memcpy(p_dst, p_src, w);
			p_dst += s->pitch;
			p_src += cg->width;
		}
	}
	
	SDL_UnlockSurface(s);
	
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
	
	if (cg->spritecolor != -1) {
		SDL_SetColorKey(s, SDL_TRUE, cg->spritecolor);
	}
	
	SDL_Rect r_src = { 0,  0, w, h};
	SDL_Rect r_dst = {dx, dy, w, h};
	
	SDL_BlitSurface(s, &r_src, sdl_dib, &r_dst);
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
	Uint32 col = (sdl_dib->format->BitsPerPixel == 8) ? c
		: SDL_MapRGB(sdl_dib->format, sdl_col[c].r, sdl_col[c].g, sdl_col[c].b);

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

void sdl_Mosaic(int sx, int sy, int w, int h, int dx, int dy, int slice) {
	
	SDL_LockSurface(sdl_dib);

	image_Mosaic(sdl_dib, sx, sy, w, h, dx, dy, slice);

	SDL_UnlockSurface(sdl_dib);
}

static void setBligtness(SDL_Surface *s, int val) {
	int i;
	Palette256 *pal = nact->sys_pal;
	Uint8 *r = pal->red, *g = pal->green, *b = pal->blue;
	SDL_Color *cl = sdl_col;
	
	for (i = 0; i < 256; i++) {
		cl->r = (val * (*(r++))) / 255;
		cl->g = (val * (*(g++))) / 255;
		cl->b = (val * (*(b++))) / 255;
		cl++;
	}
	SDL_SetPaletteColors(s->format->palette, sdl_col, 0, 256);
}

static void setWhiteness(SDL_Surface *s, int val) {
	int i;
	Palette256 *pal = nact->sys_pal;
	Uint8 *r = pal->red, *g = pal->green, *b = pal->blue;
	SDL_Color *cl = sdl_col;
	
	for (i = 0; i < 256; i++) {
		cl->r = (((255- *r) * val) / 256) + *r; r++;
		cl->g = (((255- *g) * val) / 256) + *g; g++;
		cl->b = (((255- *b) * val) / 256) + *b; b++;
		cl++;
	}
	SDL_SetPaletteColors(s->format->palette, sdl_col, 0, 256);
}

static void fader_in(int n) {
	static SDL_Surface *src;

	if (n == 0) {
		src = SDL_CreateRGBSurfaceWithFormat(0, sdl_display->w, sdl_display->h, 32, SDL_PIXELFORMAT_RGB888);
		SDL_Rect r_src = {view_x, view_y, view_w, view_h};
		SDL_Rect r_dst = {0, 0, view_w, view_h};
		SDL_BlitSurface(sdl_dib, &r_src, src, &r_dst);
	}
	
	if (n == 255) {
		SDL_FreeSurface(src);
		sdl_updateAll();
		return;
	}

	int lv = n / 16;
	for (int y = 0; y < src->h; y += 4) {
		DWORD *yls = PIXEL_AT(src, 0, y + fadeY[lv]);
		DWORD *yld = PIXEL_AT(sdl_display, 0, y + fadeY[lv]);
		for (int x = 0; x < src->w; x += 4) {
			*(yld + fadeX[lv]) = *(yls + fadeX[lv]);
			yls += 4; yld += 4;
		}
	}

	sdl_dirty = TRUE;
}

static void fader_out(int n, Uint32 c) {
	if (n == 255) {
		SDL_FillRect(sdl_display, NULL, c);
		return;
	}

	int lv = (255 - n) / 16;
	for (int y = 0; y < sdl_display->h; y += 4) {
		DWORD *yld = PIXEL_AT(sdl_display, 0, y + fadeY[lv]);
		for (int x = 0; x < sdl_display->w; x += 4) {
			*(yld + fadeX[lv]) = c;
			yld += 4;
		}
	}

	sdl_dirty = TRUE;
}

static __inline void sdl_fade_blit(void) {
	SDL_Rect r_dst = {0, 0, view_w, view_h};

	SDL_BlitSurface(sdl_dib, &sdl_view, sdl_display, &r_dst);
	sdl_dirty = TRUE;
}

void sdl_fadeIn(int step) {
	if (sdl_dib->format->BitsPerPixel == 8) {
		setBligtness(sdl_dib, fadestep[step]);
		sdl_fade_blit();
	} else {
		fader_in(step);
	}
}

void sdl_fadeOut(int step) {
	if (sdl_dib->format->BitsPerPixel == 8) {
		setBligtness(sdl_dib, fadestep[255 - step]);
		sdl_fade_blit();
	} else {
		fader_out(step, SDL_MapRGB(sdl_display->format, 0, 0, 0));
	}
}

void sdl_whiteIn(int step) {
	if (sdl_dib->format->BitsPerPixel == 8) {
		setWhiteness(sdl_dib, fadestep[255 - step]); /* ??? */
		sdl_fade_blit();
	} else {
		fader_in(step);
	}
}

void sdl_whiteOut(int step) {
	if (sdl_dib->format->BitsPerPixel == 8) {
		setWhiteness(sdl_dib, fadestep[step]); /* ??? */
		sdl_fade_blit();
	} else {
		fader_out(step, SDL_MapRGB(sdl_display->format, 255, 255, 255));
	}
}

/*
 * 指定範囲にパレット col を rate の割合で重ねる CK1
 */
void sdl_wrapColor(int sx, int sy, int w, int h, BYTE c, int rate) {
	SDL_Surface *s = SDL_CreateRGBSurface(0, w, h, sdl_dib->format->BitsPerPixel, 0, 0, 0, 0);
	assert(s->format->BitsPerPixel > 8);

	Uint32 col = SDL_MapRGB(sdl_dib->format, sdl_col[c].r, sdl_col[c].g, sdl_col[c].b);

	SDL_Rect r_src = {0, 0, w, h};
	SDL_FillRect(s, &r_src, col);
	
	SDL_SetSurfaceBlendMode(s, SDL_BLENDMODE_BLEND);
	SDL_SetSurfaceAlphaMod(s, rate);
	SDL_Rect r_dst = {sx, sy, w, h};
	SDL_BlitSurface(s, &r_src, sdl_dib, &r_dst);
	SDL_FreeSurface(s);
}
