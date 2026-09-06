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

#include <stdlib.h>
#include <string.h>
#include <SDL.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_BITMAP_H
#include FT_OUTLINE_H
#include FT_TRUETYPE_TABLES_H

#ifdef _WIN32
#include "win/resources.h"
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "portab.h"
#include "system.h"
#include "font.h"
#include "utfsjis.h"

// 26.6 fixed point helpers.
#define FT_FLOOR(x) ((x) >> 6)
#define FT_CEIL(x)  (((x) + 63) >> 6)

typedef struct {
	FT_Face face;
	int ascent;  // distance from the top of a rendered surface to the baseline
	int height;  // height of a rendered surface
} Font;

static FT_Library ft_library;

static struct {
	bool antialiase_on;
	const char *name[FONTTYPEMAX];
	int index[FONTTYPEMAX];
	FT_Face face[FONTTYPEMAX];  // opened on first use, shared between all sizes
} this;

#if defined(_WIN32) || defined(__ANDROID__)
// Create a face from the whole content of `rw`. The memory holding the font
// file must outlive the face, and faces are never closed, so it is never freed.
static FT_Face face_from_rwops(SDL_RWops *rw, int index) {
	Sint64 size = SDL_RWsize(rw);
	if (size <= 0) {
		SDL_RWclose(rw);
		return NULL;
	}
	FT_Byte *buf = malloc(size);
	if (SDL_RWread(rw, buf, 1, size) != (size_t)size) {
		free(buf);
		SDL_RWclose(rw);
		return NULL;
	}
	SDL_RWclose(rw);

	FT_Face face;
	if (FT_New_Memory_Face(ft_library, buf, size, index, &face)) {
		free(buf);
		return NULL;
	}
	return face;
}
#endif

static FT_Face open_face(const char *name, int index) {
	FT_Face face = NULL;

#ifdef _WIN32
	const char *system_font = NULL;
	if (!strcmp(name, DEFAULT_GOTHIC_TTF))
		system_font = "C:/Windows/Fonts/msgothic.ttc";
	else if (!strcmp(name, DEFAULT_MINCHO_TTF))
		system_font = "C:/Windows/Fonts/msmincho.ttc";
	if (system_font && FT_New_Face(ft_library, system_font, index, &face))
		face = NULL;

	if (!face) {
		SDL_RWops *res = open_resource(name, "fonts");
		if (res)
			face = face_from_rwops(res, index);
	}
#endif
	if (!face && FT_New_Face(ft_library, name, index, &face))
		face = NULL;  // FT_New_Face() does not clear `face` on failure.
#ifdef __ANDROID__
	// The default fonts are stored as apk assets, which can only be opened
	// through SDL_RWFromFile. (It is not used as the first choice because it
	// does not resolve a relative path against the current directory, which a
	// custom font specified in .xsys35rc may use.)
	if (!face) {
		SDL_RWops *rw = SDL_RWFromFile(name, "rb");
		if (rw)
			face = face_from_rwops(rw, index);
	}
#endif
	if (face) {
		// Prefer a Unicode charmap; if there is none, keep FreeType's choice.
		FT_Select_Charmap(face, FT_ENCODING_UNICODE);
	}
	return face;
}

// Since a face is shared between all sizes of a FontType, the size has to be
// applied on every call.
static bool font_select(FontSpec spec, Font *font) {
	if (spec.type >= FONTTYPEMAX) {
		WARNING("Invalid font type %d", spec.type);
		return false;
	}
	if (!this.face[spec.type]) {
		this.face[spec.type] = open_face(this.name[spec.type], this.index[spec.type]);
		if (!this.face[spec.type])
			SYSERROR("Cannot open font %s", this.name[spec.type]);
	}
	FT_Face face = this.face[spec.type];

	// With a resolution of 0 (i.e. the default 72dpi), 1pt equals 1px.
	if (FT_Set_Char_Size(face, 0, spec.size * 64, 0, 0) &&
	    FT_Set_Pixel_Sizes(face, 0, spec.size)) {
		WARNING("Cannot set the size of font %s to %d", this.name[spec.type], spec.size);
		return false;
	}
	font->face = face;
	font->ascent = FT_CEIL(face->size->metrics.ascender);
	font->height = font->ascent - FT_FLOOR(face->size->metrics.descender);
	return true;
}

// Distance from the top of the character cell to the baseline, the same value
// GDI reports as TEXTMETRIC::tmAscent. The original engine positions text by
// the cell top, so this determines where a glyph lands on the screen.
static int cell_ascent(const Font *font) {
	TT_OS2 *os2 = FT_Get_Sfnt_Table(font->face, FT_SFNT_OS2);
	if (!os2 || os2->version == 0xffff || !os2->usWinAscent)
		return font->ascent;
	int upem = font->face->units_per_EM;
	return (os2->usWinAscent * font->face->size->metrics.y_ppem + upem / 2) / upem;
}

int font_cell_overhang(FontSpec spec) {
	Font font;
	if (!font_select(spec, &font))
		return 0;
	return font.ascent - cell_ascent(&font);
}

