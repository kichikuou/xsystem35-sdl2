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

#include <SDL.h>

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

static int measure_w(FontSpec f, const char *s) {
	int w;
	font_measure_text(f, s, -1, &w, NULL);
	return w;
}

static int measure_h(FontSpec f) {
	int h;
	font_measure_text(f, "", -1, NULL, &h);
	return h;
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
					ASSERT_EQUAL(s->w, measure_w(f, text));
					// microui lays out text rows by the measured height,
					// so the surface must be exactly that tall.
					ASSERT_EQUAL(s->h, measure_h(f));
					// The antialiased surface goes to gfx_drawAntiAlias_8bpp(),
					// which reads a 32-bit alpha channel. The other one is
					// blitted onto the indexed screen surface as is.
					ASSERT_EQUAL(s->format->BitsPerPixel, aa ? 32 : 8);
					SDL_FreeSurface(s);
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
				font_measure_text(f, text, len, &w, NULL);
				memcpy(buf, text, len);
				buf[len] = '\0';
				ASSERT_EQUAL(w, measure_w(f, buf));
			}
		}
	}
}

void font_test(void) {
	font_init();
	font_set_name_and_index(FONT_GOTHIC, TEST_FONT_DIR "/MTLc3m.ttf", 0);
	font_set_name_and_index(FONT_MINCHO, TEST_FONT_DIR "/mincho.ttf", 0);

	invariants_test();
}
