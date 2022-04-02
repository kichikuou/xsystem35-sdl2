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

#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
#undef max
#undef min

#include "config.h"

#include "system.h"
#include "ags.h"
#include "jpeg.h"

boolean jpeg_checkfmt(BYTE *data) {
	return data[0] == 0xff && data[1] == 0xd8;
}

cgdata *jpeg_extract(BYTE *data, size_t size) {
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	// TODO: Error handling
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	jpeg_mem_src(&cinfo, data, size);
	if (jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK) {
		WARNING("jpeg_read_header failed\n");
		jpeg_destroy_decompress(&cinfo);
		return NULL;
	}

	jpeg_start_decompress(&cinfo);

	cgdata *cg = calloc(1, sizeof(cgdata));
	cg->type = ALCG_JPEG;
	cg->width  = cinfo.output_width;
	cg->height = cinfo.output_height;
	cg->pic = malloc(3 * cg->width * cg->height);

	int row_stride = cinfo.output_width * cinfo.output_components;

	while (cinfo.output_scanline < cinfo.output_height) {
		JSAMPROW dst = cg->pic + cinfo.output_scanline * row_stride;
		jpeg_read_scanlines(&cinfo, &dst, 1);
	}
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	return cg;
}
