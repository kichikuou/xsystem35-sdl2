/*
 * virtual_pointer.c  Trackpad-style virtual mouse pointer for touch devices
 *
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
 *
*/

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>

#include "portab.h"
#include "system.h"
#include "nact.h"
#include "event.h"
#include "virtual_pointer.h"

#include "bitmaps/virtual_cursor.xpm"

bool vp_is_enabled(void) {
	return nact->ags.virtualpointer;
}

// Builds an ARGB surface from the (simple, 1 char/pixel) XPM array above.
static SDL_Surface *create_cursor_surface(void) {
	int w, h, ncolors, cpp;
	if (sscanf(virtual_cursor[0], "%d %d %d %d", &w, &h, &ncolors, &cpp) != 4 || cpp != 1) {
		WARNING("virtual_pointer: unexpected cursor bitmap header");
		return NULL;
	}

	// Map each character to an ARGB8888 value.
	Uint32 colormap[256] = {0};
	for (int i = 0; i < ncolors; i++) {
		const char *line = virtual_cursor[1 + i];
		unsigned char ch = (unsigned char)line[0];
		// Format: "<ch> c <value>", value is "#rrggbb" or "None".
		const char *val = line + 4;
		if (val[0] == '#') {
			unsigned int rgb = (unsigned int)strtoul(val + 1, NULL, 16);
			colormap[ch] = 0xff000000u | rgb;  // opaque
		} else {
			colormap[ch] = 0;  // transparent
		}
	}

	SDL_Surface *s = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_ARGB8888);
	if (!s)
		return NULL;
	const char **pixels = &virtual_cursor[1 + ncolors];
	for (int y = 0; y < h; y++) {
		Uint32 *row = (Uint32 *)((Uint8 *)s->pixels + y * s->pitch);
		const char *src = pixels[y];
		for (int x = 0; x < w; x++)
			row[x] = colormap[(unsigned char)src[x]];
	}
	return s;
}

// Lazily-created cursor texture. The hotspot (arrow tip) is at its top-left
// corner, so it is drawn with its origin at the pointer position.
static SDL_Texture *cursor_texture;
static int cursor_w, cursor_h;

void vp_draw(SDL_Renderer *renderer) {
	if (!vp_is_enabled())
		return;

	if (!cursor_texture) {
		SDL_Surface *s = create_cursor_surface();
		if (!s)
			return;
		cursor_w = s->w;
		cursor_h = s->h;
		cursor_texture = SDL_CreateTextureFromSurface(renderer, s);
		SDL_FreeSurface(s);
		if (!cursor_texture)
			return;
		SDL_SetTextureBlendMode(cursor_texture, SDL_BLENDMODE_BLEND);
	}

	int x, y;
	event_get_pointer_pos(&x, &y);
	// The renderer's logical size is set to the game view, so drawing in view
	// coordinates positions the cursor exactly at the game's mouse coordinates.
	SDL_Rect dst = { x, y, cursor_w, cursor_h };
	SDL_RenderCopy(renderer, cursor_texture, NULL, &dst);
}
