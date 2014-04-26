/*
 * jpeg2surface.c: 
 *
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
/* $Id: jpeg2surface.c,v 1.2 2003/11/09 15:06:12 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <glib.h>
#include <jpeglib.h>

#include "portab.h"
#include "ags.h"
#include "surface.h"
#include "ngraph.h"

static void ppm2surface(surface_t *sf, int curscanline, unsigned char *src, int width) {
	BYTE *dp = GETOFFSET_PIXEL(sf, 0, curscanline);
	
	switch (sf->depth) {
	case 15:
	{
		WORD *dst = (WORD *)dp;
		while (width--) {
			*dst = PIX15(*src, *(src +1), *(src +2));
			dst++; src+= 3;
		}
		break;
		
	}
	case 16:
	{
		WORD *dst = (WORD *)dp;
		while (width--) {
			*dst = PIX16(*src, *(src +1), *(src +2));
			dst++; src+= 3;
		}
		break;
	}
	case 24:
	case 32:
	{
		DWORD *dst = (DWORD *)dp;
		while (width--) {
			*dst = PIX24(*src, *(src +1), *(src +2));
			dst++; src+= 3;
		}
		break;
	}}
}

// sf0 に直接描く(それでも遅いけど)
surface_t *jpeg2surface(FILE *fp, int offset) {
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	int row_stride;
	surface_t *sf;
	JSAMPARRAY buffer;
	
	fseek(fp, offset, SEEK_SET);

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	
	jpeg_stdio_src(&cinfo, fp);
	jpeg_read_header(&cinfo, TRUE);
	
	cinfo.do_fancy_upsampling = FALSE;
	cinfo.do_block_smoothing  = FALSE;
	
	jpeg_start_decompress(&cinfo);
	//fprintf(stderr, "width = %d, height = %d\n", cinfo.output_width, cinfo.output_height);
	
	//sf = sf_create_pixel(cinfo.output_width, cinfo.output_height, sf0->depth);
	sf = sf0;
	
	row_stride = cinfo.output_width * cinfo.output_components;
	buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
	
	while (cinfo.output_scanline < cinfo.output_height) {
		jpeg_read_scanlines(&cinfo, buffer, 1);
		ppm2surface(sf, cinfo.output_scanline -1, buffer[0], cinfo.output_width);
	}
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	
	return sf;
}
