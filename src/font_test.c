/*
 * Copyright (C) 2026 <KichikuouChrome@gmail.com>
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
 */

#include "font.h"
#include "unittest.h"

#include <stdlib.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <unistd.h>
#include "sdl_compat.h"

static const FontType font_types[] = {FONT_GOTHIC, FONT_MINCHO};
#define NTYPES ((int)(sizeof(font_types) / sizeof(font_types[0])))

static const int sizes[] = {8, 16, 24, 32};
#define NSIZES ((int)(sizeof(sizes) / sizeof(sizes[0])))

static const char *texts[] = {
	"Hello, world!",
	" ",
	"iWiW",
	"\xe9\xad\x94\xe7\x89\xa9\xe3\x81\xae\xe6\xa3\xae",  // 魔物の森
	"\xe3\x81\x82\xe3\x81\x84\xe3\x81\x86"               // あいう
	"\xe3\x82\xa2\xe3\x82\xa4\xe3\x82\xa6",              // アイウ
};
#define NTEXTS ((int)(sizeof(texts) / sizeof(texts[0])))

static const SDL_Color white = {255, 255, 255, 255};

static int measure_w(FontSpec f, const char *s, bool antialias) {
	int w;
	font_measure_text(f, s, -1, antialias, &w, NULL);
	return w;
}

static int measure_h(FontSpec f) {
	int h;
	font_measure_text(f, "", -1, false, NULL, &h);
	return h;
}

static void assert_same_pixels(SDL_Surface *a, SDL_Surface *b) {
	ASSERT_EQUAL(a->w, b->w);
	ASSERT_EQUAL(a->h, b->h);
	ASSERT_EQUAL(sdl_SurfaceFormat(a), sdl_SurfaceFormat(b));
	int row_bytes = a->w * sdl_SurfaceBytesPerPixel(a);
	for (int y = 0; y < a->h; y++) {
		const uint8_t *ap = (uint8_t *)a->pixels + y * a->pitch;
		const uint8_t *bp = (uint8_t *)b->pixels + y * b->pitch;
		ASSERT_EQUAL(memcmp(ap, bp, row_bytes), 0);
	}
}

static void glyph_cache_test(void) {
	for (int ti = 0; ti < NTYPES; ti++) {
		for (int weight = FONT_WEIGHT_NORMAL; weight <= FONT_WEIGHT_BOLD; weight++) {
			FontSpec f = {font_types[ti], weight, 24};
			for (int aa = 0; aa < 2; aa++) {
				SDL_Surface *first = font_render_text(f, texts[3], white, aa);
				SDL_Surface *cached = font_render_text(f, texts[3], white, aa);
				ASSERT_TRUE(first);
				ASSERT_TRUE(cached);
				assert_same_pixels(first, cached);
				SDL_DestroySurface(first);
				SDL_DestroySurface(cached);
			}
		}
	}
}

static void invariants_test(void) {
	for (int ti = 0; ti < NTYPES; ti++) {
		for (int si = 0; si < NSIZES; si++) {
			FontSpec f = {font_types[ti], FONT_WEIGHT_NORMAL, sizes[si]};
			for (int i = 0; i < NTEXTS; i++) {
				const char *text = texts[i];
				for (int aa = 0; aa < 2; aa++) {
					SDL_Surface *s = font_render_text(f, text, white, aa);
					ASSERT_TRUE(s);
					// gfx_drawString() blits the surface using the measured
					// width, so the two must agree.
					ASSERT_EQUAL(s->w, measure_w(f, text, aa));
					// microui lays out text rows by the measured height,
					// so the surface must be exactly that tall.
					ASSERT_EQUAL(s->h, measure_h(f));
					// The antialiased surface goes to gfx_drawAntiAlias_8bpp(),
					// which reads a 32-bit alpha channel. The other one is
					// blitted onto the indexed screen surface as is.
					ASSERT_EQUAL(sdl_SurfaceBitsPerPixel(s), aa ? 32 : 8);
					SDL_DestroySurface(s);
				}
			}
			// gfx_drawString() draws text by the top of the character cell,
			// shifting the surface up by the overhang. The result must cover
			// the whole cell.
			ASSERT_TRUE(font_cell_overhang(f) >= 0);
			ASSERT_TRUE(measure_h(f) - font_cell_overhang(f) >= f.size);

			// microui positions the text cursor by measuring a prefix of the
			// string, so that must equal measuring a truncated copy.
			const char *text = texts[0];
			for (int len = 0; text[len]; len++) {
				char buf[64];
				int w;
				font_measure_text(f, text, len, false, &w, NULL);
				memcpy(buf, text, len);
				buf[len] = '\0';
				ASSERT_EQUAL(w, measure_w(f, buf, false));
			}
		}
	}
}

