/*
 * antialiase.c  make antialiased pattern 
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
/* $Id: antialiase.c,v 1.3 2001/03/19 12:18:39 chikama Exp $ */

#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "portab.h"

static void memadd(BYTE *s, BYTE *d, int w, int h) {
	int x, y;
	
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			*d           += *s;
			*(d+1)       += *s;
			*(d+2)       += *s;
			*(d+w+2)     += *s;
			*(d+w+1+2)   += (*s * 16);
			*(d+w+1+2)   += *s;
			*(d+w+w+4)   += *s;
			*(d+w+w+1+4) += *s;
			*(d+w+w+2+4) += *s;
			d++; s++;
		}
		d+=2;
	}
}

static void memmul(BYTE *s, int mul, int pixel) {
	while(pixel--) {
		*s = min(255, (*s) * mul); s++;
	}
}
 
void aa_make(BYTE *data, int w, int h, int bytes_per_line) {
	int y;
	BYTE *b, *_b, *__b;
	BYTE *d = data;
	
	 b = g_new0(BYTE, w*h);
	_b = g_new0(BYTE, (w+2)*(h+2));
	
	__b = b;
	for (y = 0; y < h; y++) {
		memcpy(__b, d, w);
		__b+=w; d+=bytes_per_line;
	}
	memadd(b, _b, w, h);
	memmul(_b, 16, (w+2)*(h+2));
	
	d = data;
	__b = _b;
	for (y = 0; y < h+2; y++) {
		memcpy(d, __b, w+2);
		__b += (w+2); d+=bytes_per_line;
	}
	g_free(b);
	g_free(_b);
}
