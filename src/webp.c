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

#include <webp/decode.h>

#include "system.h"
#include "webp.h"
#include "dri.h"
#include "ald_manager.h"
#include "LittleEndian.h"

boolean webp_checkfmt(BYTE *data) {
	if (data[0] != 'R' || data[1] != 'I' || data[2] != 'F' || data[3] != 'F')
		return FALSE;
	if (data[8] != 'W' || data[9] != 'E' || data[10] != 'B' || data[11] != 'P')
		return FALSE;
	return TRUE;
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

static uint8_t *webp_load(uint8_t *data, size_t size, int *width, int *height, int *has_alpha) {
	uint8_t *pixels = WebPDecodeRGBA(data, size, width, height);
	if (!pixels) {
		return NULL;
	}

	WebPBitstreamFeatures features;
	WebPGetFeatures(data, size, &features);
	*has_alpha = features.has_alpha;

	int base = get_base_cg(data, size);
	if (!base)
		return pixels;

	dridata *dfile = ald_getdata(DRIFILE_CG, base - 1);
	if (!dfile) {
		WARNING("Cannot load base CG %d\n", base - 1);
		return pixels;
	}

	int base_width, base_height, base_has_alpha;
	uint8_t *base_pixels = webp_load(dfile->data, dfile->size, &base_width, &base_height, &base_has_alpha);
	ald_freedata(dfile);
	if (!base_pixels) {
		WARNING("Cannot decode base CG %d\n", base - 1);
		return pixels;
	}
	if (*width != base_width || *height != base_height) {
		WARNING("webp base CG dimensions don't match: (%d,%d) / (%d,%d)\n",
				base_width, base_height, *width, *height);
		WebPFree(base_pixels);
		return pixels;
	}
	*has_alpha |= base_has_alpha;

	uint8_t *p = pixels;
	uint8_t *bp = base_pixels;
	for (int n = base_width * base_height; n > 0; n--) {
		if (p[0] == 255 && p[1] == 0 && p[2] == 255) {
			p[0] = bp[0];
			p[1] = bp[1];
			p[2] = bp[2];
			p[3] = bp[3];
		}
		p += 4;
		bp += 4;
	}
	WebPFree(base_pixels);
	return pixels;
}

static void rgba_to_rgb_and_alpha(uint8_t *rgba, uint8_t *rgb, uint8_t *alpha, int nr_pixels) {
	for (int i = 0; i < nr_pixels; i++) {
		*rgb++ = *rgba++;
		*rgb++ = *rgba++;
		*rgb++ = *rgba++;
		*alpha++ = *rgba++;
	}
}

cgdata *webp_extract(BYTE *data, size_t size) {
	int width, height, has_alpha;
	uint8_t *rgba = webp_load(data, size, &width, &height, &has_alpha);
	if (!rgba) {
		WARNING("webp image decode failed");
		return NULL;
	}

	cgdata *cg = calloc(1, sizeof(cgdata));
	cg->type = ALCG_WEBP;
	cg->width = width;
	cg->height = height;
	cg->pic = malloc(cg->width * cg->height * 3);
	cg->alpha = malloc(cg->width * cg->height);
	rgba_to_rgb_and_alpha(rgba, cg->pic, cg->alpha, cg->width * cg->height);
	if (!has_alpha) {
		free(cg->alpha);
		cg->alpha = NULL;
	}
	WebPFree(rgba);

	return cg;
}
