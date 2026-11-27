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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include "sdl_compat.h"

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
#include "cache.h"
#include "font.h"
#include "utfsjis.h"

// 26.6 fixed point helpers.
#define FT_FLOOR(x) ((x) >> 6)
#define FT_CEIL(x)  (((x) + 63) >> 6)

#define GLYPH_CACHE_LIMIT (4 * 1024 * 1024)

typedef struct {
	FT_Face face;
	int ascent;  // distance from the top of a rendered surface to the baseline
	int height;  // height of a rendered surface
} Font;

static FT_Library ft_library;

typedef struct Glyph {
	FT_Pos advance;
	int bitmap_left;
	int bitmap_top;
	FT_Bitmap bitmap;
} Glyph;

typedef struct {
	FontSpec spec;
	int code;
	bool antialias;
} GlyphKey;

static Cache *glyph_cache;

static struct {
	bool antialiase_on;
	CharacterEncoding encoding;
	const char *name[FONTTYPEMAX];
	int index[FONTTYPEMAX];
	FT_Face face[FONTTYPEMAX];  // opened on first use, shared between all sizes
	bool face_is_fallback[FONTTYPEMAX];
	FT_Face fallback_face[FONTTYPEMAX][2];  // [0] default/SJIS, [1] GBK/UTF8 (SC)
	bool fallback_checked[FONTTYPEMAX][2];
} this = {
	.encoding = SHIFT_JIS,
};

#define GZ_FONT_CACHE_MAX (FONTTYPEMAX * 2)
static struct {
	char *path;
	uint8_t *data;
	size_t size;
} gz_font_cache[GZ_FONT_CACHE_MAX];

static bool has_gz_suffix(const char *path) {
	size_t len = path ? strlen(path) : 0;
	return len > 3 && strcmp(path + len - 3, ".gz") == 0;
}

static FT_Face open_gzipped_face(const char *path, int index) {
	if (!path || !*path || !ft_library)
		return NULL;

	for (int i = 0; i < GZ_FONT_CACHE_MAX; i++) {
		if (gz_font_cache[i].path && !strcmp(gz_font_cache[i].path, path) && gz_font_cache[i].data) {
			FT_Face face = NULL;
			if (FT_New_Memory_Face(ft_library, gz_font_cache[i].data, (FT_Long)gz_font_cache[i].size, index, &face) == 0) {
				FT_Select_Charmap(face, FT_ENCODING_UNICODE);
				return face;
			}
			return NULL;
		}
	}

	int slot = -1;
	for (int i = 0; i < GZ_FONT_CACHE_MAX; i++) {
		if (!gz_font_cache[i].data) {
			slot = i;
			break;
		}
	}
	if (slot < 0)
		return NULL;

	FILE *fp = fopen(path, "rb");
	if (!fp)
		return NULL;

	z_stream zs;
	memset(&zs, 0, sizeof(zs));
	if (inflateInit2(&zs, 16 + MAX_WBITS) != Z_OK) {
		fclose(fp);
		return NULL;
	}

	const size_t max_out = 64 * 1024 * 1024;
	size_t cap = 256 * 1024;
	size_t out_len = 0;
	uint8_t *out = malloc(cap);
	if (!out) {
		inflateEnd(&zs);
		fclose(fp);
		return NULL;
	}

	uint8_t inbuf[32768];
	int zret = Z_OK;
	while (zret == Z_OK) {
		if (zs.avail_in == 0) {
			size_t n = fread(inbuf, 1, sizeof(inbuf), fp);
			if (n == 0) {
				if (ferror(fp))
					zret = Z_ERRNO;
				break;
			}
			zs.next_in = inbuf;
			zs.avail_in = (uInt)n;
		}
		do {
			if (out_len == cap) {
				if (cap >= max_out) {
					zret = Z_MEM_ERROR;
					break;
				}
				size_t new_cap = cap * 2;
				if (new_cap > max_out)
					new_cap = max_out;
				uint8_t *grown = realloc(out, new_cap);
				if (!grown) {
					zret = Z_MEM_ERROR;
					break;
				}
				out = grown;
				cap = new_cap;
			}
			zs.next_out = out + out_len;
			zs.avail_out = (uInt)(cap - out_len);
			zret = inflate(&zs, Z_NO_FLUSH);
			if (zret != Z_OK && zret != Z_STREAM_END)
				break;
			out_len = cap - zs.avail_out;
		} while (zret == Z_OK && zs.avail_out == 0);
	}
	inflateEnd(&zs);
	fclose(fp);

	if (zret != Z_STREAM_END || out_len == 0) {
		free(out);
		return NULL;
	}

	FT_Face face = NULL;
	if (FT_New_Memory_Face(ft_library, out, (FT_Long)out_len, index, &face) != 0) {
		free(out);
		return NULL;
	}
	FT_Select_Charmap(face, FT_ENCODING_UNICODE);
	gz_font_cache[slot].path = strdup(path);
	gz_font_cache[slot].data = out;
	gz_font_cache[slot].size = out_len;
	return face;
}

