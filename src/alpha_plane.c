/*
 * alpha_plane.c  alpha plane
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
/* $Id: alpha_plane.c,v 1.4 2001/12/16 17:12:55 chikama Exp $ */

#include <string.h>

#include "portab.h"
#include "ags.h"
#include "alpha_plane.h"

/*
 * Copy alpha pixel from other alpha plane
 *   dst : destination  surface
 *   dx  : location x to be copied
 *   dy  : location y to be copied
 *   dw  : width to be copied
 *   dh  : height to be copied
 *   src : source alpha plane pixel
 *   src_pitch: soruce alpha plane pitch
*/
void alpha_set_pixels(surface_t *dst, int dx, int dy, int w, int h, uint8_t *src, int src_pitch) {
	uint8_t *ad = GETOFFSET_ALPHA(dst, dx, dy);
	uint8_t *as = src;
	int i;
	
	for (i = 0; i < h; i++) {
		memcpy(ad, as, w);
		ad += dst->width;
		as += src_pitch;
	}
}

/*
 * Get pixel from alpha plane
 *   suf: target surface
 *   x  : location x
 *   y  : location y
 *   pic: acired alpha pixel
*/
void alpha_get_pixel(surface_t *suf, int x, int y, uint8_t *pic) {
	*pic = *GETOFFSET_ALPHA(suf, x, y);
}

/*
 * Set pixel level to 'd' which lower than 's'
 *   suf: target surface
 *   sx : location x
 *   sy : location y
 *   w  : width
 *   h  : height
 *   s  : top level to be cut
 *   d  : setteled level
*/
void alpha_lowercut(surface_t *suf, int sx, int sy, int w, int h, int s, int d) {
	uint8_t *a = GETOFFSET_ALPHA(suf, sx, sy), *b;
	int x, y;
	
	for (y = 0; y < h; y++) {
		b = a + y * suf->width;
		for (x = 0; x < w; x++) {
			if (*b <= (uint8_t)s) *b = (uint8_t)d;
			b++;
		}
	}
}

/*
 * Set pixel level to 'd' which lower than 's'
 *   suf: target surface
 *   sx : location x
 *   sy : location y
 *   w  : width
 *   h  : height
 *   s  : bottom level to be cut
 *   d  : setteled level
*/
void alpha_uppercut(surface_t *suf, int sx, int sy, int w, int h, int s, int d) {
	uint8_t *dp = GETOFFSET_ALPHA(suf, sx, sy);
	int x, y;
	
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			if (*(dp + x) >= (uint8_t)s) {
				*(dp + x) = (uint8_t)d;
			}
		}
		dp += suf->width;
	}
}

/*
 * Set alpha level in region
 *   suf: target surface
 *   sx : region x
 *   sy : region y
 *   w  : region width
 *   h  : region height
 *   lv : level to be set
*/
void alpha_set_level(surface_t *suf, int sx, int sy, int w, int h, int lv) {
	uint8_t *a = GETOFFSET_ALPHA(suf, sx, sy);
	
	while(h--){
		memset(a, lv, w);
		a += suf->width;
	}
}

/*
 * Copy alpha plane
 *   suf: target surface
 *   sx : source x
 *   sy : source y
 *   w  : source width
 *   h  : source height
 *   dx : destination x
 *   dy : destination y
*/
void alpha_copy_area(surface_t *suf, int sx, int sy, int w, int h, int dx, int dy) {
	uint8_t *src = GETOFFSET_ALPHA(suf, sx, sy);
	uint8_t *dst = GETOFFSET_ALPHA(suf, dx, dy);
	
	if (sy <= dy && dy < (sy + h)) {
		src += (h-1) * suf->width;
		dst += (h-1) * suf->width;
		while (h--) {
 			memmove(dst, src, w);
			src -= suf->width;
			dst -= suf->width;
		}
	} else {
	        while(h--) {
			memmove(dst, src, w);
			src += suf->width;
			dst += suf->width;
		}
	}
}
