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
#include "gfx.h"
#include "hacks.h"

typedef struct {
	int      size;
	FontType type;
	TTF_Font *id;
} FontTable;

#define FONTTABLEMAX 256
static FontTable fonttbl[FONTTABLEMAX];
static int       fontcnt = 0;

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

// Resolve a (type, size) pair to a TTF_Font, opening and caching it on first
// use, and apply the requested weight. Returns NULL for an invalid type.
static FontTable *font_resolve(FontSpec font) {
	FontType type = font.type;
	int size = font.size;

	if (type >= FONTTYPEMAX) {
		WARNING("Invalid font type %d", type);
		return NULL;
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
		tbl = &fonttbl[fontcnt - 1];
	}
	TTF_SetFontStyle(tbl->id, font.weight == FONT_WEIGHT_BOLD ? TTF_STYLE_BOLD : TTF_STYLE_NORMAL);
	return tbl;
}

SDL_Surface *font_render_text(FontSpec font, const char *str_utf8, SDL_Color color) {
	FontTable *fontset = font_resolve(font);
	if (!fontset)
		return NULL;

	SDL_Surface *fs;
	fs = TTF_RenderUTF8_Blended(fontset->id, str_utf8, color);
	if (!fs)
		WARNING("Text rendering failed: %s", TTF_GetError());

	return fs;
}

void font_measure_text(FontSpec font, const char *str_utf8, int len, int *w, int *h) {
	if (w) *w = 0;
	if (h) *h = 0;
	FontTable *fontset = font_resolve(font);
	if (!fontset)
		return;
	if (h)
		*h = TTF_FontHeight(fontset->id);
	if (!w)
		return;

	// A negative length means the whole string.
	if (len < 0) {
		TTF_SizeUTF8(fontset->id, str_utf8, w, NULL);
	} else {
		char buf[256];
		if (len > (int)sizeof(buf) - 1)
			len = sizeof(buf) - 1;
		memcpy(buf, str_utf8, len);
		buf[len] = '\0';
		TTF_SizeUTF8(fontset->id, buf, w, NULL);
	}
}

SDL_Rect font_draw_text(FontSpec font, int x, int y, const char *str_utf8, uint8_t cl) {
	SDL_Surface *fs;
	SDL_Rect r_src, r_dst = {};
	int w, h;

	if (!*str_utf8)
		return r_dst;
	FontTable *fontset = font_resolve(font);
	if (!fontset)
		return r_dst;

	bool antialias = this.antialiase_on;
	// The post-effect in Rance 3 opening does not work properly if colors other
	// than the specified text color (32) are used.
	// https://github.com/kichikuou/xsystem35-sdl2/issues/54
	// In Rance 3, text color 32 is only used in the opening.
	if ((game_id == GAME_RANCE3 || game_id == GAME_RANCE3_ENG) && cl == 32)
		antialias = false;

	SDL_Color color = gfx_getPaletteColor(cl);
	if (antialias)
		fs = TTF_RenderUTF8_Blended(fontset->id, str_utf8, color);
	else
		fs = TTF_RenderUTF8_Solid(fontset->id, str_utf8, color);
	if (!fs) {
		WARNING("Text rendering failed: %s", TTF_GetError());
		return r_dst;
	}
	
	TTF_SizeUTF8(fontset->id, str_utf8, &w, &h);
	// Center vertically to the box.
	y -= (TTF_FontHeight(fontset->id) - fontset->size) / 2;
	r_dst = (SDL_Rect){x, y, w, h};
	
	if (main_surface->format->BitsPerPixel == 8 && antialias) {
		gfx_drawAntiAlias_8bpp(x, y, fs, cl);
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