#if defined(_WIN32) || defined(__ANDROID__)
// Create a face from the whole content of `rw`. The memory holding the font
// file must outlive the face, and faces are never closed, so it is never freed.
static FT_Face face_from_rwops(SDL_IOStream *rw, int index) {
	Sint64 size = SDL_GetIOSize(rw);
	if (size <= 0) {
		SDL_CloseIO(rw);
		return NULL;
	}
	FT_Byte *buf = malloc(size);
	if (SDL_ReadIO(rw, buf, size) != (size_t)size) {
		free(buf);
		SDL_CloseIO(rw);
		return NULL;
	}
	SDL_CloseIO(rw);

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
		SDL_IOStream *res = open_resource(name, "fonts");
		if (res)
			face = face_from_rwops(res, index);
	}
#endif
	if (!face && FT_New_Face(ft_library, name, index, &face))
		face = NULL;  // FT_New_Face() does not clear `face` on failure.
#ifdef __ANDROID__
	// The default fonts are stored as apk assets, which can only be opened
	// through SDL_IOFromFile. (It is not used as the first choice because it
	// does not resolve a relative path against the current directory, which a
	// custom font specified in .xsys35rc may use.)
	if (!face) {
		SDL_IOStream *rw = SDL_IOFromFile(name, "rb");
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

static bool is_sc_face_name(const char *family) {
	if (!family)
		return false;
	return strstr(family, " SC") != NULL || strstr(family, "CN") != NULL;
}

static void select_sc_ttc_face(const char *path, FT_Face *face_io) {
	FT_Face cur = *face_io;
	if ((this.encoding != GBK && this.encoding != UTF8 && !chs_tbl_is_loaded()) || !cur || cur->num_faces <= 1)
		return;
	if (is_sc_face_name(cur->family_name) && strstr(cur->family_name, "Mono") != NULL)
		return;

	FT_Long num_faces = cur->num_faces;
	FT_Face fallback_sc = NULL;
	for (FT_Long i = 1; i < num_faces; i++) {
		FT_Face cand = NULL;
		if (FT_New_Face(ft_library, path, i, &cand) || !cand)
			continue;
		if (FT_Select_Charmap(cand, FT_ENCODING_UNICODE) == 0 && is_sc_face_name(cand->family_name)) {
			if (strstr(cand->family_name, "Mono") != NULL) {
				if (fallback_sc)
					FT_Done_Face(fallback_sc);
				FT_Done_Face(cur);
				*face_io = cand;
				return;
			}
			if (!fallback_sc) {
				fallback_sc = cand;
				continue;
			}
		}
		FT_Done_Face(cand);
	}
	if (fallback_sc) {
		FT_Done_Face(cur);
		*face_io = fallback_sc;
	}
}

static FT_Face get_fallback_face(FontType type) {
	if (type >= FONTTYPEMAX)
		return NULL;
	int slot = (this.encoding == GBK || this.encoding == UTF8 || chs_tbl_is_loaded()) ? 1 : 0;
	if (this.fallback_checked[type][slot])
		return this.fallback_face[type][slot];
	this.fallback_checked[type][slot] = true;

	const char *env_fallback = getenv("XSYSTEM35_FONT_FALLBACK");
	if (env_fallback && env_fallback[0]) {
		FT_Face face = has_gz_suffix(env_fallback)
			? open_gzipped_face(env_fallback, 0)
			: open_face(env_fallback, 0);
		if (face) {
			if (env_fallback[0] == '/' && !has_gz_suffix(env_fallback))
				select_sc_ttc_face(env_fallback, &face);
			if (FT_Get_Char_Index(face, 0x8fd9) != 0) {
				this.fallback_face[type][slot] = face;
				return face;
			}
			FT_Done_Face(face);
		}
	}

#ifdef __SWITCH__
	{
		FT_Face face = open_gzipped_face("romfs:/fonts/HanaMinA.ttf.gz", 0);
		if (face) {
			if (FT_Get_Char_Index(face, 0x8fd9) != 0) {
				this.fallback_face[type][slot] = face;
				return face;
			}
			FT_Done_Face(face);
		}
	}
#endif

	static const char *const gothic_candidates[] = {
#ifdef __ANDROID__
		"/system/fonts/NotoSansCJK-Regular.ttc",
		"/system/fonts/NotoSansSC-Regular.otf",
		"/system/fonts/NotoSansSC-VF.ttf",
		"/system/fonts/DroidSansFallback.ttf",
		"fonts/HanaMinA.ttf",
		"/system/fonts/NotoSerifCJK-Regular.ttc",
#elif defined(__SWITCH__)
		"romfs:/fonts/HanaMinA.ttf",
#else
		"/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
		"/usr/share/fonts/noto-cjk/NotoSansCJK-Regular.ttc",
		"/usr/share/fonts/truetype/NotoSansCJK-Regular.ttc",
		"/usr/share/fonts/truetype/noto/NotoSansSC-Regular.otf",
		"/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf",
#endif
		"fonts/HanaMinA.ttf",
		"fonts/mincho.ttf",
		NULL
	};
	static const char *const mincho_candidates[] = {
#ifdef __ANDROID__
		"/system/fonts/NotoSerifCJK-Regular.ttc",
		"fonts/HanaMinA.ttf",
		"/system/fonts/NotoSansCJK-Regular.ttc",
		"/system/fonts/NotoSansSC-Regular.otf",
		"/system/fonts/NotoSansSC-VF.ttf",
		"/system/fonts/DroidSansFallback.ttf",
#elif defined(__SWITCH__)
		"romfs:/fonts/HanaMinA.ttf",
#else
		"/usr/share/fonts/opentype/noto/NotoSerifCJK-Regular.ttc",
		"/usr/share/fonts/noto-cjk/NotoSerifCJK-Regular.ttc",
		"/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
		"/usr/share/fonts/truetype/NotoSansCJK-Regular.ttc",
		"/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf",
#endif
		"fonts/HanaMinA.ttf",
		"fonts/MTLc3m.ttf",
		NULL
	};
	const char *const *candidates = (type == FONT_MINCHO) ? mincho_candidates : gothic_candidates;
	for (int i = 0; candidates[i]; i++) {
		FT_Face face = open_face(candidates[i], 0);
		if (!face)
			continue;
		if (candidates[i][0] == '/')
			select_sc_ttc_face(candidates[i], &face);
		// Verify the candidate actually contains Simplified Chinese glyphs (e.g. U+8FD9 "这").
		if (FT_Get_Char_Index(face, 0x8fd9) != 0) {
			this.fallback_face[type][slot] = face;
			return face;
		}
		FT_Done_Face(face);
	}
	return NULL;
}

static bool set_face_size(FT_Face face, FontSpec spec, const char *label) {
	// With a resolution of 0 (i.e. the default 72dpi), 1pt equals 1px.
	if (FT_Set_Char_Size(face, 0, spec.size * 64, 0, 0) &&
	    FT_Set_Pixel_Sizes(face, 0, spec.size)) {
		WARNING("Cannot set the size of font %s to %d", label, spec.size);
		return false;
	}
	return true;
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
		this.face_is_fallback[spec.type] = false;
		if (!this.face[spec.type]) {
			const char *fallback_name = spec.type == FONT_MINCHO ? DEFAULT_MINCHO_TTF : DEFAULT_GOTHIC_TTF;
			FT_Face fallback = get_fallback_face(spec.type);
			if (!fallback)
				fallback = open_face(fallback_name, 0);
			if (fallback) {
				WARNING("Cannot open font %s; using fallback font", this.name[spec.type]);
				this.face[spec.type] = fallback;
				this.face_is_fallback[spec.type] = true;
			} else {
				SYSERROR("Cannot open font %s", this.name[spec.type]);
			}
		}
	}
	FT_Face face = this.face[spec.type];
	// GBK always denotes Simplified Chinese, so replace an incomplete primary
	// face with the SC fallback font. UTF-8 is encoding-only (used by Japanese,
	// Latin, and Chinese games alike), so preserve the configured primary face
	// here and fall back per-glyph in get_glyph().
	if ((this.encoding == GBK || chs_tbl_is_loaded()) && FT_Get_Char_Index(face, 0x8fd9) == 0) {
		FT_Face cjk = get_fallback_face(spec.type);
		if (cjk)
			face = cjk;
	}

	if (!set_face_size(face, spec, this.name[spec.type]))
		return false;
	font->face = face;
	font->ascent = FT_CEIL(face->size->metrics.ascender);
	font->height = font->ascent - FT_FLOOR(face->size->metrics.descender);
	return true;
}

