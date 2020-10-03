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
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "portab.h"
#include "system.h"
#include "font.h"
#include "sdl_private.h"

typedef struct {
	int      size;
	int      type;
	TTF_Font *id;
} FontTable;

#define FONTTABLEMAX 256
static FontTable fonttbl[FONTTABLEMAX];
static int       fontcnt = 0;

static FontTable *fontset;

static struct {
	boolean antialiase_on;
	const char *name[FONTTYPEMAX];
	int face[FONTTYPEMAX];
} this;

static void font_insert(int size, int type, TTF_Font *font) {
	fonttbl[fontcnt].size = size;
	fonttbl[fontcnt].type = type;
	fonttbl[fontcnt].id   = font;
	
	if (fontcnt >= (FONTTABLEMAX -1)) {
		WARNING("Font table is full.\n");
	} else {
		fontcnt++;
	}
}

static FontTable *font_lookup(int size, int type) {
	int i;
	
	for (i = 0; i < fontcnt; i++) {
		if (fonttbl[i].size == size && fonttbl[i].type == type) { 
			return &fonttbl[i];
		}
	}
	return NULL;
}

void font_select(int type, int size) {
	FontTable *tbl;

	if (NULL == (tbl = font_lookup(size, type))) {
		TTF_Font *fs;
		
		fs = TTF_OpenFontIndex(this.name[type], size, this.face[type]);
		
		if (fs == NULL) {
			WARNING("%s is not found:\n", this.name[type]);
			return;
		}
		
		font_insert(size, type, fs);
		fontset = &fonttbl[fontcnt - 1];
	} else {
		fontset = tbl;
	}
}

agsurface_t *font_get_glyph(const char *str_utf8) {
	static SDL_Surface *fs;
	static agsurface_t result;

	if (!fontset)
		return NULL;
	if (fs) {
		SDL_FreeSurface(fs);
		fs = NULL;
	}

	SDL_Color color = {255, 255, 255, 0};
	if (this.antialiase_on) {
		fs = TTF_RenderUTF8_Shaded(fontset->id, str_utf8, color, color);
	} else {
		fs = TTF_RenderUTF8_Solid(fontset->id, str_utf8, color);
	}
	if (!fs) {
		WARNING("Text rendering failed: %s\n", TTF_GetError());
		return NULL;
	}

	result.depth = fs->format->BitsPerPixel;
	result.bytes_per_pixel = fs->format->BytesPerPixel;
	result.bytes_per_line = fs->pitch;
	result.pixel = fs->pixels;
	result.width = fs->w;
	result.height = fs->h;
	return &result;
}

// SDL can't blit ARGB to an indexed bitmap properly, so we do it ourselves.
static void sdl_drawAntiAlias_8bpp(int dstx, int dsty, SDL_Surface *src, unsigned long col)
{
	SDL_LockSurface(sdl_dib);

	Uint8 cache[256*7];
	memset(cache, 0, 256);

	for (int y = 0; y < src->h && dsty + y < sdl_dib->h; y++) {
		BYTE *sp = (BYTE*)src->pixels + y * src->pitch;
		BYTE *dp = (BYTE*)sdl_dib->pixels + (dsty + y) * sdl_dib->pitch + dstx;
		for (int x = 0; x < src->w && dstx + x < sdl_dib->w; x++) {
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

	SDL_UnlockSurface(sdl_dib);
}

int font_draw_glyph(int x, int y, const char *str_utf8, int cl) {
	SDL_Surface *fs;
	SDL_Rect r_src, r_dst;
	int w, h, maxy;
	
	if (!*str_utf8)
		return 0;
	if (!fontset)
		return 0;
	
	if (this.antialiase_on) {
		fs = TTF_RenderUTF8_Blended(fontset->id, str_utf8, sdl_col[cl]);
	} else {
		fs = TTF_RenderUTF8_Solid(fontset->id, str_utf8, sdl_col[cl]);
	}
	if (!fs) {
		WARNING("Text rendering failed: %s\n", TTF_GetError());
		return 0;
	}
	
	TTF_SizeUTF8(fontset->id, str_utf8, &w, &h);
	y = max(0, y - (TTF_FontAscent(fontset->id) - fontset->size * 0.9));
	
	if (sdl_dib->format->BitsPerPixel == 8 && this.antialiase_on) {
		sdl_drawAntiAlias_8bpp(x, y, fs, cl);
	} else {
		setRect(r_src, 0, 0, w, h);
		setRect(r_dst, x, y, w, h);
		SDL_BlitSurface(fs, &r_src, sdl_dib, &r_dst);
	}

	SDL_FreeSurface(fs);
	return w;
}

void font_init(void) {
	this.antialiase_on = FALSE;
	
	if (TTF_Init() == -1)
		SYSERROR("Failed to intialize SDL_ttf: %s\n", TTF_GetError());
}

void font_set_name_and_index(int type, const char *name, int index) {
	this.name[type] = name;
	this.face[type] = index;
}

void font_set_antialias(boolean enable) {
	this.antialiase_on = enable;
}

boolean font_get_antialias(void) {
	return this.antialiase_on;
}

#ifdef __EMSCRIPTEN__
EM_JS(int, load_mincho_font, (void), {
	return Asyncify.handleSleep(function(wakeUp) {
		xsystem35.load_mincho_font().then(wakeUp);
	});
});
#endif