static bool load_glyph(FT_Face face, int code, bool bold) {
	if (FT_Load_Char(face, code, FT_LOAD_DEFAULT))
		return false;
	if (bold) {
		// Approximate GDI's synthetic bold at small sizes: grow only to the
		// right by 1px, without changing the advance. Expand outlines before
		// rasterization so antialiased edges are generated only once.
		FT_GlyphSlot slot = face->glyph;
		if (slot->format == FT_GLYPH_FORMAT_OUTLINE) {
			if (FT_Outline_EmboldenXY(&slot->outline, 64, 0))
				return false;
		} else if (slot->format == FT_GLYPH_FORMAT_BITMAP) {
			if (FT_GlyphSlot_Own_Bitmap(slot) ||
			    FT_Bitmap_Embolden(ft_library, &slot->bitmap, 64, 0))
				return false;
		}
	}
	return true;
}

// Total advance of `str_utf8`, in pixels. A negative `len` means the whole
// string, otherwise only its first `len` bytes are measured.
static int text_width(FT_Face face, const char *str_utf8, int len) {
	const char *end = len < 0 ? NULL : str_utf8 + len;
	int pen = 0;  // 26.6

	while (*str_utf8 && (!end || str_utf8 < end)) {
		if (load_glyph(face, utf8_next_codepoint(&str_utf8), false))
			pen += face->glyph->advance.x;
	}
	return FT_CEIL(pen);
}

// Composite a rendered glyph into `dst`, which is either an ARGB8888 surface
// (the glyph is drawn in `color`, its coverage scaled by `color.a` becoming the
// alpha value) or an INDEX8 surface (covered pixels are set to the index 1).
static void blit_glyph(const FT_Bitmap *bmp, SDL_Surface *dst, int x, int y, SDL_Color color) {
	int bpp = dst->format->BytesPerPixel;
	uint32_t rgb = color.r << 16 | color.g << 8 | color.b;

	for (unsigned int row = 0; row < bmp->rows; row++) {
		int dy = y + row;
		if (dy < 0 || dy >= dst->h)
			continue;
		const uint8_t *src = bmp->buffer + row * bmp->pitch;
		uint8_t *dst_row = (uint8_t *)dst->pixels + dy * dst->pitch;
		for (unsigned int col = 0; col < bmp->width; col++) {
			int dx = x + col;
			if (dx < 0 || dx >= dst->w)
				continue;
			uint8_t coverage;
			switch (bmp->pixel_mode) {
			case FT_PIXEL_MODE_MONO:
				coverage = src[col >> 3] & (0x80 >> (col & 7)) ? 255 : 0;
				break;
			case FT_PIXEL_MODE_GRAY:
				coverage = src[col];
				break;
			default:
				continue;
			}
			if (!coverage)
				continue;
			uint8_t *dp = dst_row + dx * bpp;
			if (bpp == 4)
				*(uint32_t *)dp = (uint32_t)(coverage * color.a / 255) << 24 | rgb;
			else
				*dp = 1;
		}
	}
}

SDL_Surface *font_render_text(FontSpec spec, const char *str_utf8, SDL_Color color, bool antialias) {
	Font font;
	if (!font_select(spec, &font))
		return NULL;
	bool bold = spec.weight == FONT_WEIGHT_BOLD;
	int width = max(text_width(font.face, str_utf8, -1), 1);

	SDL_Surface *sf;
	if (antialias) {
		sf = SDL_CreateRGBSurfaceWithFormat(0, width, font.height, 32, SDL_PIXELFORMAT_ARGB8888);
		if (sf)
			SDL_SetSurfaceBlendMode(sf, SDL_BLENDMODE_BLEND);
	} else {
		sf = SDL_CreateRGBSurfaceWithFormat(0, width, font.height, 8, SDL_PIXELFORMAT_INDEX8);
		if (sf) {
			SDL_Color pal[2] = {{0, 0, 0, 0}, {color.r, color.g, color.b, 255}};
			SDL_SetPaletteColors(sf->format->palette, pal, 0, 2);
			SDL_SetColorKey(sf, SDL_TRUE, 0);
		}
	}
	if (!sf) {
		WARNING("Text rendering failed: %s", SDL_GetError());
		return NULL;
	}

	int pen = 0;  // 26.6
	while (*str_utf8) {
		if (!load_glyph(font.face, utf8_next_codepoint(&str_utf8), bold))
			continue;
		FT_GlyphSlot slot = font.face->glyph;
		if (!FT_Render_Glyph(slot, antialias ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_MONO)) {
			blit_glyph(&slot->bitmap, sf, FT_FLOOR(pen) + slot->bitmap_left,
			           font.ascent - slot->bitmap_top, color);
		}
		pen += slot->advance.x;
	}
	return sf;
}

void font_measure_text(FontSpec spec, const char *str_utf8, int len, int *w, int *h) {
	if (w) *w = 0;
	if (h) *h = 0;
	Font font;
	if (!font_select(spec, &font))
		return;
	if (h)
		*h = font.height;
	if (w)
		*w = text_width(font.face, str_utf8, len);
}

void font_init(void) {
	this.antialiase_on = false;

	if (FT_Init_FreeType(&ft_library))
		SYSERROR("Failed to initialize FreeType");
}

void font_set_name_and_index(FontType type, const char *name, int index) {
	if (type >= FONTTYPEMAX) {
		WARNING("Invalid font type %d", type);
		return;
	}
	this.name[type] = name;
	this.index[type] = index;
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