// Distance from the top of the character cell to the baseline, the same value
// GDI reports as TEXTMETRIC::tmAscent. The original engine positions text by
// the cell top, so this determines where a glyph lands on the screen.
//
// The games were laid out for MS Gothic / MS Mincho / SimSun, whose cell is
// the ideographic em box. CJK fonts such as Noto Sans/Serif CJK add line
// spacing to usWinAscent (1.16em) and keep the em box in the typo metrics
// (0.88em / -0.12em); use the em box for them, or text sits ~0.28em too low.
// Latin fonts (e.g. DejaVu) keep usWinAscent, as GDI does.
static bool has_cjk_em_box(const TT_OS2 *os2, int upem) {
	enum { CJK_CODE_PAGES = 0x1f << 17 };  // JIS, GBK, Wansung, Big5, Johab
	return os2->version >= 1 && (os2->ulCodePageRange1 & CJK_CODE_PAGES) &&
		os2->sTypoLineGap == 0 && os2->sTypoAscender > 0 &&
		os2->sTypoAscender < os2->usWinAscent &&
		os2->sTypoAscender - os2->sTypoDescender == upem;
}

static int cell_ascent(const Font *font) {
	TT_OS2 *os2 = FT_Get_Sfnt_Table(font->face, FT_SFNT_OS2);
	if (!os2 || os2->version == 0xffff || !os2->usWinAscent)
		return font->ascent;
	int upem = font->face->units_per_EM;
	int ascent = has_cjk_em_box(os2, upem) ? os2->sTypoAscender : os2->usWinAscent;
	return (ascent * font->face->size->metrics.y_ppem + upem / 2) / upem;
}

