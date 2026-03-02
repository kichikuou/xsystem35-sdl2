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

#include "config.h"

#include "system.h"
#include "jpeg.h"

// Disable the bicubic chroma filter to avoid color bleeding in Graymerca.
#define NJ_CHROMA_FILTER 0
#include "nanojpeg.c"

bool jpeg_checkfmt(uint8_t *data) {
	return data[0] == 0xff && data[1] == 0xd8;
}

cgdata *jpeg_extract(uint8_t *data, size_t size) {
	njInit();
	nj_result_t err = njDecode(data, size);
	if (err != NJ_OK) {
		WARNING("cannot decode jpeg: %d", err);
		return NULL;
	}
	if (!njIsColor()) {
		WARNING("grayscale jpeg is not supported:");
		njDone();
		return NULL;
	}
	cgdata *cg = calloc(1, sizeof(cgdata));
	cg->type = ALCG_JPEG;
	cg->width  = njGetWidth();
	cg->height = njGetHeight();
	cg->depth  = 24;
	cg->pic = nj.rgb;  // steal the buffer from nj_context_t
	nj.rgb = NULL;
	njDone();
	return cg;
}
