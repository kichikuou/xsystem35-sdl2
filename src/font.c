/*
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

#include <limits.h>
#include <math.h>
#include <SDL.h>
#include <SDL_ttf.h>

#ifdef _WIN32
#include "win/resources.h"
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "portab.h"
#include "system.h"
#include "font.h"
#include "sdl_private.h"
#include "hacks.h"

typedef struct {
	int      size;
	FontType type;
	TTF_Font *id;
} FontTable;

#define FONTTABLEMAX 256
static FontTable fonttbl[FONTTABLEMAX];
static int       fontcnt = 0;

static FontTable *fontset;

static struct {
	bool antialiase_on;
	const char *name[FONTTYPEMAX];
	int face[FONTTYPEMAX];
} this;

static void font_insert(int size, FontType type, TTF_Font *font) {
	fonttbl[fontcnt].size = size;
	fonttbl[fontcnt].type = type;
	fonttbl[fontcnt].id   = font;
	
	if (fontcnt >= (FONTTABLEMAX -1)) {
		WARNING("Font table is full.");
	} else {
		fontcnt++;
	}
}

static FontTable *font_lookup(int size, FontType type) {
	int i;
	
	for (i = 0; i < fontcnt; i++) {
		if (fonttbl[i].size == size && fonttbl[i].type == type) { 
			return &fonttbl[i];
		}
	}
	return NULL;
}

void font_select(FontType type, int size, int weight) {
	if (type >= FONTTYPEMAX) {
		WARNING("Invalid font type %d", type);
		return;
	}

	FontTable *tbl;
	if (NULL == (tbl = font_lookup(size, type))) {
		TTF_Font *fs = TTF_OpenFontIndex(this.name[type], size, this.face[type]);
#ifdef __ANDROID__
		// If `this.name[type]` is a custom font file specified in .xys35rc,
		// SDL_RWFromFile used by TTF_OpenFontIndex does not work because it
		// does not resolve relative path with the current directory. On the
		// other hand, we can't just use fopen because SDL_RWFromFile can open
		// apk assets and the default fonts are stored as assets.
		if (!fs) {
			FILE *fp = fopen(this.name[type], "r");
			if (fp)
				fs = TTF_OpenFontIndexRW(SDL_RWFromFP(fp, true), true, size, this.face[type]);
		}
#endif
#ifdef _WIN32
		SDL_RWops *r = open_resource(this.name[type], "fonts");
		if (r)
			fs = TTF_OpenFontIndexRW(r, true, size, this.face[type]);
#endif
		if (!fs)
			SYSERROR("Cannot open font %s", this.name[type]);
		
		font_insert(size, type, fs);
		fontset = &fonttbl[fontcnt - 1];
	} else {
		fontset = tbl;
	}
	TTF_SetFontStyle(fontset->id, weight > 5 ? TTF_STYLE_BOLD : TTF_STYLE_NORMAL);
}

SDL_Surface *font_get_glyph(const char *str_utf8, int r, int g, int b) {
	if (!fontset)
		return NULL;

	SDL_Surface *fs;
	SDL_Color color = {r, g, b, 255};
	fs = TTF_RenderUTF8_Blended(fontset->id, str_utf8, color);
	if (!fs)
		WARNING("Text rendering failed: %s", TTF_GetError());

	return fs;
}

// SDL can't blit ARGB to an indexed bitmap properly, so we do it ourselves.
static void sdl_drawAntiAlias_8bpp(int dstx, int dsty, SDL_Surface *src, uint8_t col)
{
	Uint8 cache[256*7];
	memset(cache, 0, 256);

	for (int y = 0; y < src->h && dsty + y < main_surface->h; y++) {
		if (dsty + y < 0)
			continue;
		uint8_t *sp = (uint8_t*)src->pixels + y * src->pitch;
		uint8_t *dp = (uint8_t*)main_surface->pixels + (dsty + y) * main_surface->pitch + dstx;
		for (int x = 0; x < src->w && dstx + x < main_surface->w; x++) {
			Uint8 r, g, b, alpha;
			SDL_GetRGBA(*((Uint32*)sp), src->format, &r, &g, &b, &alpha);
			alpha = alpha >> 5; // reduce bit depth
			if (!alpha) {
				// Transparent, do nothing
			} else if (alpha == 7) {
				*dp = col; // Fully opaque
			} else if (cache[*dp] & 1 << alpha) {
				*dp = cache[alpha << 8 | *dp]; // use cached value
			} else {
				// find nearest color in palette
				cache[*dp] |= 1 << alpha;
				int c = sdl_nearest_color(
					(sdl_col[col].r * alpha + sdl_col[*dp].r * (7 - alpha)) / 7,
					(sdl_col[col].g * alpha + sdl_col[*dp].g * (7 - alpha)) / 7,
					(sdl_col[col].b * alpha + sdl_col[*dp].b * (7 - alpha)) / 7);
				cache[alpha << 8 | *dp] = c;
				*dp = c;
			}
			sp += src->format->BytesPerPixel;
			dp++;
		}
	}
}

SDL_Rect font_draw_glyph(int x, int y, const char *str_utf8, uint8_t cl) {
	SDL_Surface *fs;
	SDL_Rect r_src, r_dst = {};
	int w, h;
	
	if (!*str_utf8)
		return r_dst;
	if (!fontset)
		return r_dst;
	
	bool antialias = this.antialiase_on;
	// The post-effect in Rance 3 opening does not work properly if colors other
	// than the specified text color (32) are used.
	// https://github.com/kichikuou/xsystem35-sdl2/issues/54
	// In Rance 3, text color 32 is only used in the opening.
	if ((game_id == GAME_RANCE3 || game_id == GAME_RANCE3_ENG) && cl == 32)
		antialias = false;

	if (antialias) {
		fs = TTF_RenderUTF8_Blended(fontset->id, str_utf8, sdl_col[cl]);
	} else {
		fs = TTF_RenderUTF8_Solid(fontset->id, str_utf8, sdl_col[cl]);
	}
	if (!fs) {
		WARNING("Text rendering failed: %s", TTF_GetError());
		return r_dst;
	}
	
	TTF_SizeUTF8(fontset->id, str_utf8, &w, &h);
	// Center vertically to the box.
	y -= (TTF_FontHeight(fontset->id) - fontset->size) / 2;
	r_dst = (SDL_Rect){x, y, w, h};
	
	if (main_surface->format->BitsPerPixel == 8 && antialias) {
		sdl_drawAntiAlias_8bpp(x, y, fs, cl);
	} else {
		r_src = (SDL_Rect){0, 0, w, h};
		SDL_BlitSurface(fs, &r_src, main_surface, &r_dst);
	}

	SDL_FreeSurface(fs);
	if (r_dst.y < 0) {
		r_dst.h += r_dst.y;
		r_dst.y = 0;
	}
	return r_dst;
}

void font_init(void) {
	this.antialiase_on = false;
	
	if (TTF_Init() == -1)
		SYSERROR("Failed to intialize SDL_ttf: %s", TTF_GetError());
}

void font_set_name_and_index(FontType type, const char *name, int index) {
	if (type >= FONTTYPEMAX) {
		WARNING("Invalid font type %d", type);
		return;
	}
	this.name[type] = name;
	this.face[type] = index;
}

void font_set_antialias(bool enable) {
	this.antialiase_on = enable;
}

bool font_get_antialias(void) {
	return this.antialiase_on;
}

#ifdef __EMSCRIPTEN__
EM_ASYNC_JS(bool, load_mincho_font, (void), {
	return await xsystem35.load_mincho_font();
});
#endif