int font_cell_overhang(FontSpec spec) {
	Font font;
	if (!font_select(spec, &font))
		return 0;
	return font.ascent - cell_ascent(&font);
}

static bool load_glyph(FontSpec spec, FT_Face face, int code) {
	if (FT_Load_Char(face, code, FT_LOAD_DEFAULT))
		return false;
	if (spec.weight == FONT_WEIGHT_BOLD) {
		// Approximate GDI's synthetic bold without changing the advance.
		// Expand outlines before rasterization so antialiased edges are
		// generated only once.
		FT_GlyphSlot slot = face->glyph;
		if (slot->format == FT_GLYPH_FORMAT_OUTLINE) {
			// At small sizes, use half-pixel emboldening to better match
			// GDI's synthetic bold for the thin bitmap glyphs of MS Gothic
			// and MS Mincho.
			int xstrength = spec.size <= 22 ? 32 : 64;
			if (FT_Outline_EmboldenXY(&slot->outline, xstrength, 0))
				return false;
		} else if (slot->format == FT_GLYPH_FORMAT_BITMAP) {
			if (FT_GlyphSlot_Own_Bitmap(slot) ||
			    FT_Bitmap_Embolden(ft_library, &slot->bitmap, 64, 0))
				return false;
		}
	}
	return true;
}

static uint32_t glyph_hash(const void *p) {
	const GlyphKey *key = p;
	uint32_t hash = (uint32_t)key->code;
	hash = hash * 31 + key->spec.type;
	hash = hash * 31 + key->spec.weight;
	hash = hash * 31 + key->spec.size;
	return hash * 31 + key->antialias;
}

