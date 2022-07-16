/*
 * Copyright (C) 2020 <KichikuouChrome@gmail.com>
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

#include <stdlib.h>

#define STBI_NO_STDIO
#define STBI_ONLY_JPEG
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "config.h"

#include "system.h"
#include "jpeg.h"

boolean jpeg_checkfmt(BYTE *data) {
	return data[0] == 0xff && data[1] == 0xd8;
}

cgdata *jpeg_extract(BYTE *data, size_t size) {
	int width, height, channels;
	BYTE *pixels = stbi_load_from_memory(data, size, &width, &height, &channels, 3);
	if (!pixels) {
		WARNING("cannot decode jpeg: %s\n", stbi_failure_reason());
		return NULL;
	}

	cgdata *cg = calloc(1, sizeof(cgdata));
	cg->type = ALCG_JPEG;
	cg->width  = width;
	cg->height = height;
	cg->depth  = 24;
	cg->pic = pixels;
	return cg;
}