static bool surfaces_differ(SDL_Surface *a, SDL_Surface *b) {
	if (a->w != b->w || a->h != b->h || sdl_SurfaceFormat(a) != sdl_SurfaceFormat(b))
		return true;
	int row_bytes = a->w * sdl_SurfaceBytesPerPixel(a);
	for (int y = 0; y < a->h; y++) {
		const uint8_t *ap = (uint8_t *)a->pixels + y * a->pitch;
		const uint8_t *bp = (uint8_t *)b->pixels + y * b->pitch;
		if (memcmp(ap, bp, row_bytes) != 0)
			return true;
	}
	return false;
}

static bool surface_has_ink(SDL_Surface *s) {
	int bpp = sdl_SurfaceBytesPerPixel(s);
	for (int y = 0; y < s->h; y++) {
		const uint8_t *p = (uint8_t *)s->pixels + y * s->pitch;
		for (int x = 0; x < s->w; x++) {
			if (bpp == 4) {
				if ((*(const uint32_t *)(p + x * 4) >> 24) != 0)
					return true;
			} else {
				if (p[x] != 0)
					return true;
			}
		}
	}
	return false;
}

static int first_ink_row(SDL_Surface *s) {
	int bpp = sdl_SurfaceBytesPerPixel(s);
	for (int y = 0; y < s->h; y++) {
		const uint8_t *p = (uint8_t *)s->pixels + y * s->pitch;
		for (int x = 0; x < s->w; x++) {
			if (bpp == 4 ? (*(const uint32_t *)(p + x * 4) >> 24) != 0 : p[x] != 0)
				return y;
		}
	}
	return -1;
}

// gfx_drawString() places the surface `font_cell_overhang()` pixels above the
// requested y. A full-width ideograph must start near the top of the cell, as
// with MS Gothic / MS Mincho, whatever font renders it.
static void assert_ideograph_at_cell_top(FontSpec f, const char *ideograph) {
	SDL_Surface *s = font_render_text(f, ideograph, white, false);
	ASSERT_TRUE(s);
	int top = first_ink_row(s) - font_cell_overhang(f);
	ASSERT_TRUE(top >= 0);
	ASSERT_TRUE(top <= f.size / 4 + 1);
	SDL_DestroySurface(s);
}

// Mirrors get_fallback_face(), which only accepts a fallback that has U+8FD9.
static bool font_file_has_sc(const char *path) {
	FT_Library lib;
	if (FT_Init_FreeType(&lib))
		return false;
	bool found = false;
	FT_Face face;
	if (!FT_New_Face(lib, path, 0, &face)) {
		found = FT_Get_Char_Index(face, 0x8fd9) != 0;
		FT_Done_Face(face);
	}
	FT_Done_FreeType(lib);
	return found;
}

static bool has_system_sc_fallback(void) {
	const char *env_fallback = getenv("XSYSTEM35_FONT_FALLBACK");
	if (env_fallback && env_fallback[0] && font_file_has_sc(env_fallback))
		return true;
	static const char *const paths[] = {
		"/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
		"/usr/share/fonts/noto-cjk/NotoSansCJK-Regular.ttc",
		"/usr/share/fonts/truetype/NotoSansCJK-Regular.ttc",
		"/usr/share/fonts/truetype/noto/NotoSansSC-Regular.otf",
		"/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf",
		NULL
	};
	for (int i = 0; paths[i]; i++) {
		if (access(paths[i], R_OK) == 0)
			return true;
	}
	return false;
}