static bool glyph_key_equal(const void *a, const void *b) {
	const GlyphKey *ka = a;
	const GlyphKey *kb = b;
	return ka->spec.type == kb->spec.type &&
	       ka->spec.weight == kb->spec.weight &&
	       ka->spec.size == kb->spec.size &&
	       ka->code == kb->code && ka->antialias == kb->antialias;
}

static void glyph_destroy(void *data) {
	Glyph *glyph = data;
	FT_Bitmap_Done(ft_library, &glyph->bitmap);
	free(glyph);
}

// Load and render a glyph on a cache miss. If allocation fails, return a
// temporary view of the face's glyph slot so text rendering can continue.
static const Glyph *get_glyph(FontSpec spec, FT_Face face, int code, bool antialias) {
	GlyphKey key = {spec, code, antialias};
	const Glyph *cached = cache_get(glyph_cache, &key);
	if (cached)
		return cached;

	FT_Face render_face = face;
	if (code > 0 && FT_Get_Char_Index(face, code) == 0) {
		FT_Face alt = (face == this.face[spec.type])
			? get_fallback_face(spec.type)
			: this.face[spec.type];
		if (alt && FT_Get_Char_Index(alt, code) != 0 && set_face_size(alt, spec, "fallback"))
			render_face = alt;
	}

	if (!load_glyph(spec, render_face, code))
		return NULL;
	FT_GlyphSlot slot = render_face->glyph;
	static Glyph temporary;
	temporary.advance = slot->advance.x;
	FT_Bitmap_Init(&temporary.bitmap);
	if (FT_Render_Glyph(slot, antialias ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_MONO))
		return &temporary;
	temporary.bitmap_left = slot->bitmap_left;
	temporary.bitmap_top = slot->bitmap_top;
	temporary.bitmap = slot->bitmap;

	size_t bitmap_size = slot->bitmap.rows * (size_t)abs(slot->bitmap.pitch);
	size_t cache_size = sizeof(Glyph) + bitmap_size;
	if (cache_size > GLYPH_CACHE_LIMIT)
		return &temporary;

	Glyph *glyph = calloc(1, sizeof(Glyph));
	if (!glyph)
		return &temporary;
	FT_Bitmap_Init(&glyph->bitmap);
	if (FT_Bitmap_Copy(ft_library, &slot->bitmap, &glyph->bitmap)) {
		FT_Bitmap_Done(ft_library, &glyph->bitmap);
		free(glyph);
		return &temporary;
	}
	glyph->advance = slot->advance.x;
	glyph->bitmap_left = slot->bitmap_left;
	glyph->bitmap_top = slot->bitmap_top;
	if (cache_insert(glyph_cache, &key, glyph, cache_size) != CACHE_INSERT_OK) {
		glyph_destroy(glyph);
		return &temporary;
	}
	return glyph;
}

// Total advance of `str_utf8`, in pixels. A negative `len` means the whole
// string, otherwise only its first `len` bytes are measured.
static int text_width(FontSpec spec, FT_Face face, const char *str_utf8, int len,
		      bool antialias) {
	const char *end = len < 0 ? NULL : str_utf8 + len;
	int pen = 0;  // 26.6

	while (*str_utf8 && (!end || str_utf8 < end)) {
		const Glyph *glyph = get_glyph(spec, face,
		                               utf8_next_codepoint(&str_utf8), antialias);
		if (glyph)
			pen += glyph->advance;
	}
	return FT_CEIL(pen);
}

