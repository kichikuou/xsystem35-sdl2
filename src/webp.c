/*
 * Copyright (C) 2021 kichikuou <KichikuouChrome@gmail.com>
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

#include <SDL.h>
#include <webp/decode.h>

#include "system.h"
#include "webp.h"
#include "dri.h"
#include "ald_manager.h"
#include "LittleEndian.h"

bool webp_checkfmt(uint8_t *data) {
	if (data[0] != 'R' || data[1] != 'I' || data[2] != 'F' || data[3] != 'F')
		return false;
	if (data[8] != 'W' || data[9] != 'E' || data[10] != 'B' || data[11] != 'P')
		return false;
	return true;
}

static int get_base_cg(uint8_t *data, size_t size) {
	// Skip 'NULL' chunk if any.
	if (size >= 12 &&
		data[size-12] == 'N' &&
		data[size-11] == 'U' &&
		data[size-10] == 'L' &&
		data[size-9] == 'L' &&
		LittleEndian_getDW(data, size - 8) == 4) {
		size -= 12;
	}

	if (size >= 12 &&
		data[size-12] == 'O' &&
		data[size-11] == 'V' &&
		data[size-10] == 'E' &&
		data[size-9] == 'R' &&
		LittleEndian_getDW(data, size - 8) == 4) {
		return LittleEndian_getDW(data, size - 4);
	}
	return 0;
}

SDL_Surface *webp_extract(uint8_t *data, size_t size) {
	WebPBitstreamFeatures features;
	if (WebPGetFeatures(data, size, &features) != VP8_STATUS_OK) {
		return NULL;
	}
	SDL_Surface *sf = SDL_CreateRGBSurfaceWithFormat(
		0, features.width, features.height, 32,
		features.has_alpha ? SDL_PIXELFORMAT_ARGB8888 : SDL_PIXELFORMAT_XRGB8888);
	if (!WebPDecodeBGRAInto(data, size, sf->pixels, features.height * sf->pitch, sf->pitch)) {
		SDL_FreeSurface(sf);
		return NULL;
	}

	int base_no = get_base_cg(data, size);
	if (!base_no)
		return sf;

	dridata *dfile = ald_getdata(DRIFILE_CG, base_no - 1);
	if (!dfile) {
		WARNING("Cannot load base CG %d", base_no - 1);
		return sf;
	}

	SDL_Surface *base = webp_extract(dfile->data, dfile->size);
	ald_freedata(dfile);
	if (!base) {
		WARNING("Cannot decode base CG %d", base_no - 1);
		return sf;
	}
	if (sf->w != base->w || sf->h != base->h) {
		WARNING("webp base CG dimensions don't match: (%d,%d) / (%d,%d)",
			base->w, base->h, sf->w, sf->h);
		SDL_FreeSurface(base);
		return sf;
	}

	for (int y = 0; y < sf->h; y++) {
		uint8_t *p = sf->pixels + y * sf->pitch;
		uint8_t *bp = base->pixels + y * base->pitch;
		for (int x = 0; x < sf->w; x++) {
			if (p[0] == 255 && p[1] == 0 && p[2] == 255) {
				p[0] = bp[0];
				p[1] = bp[1];
				p[2] = bp[2];
				p[3] = bp[3];
			}
			p += 4;
			bp += 4;
		}
	}
	SDL_FreeSurface(base);
	return sf;
}