static void utf8_encoding_test(void) {
	static const char *const primary_sample =
		"Hello, \xe3\x81\x82\xe3\x81\x84\xe3\x81\x86 \xe9\xad\x94\xe7\x89\xa9";  // Hello, あいう 魔物
	static const char *const sc_text =
		"\xe9\xac\xbc\xe7\x95\x9c\xe7\x8e\x8b\xe5\x85\xb0\xe6\x96\xaf";  // 鬼畜王兰斯
	static const char *const sc_zhe = "\xe8\xbf\x99";  // 这 (U+8FD9, missing from MTLc3m/mincho)
	bool has_sc = has_system_sc_fallback();

	for (int ti = 0; ti < NTYPES; ti++) {
		FontSpec f = {font_types[ti], FONT_WEIGHT_NORMAL, 24};

		font_set_encoding(SHIFT_JIS);
		SDL_Surface *sjis_primary = font_render_text(f, primary_sample, white, false);
		ASSERT_TRUE(sjis_primary);

		// UTF-8 must preserve the configured primary face even when it lacks U+8FD9.
		font_set_encoding(UTF8);
		SDL_Surface *utf8_primary = font_render_text(f, primary_sample, white, false);
		ASSERT_TRUE(utf8_primary);
		assert_same_pixels(sjis_primary, utf8_primary);

		// Simplified Chinese missing from the primary font still falls back per-glyph in UTF-8.
		SDL_Surface *utf8_sc = font_render_text(f, sc_text, white, false);
		SDL_Surface *utf8_zhe = font_render_text(f, sc_zhe, white, false);
		ASSERT_TRUE(utf8_sc);
		ASSERT_TRUE(utf8_zhe);
		ASSERT_TRUE(utf8_sc->w > 0 && utf8_sc->h > 0);
		ASSERT_TRUE(utf8_zhe->w > 0 && utf8_zhe->h > 0);
		if (has_sc) {
			ASSERT_TRUE(surface_has_ink(utf8_sc));
			ASSERT_TRUE(surface_has_ink(utf8_zhe));
			SDL_Surface *utf8_unmapped = font_render_text(f, "\xef\xbf\xbf", white, false);  // U+FFFF
			ASSERT_TRUE(utf8_unmapped);
			ASSERT_TRUE(surfaces_differ(utf8_zhe, utf8_unmapped));
			SDL_DestroySurface(utf8_unmapped);
		}

		// GBK still selects the SC fallback face as primary when configured font lacks U+8FD9.
		font_set_encoding(GBK);
		SDL_Surface *gbk_primary = font_render_text(f, primary_sample, white, false);
		SDL_Surface *gbk_sc = font_render_text(f, sc_text, white, false);
		SDL_Surface *gbk_zhe = font_render_text(f, sc_zhe, white, false);
		ASSERT_TRUE(gbk_primary);
		ASSERT_TRUE(gbk_sc);
		ASSERT_TRUE(gbk_zhe);
		if (has_sc) {
			ASSERT_TRUE(surfaces_differ(sjis_primary, gbk_primary));
			ASSERT_TRUE(surface_has_ink(gbk_sc));
			ASSERT_TRUE(surface_has_ink(gbk_zhe));
		}

		if (has_sc) {
			static const char *const kou = "\xe5\x8f\xa3";  // 口
			for (int si = 0; si < NSIZES; si++) {
				FontSpec fs = {font_types[ti], FONT_WEIGHT_NORMAL, sizes[si]};
				font_set_encoding(SHIFT_JIS);
				assert_ideograph_at_cell_top(fs, kou);
				font_set_encoding(GBK);
				assert_ideograph_at_cell_top(fs, kou);
			}
		}

		SDL_DestroySurface(sjis_primary);
		SDL_DestroySurface(utf8_primary);
		SDL_DestroySurface(utf8_sc);
		SDL_DestroySurface(utf8_zhe);
		SDL_DestroySurface(gbk_primary);
		SDL_DestroySurface(gbk_sc);
		SDL_DestroySurface(gbk_zhe);
	}

	if (has_sc) {
		// Missing configured font (e.g. QiushuiShotai.ttf absent) falls back cleanly even if
		// initially opened before switching from SHIFT_JIS to UTF8.
		FontSpec f = {FONT_GOTHIC, FONT_WEIGHT_NORMAL, 24};
		font_set_encoding(SHIFT_JIS);
		font_set_name_and_index(FONT_GOTHIC, "/nonexistent/QiushuiShotai.ttf", 0);
		SDL_Surface *s1 = font_render_text(f, "Hello", white, false);
		ASSERT_TRUE(s1);
		SDL_DestroySurface(s1);

		font_set_encoding(UTF8);
		SDL_Surface *s2 = font_render_text(f, sc_text, white, false);
		SDL_Surface *s3 = font_render_text(f, sc_zhe, white, false);
		ASSERT_TRUE(s2 && s2->w > 0 && surface_has_ink(s2));
		ASSERT_TRUE(s3 && s3->w > 0 && surface_has_ink(s3));
		SDL_DestroySurface(s2);
		SDL_DestroySurface(s3);

		font_set_name_and_index(FONT_GOTHIC, TEST_FONT_DIR "/MTLc3m.ttf", 0);
	}

	font_set_encoding(SHIFT_JIS);
}

void font_test(void) {
	font_init();
	font_set_name_and_index(FONT_GOTHIC, TEST_FONT_DIR "/MTLc3m.ttf", 0);
	font_set_name_and_index(FONT_MINCHO, TEST_FONT_DIR "/mincho.ttf", 0);

	invariants_test();
	glyph_cache_test();
	utf8_encoding_test();
}