// Composite a rendered glyph into `dst`, which is either an ARGB8888 surface
// (the glyph is drawn in `color`, its coverage scaled by `color.a` becoming the
// alpha value) or an INDEX8 surface (covered pixels are set to the index 1).
static void blit_glyph(const FT_Bitmap *bmp, SDL_Surface *dst, int x, int y, SDL_Color color) {
	int bpp = sdl_SurfaceBytesPerPixel(dst);
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
	int width = max(text_width(spec, font.face, str_utf8, -1, antialias), 1);

	SDL_Surface *sf;
	if (antialias) {
		sf = sdl_CreateSurface(width, font.height, SDL_PIXELFORMAT_ARGB8888);
		if (sf)
			SDL_SetSurfaceBlendMode(sf, SDL_BLENDMODE_BLEND);
	} else {
		sf = sdl_CreateSurface(width, font.height, SDL_PIXELFORMAT_INDEX8);
		if (sf) {
			SDL_Color pal[2] = {{0, 0, 0, 0}, {color.r, color.g, color.b, 255}};
			SDL_SetPaletteColors(SDL_GetSurfacePalette(sf), pal, 0, 2);
			SDL_SetSurfaceColorKey(sf, true, 0);
		}
	}
	if (!sf) {
		WARNING("Text rendering failed: %s", SDL_GetError());
		return NULL;
	}

	int pen = 0;  // 26.6
	while (*str_utf8) {
		const Glyph *glyph = get_glyph(spec, font.face,
		                               utf8_next_codepoint(&str_utf8), antialias);
		if (!glyph)
			continue;
		blit_glyph(&glyph->bitmap, sf, FT_FLOOR(pen) + glyph->bitmap_left,
		           font.ascent - glyph->bitmap_top, color);
		pen += glyph->advance;
	}
	return sf;
}

void font_measure_text(FontSpec spec, const char *str_utf8, int len, bool antialias,
		       int *w, int *h) {
	if (w) *w = 0;
	if (h) *h = 0;
	Font font;
	if (!font_select(spec, &font))
		return;
	if (h)
		*h = font.height;
	if (w)
		*w = text_width(spec, font.face, str_utf8, len, antialias);
}

void font_init(void) {
	cache_clear(glyph_cache);
	this.antialiase_on = false;

	if (FT_Init_FreeType(&ft_library))
		SYSERROR("Failed to initialize FreeType");
	if (!glyph_cache) {
		CacheOps ops = {
			.key_size = sizeof(GlyphKey),
			.hash = glyph_hash,
			.equal = glyph_key_equal,
			.destroy = glyph_destroy,
		};
		glyph_cache = cache_new(GLYPH_CACHE_LIMIT, &ops);
	}
}

static void clear_primary_face(FontType type) {
	FT_Face face = this.face[type];
	if (face &&
	    face != this.fallback_face[type][0] &&
	    face != this.fallback_face[type][1]) {
		FT_Done_Face(face);
	}
	this.face[type] = NULL;
	this.face_is_fallback[type] = false;
}

void font_fini(void) {
	if (glyph_cache) {
		cache_destroy(glyph_cache);
		glyph_cache = NULL;
	}
	if (ft_library) {
		FT_Done_FreeType(ft_library);
		ft_library = NULL;
	}
	for (int i = 0; i < GZ_FONT_CACHE_MAX; i++) {
		free(gz_font_cache[i].path);
		free(gz_font_cache[i].data);
		gz_font_cache[i].path = NULL;
		gz_font_cache[i].data = NULL;
		gz_font_cache[i].size = 0;
	}
	memset(&this, 0, sizeof(this));
	this.encoding = SHIFT_JIS;
}

void font_set_encoding(CharacterEncoding encoding) {
	if (this.encoding != encoding) {
		this.encoding = encoding;
		for (int i = 0; i < FONTTYPEMAX; i++) {
			if (this.face_is_fallback[i])
				clear_primary_face(i);
		}
		font_clear_cache();
	}
}

void font_clear_cache(void) {
	if (glyph_cache)
		cache_clear(glyph_cache);
}

void font_set_name_and_index(FontType type, const char *name, int index) {
	if (type >= FONTTYPEMAX) {
		WARNING("Invalid font type %d", type);
		return;
	}
	// Cached glyphs include the font type but not the font file name.
	cache_clear(glyph_cache);
	this.name[type] = name;
	this.index[type] = index;
	clear_primary_face(type);
}

void font_set_antialias(bool enable) {
	this.antialiase_on = enable;
}

bool font_get_antialias(void) {
	return this.antialiase_on;
}

CacheStats font_get_cache_stats(void) {
	return cache_get_stats(glyph_cache);
}

#ifdef __EMSCRIPTEN__
EM_ASYNC_JS(bool, load_mincho_font, (void), {
	return await xsystem35.load_mincho_font();
});
#endif
