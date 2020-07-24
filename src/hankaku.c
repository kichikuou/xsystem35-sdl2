/*
 * hankaku.c  全角->半角変換
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
 *
*/
/* $Id: hankaku.c,v 1.7 2001/03/22 11:10:13 chikama Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "portab.h"
#include "hankaku.h"

static const BYTE hankakutable[3][192] = {{
	// 0x8140 - 0x81ff
	 ' ', 0xa4, 0xa1,  ',',  '.',  0xa5, ':',  ';',
	 '?',  '!', 0xde, 0xdf,    0,  '`',    0,  '^',
	 '~',  '_',    0,    0,    0,    0,    0,    0,
	   0,    0,    0, 0xb0,    0,    0,  '/',    0,
	   0,    0,  '|',    0,    0,    0, '\'',    0,
	 '"',  '(',  ')',    0,    0,  '[',  ']',  '{',
	 '}',    0,    0,    0,    0, 0xa2, 0xa3,    0,
	   0,    0,    0,  '+',  '-',    0,    0,    0,
	   0,  '=',    0,  '<',  '>',    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0, '\\',
	 '$',    0,    0,  '%',  '#',  '&',  '*',  '@',
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
},{  // 0x8240 - 0x82ff
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,  '0',
	 '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',
	 '9',    0,    0,    0,    0,    0,    0,    0,
	 'A',  'B',  'C',  'D',  'E',  'F',  'G',  'H',
	 'I',  'J',  'K',  'L',  'M',  'N',  'O',  'P',
	 'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',
	 'Y',  'Z',    0,    0,    0,    0,    0,    0,
	   0,  'a',  'b',  'c',  'd',  'e',  'f',  'g',
	 'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o',
	 'p',  'q',  'r',  's',  't',  'u',  'v',  'w',
	 'x',  'y',  'z',    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
},{  // 0x8340 - 0x83ff
	0xa7, 0xb1, 0xa8, 0xb2, 0xa9, 0xb3, 0xaa, 0xb4,
	0xab, 0xb5, 0xb6,    0, 0xb7,    0, 0xb8,    0,
	0xb9,    0, 0xba,    0, 0xbb,    0, 0xbc,    0,
	0xbd,    0, 0xbe,    0, 0xbf,    0, 0xc0,    0,
	0xc1,    0, 0xaf, 0xc2,    0, 0xc3,    0, 0xc4,
	   0, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca,    0,
	   0, 0xcb,    0,    0, 0xcc,    0,    0, 0xcd,
	   0,    0, 0xce,    0,    0, 0xcf, 0xd0,    0,
	0xd1, 0xd2, 0xd3, 0xac, 0xd4, 0xad, 0xd5, 0xae,
	0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb,    0, 0xdc,
	   0,    0, 0xa6, 0xdd,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
	   0,    0,    0,    0,    0,    0,    0,    0,
}};

static const BYTE kanatbl[][2] = {
	0x81, 0x40, 0x81, 0x42, 0x81, 0x75, 0x81, 0x76,
	0x81, 0x41, 0x81, 0x45, 0x82, 0xf0, 0x82, 0x9f,
	0x82, 0xa1, 0x82, 0xa3, 0x82, 0xa5, 0x82, 0xa7,
	0x82, 0xe1, 0x82, 0xe3, 0x82, 0xe5, 0x82, 0xc1,
	0x81, 0x5b, 0x82, 0xa0, 0x82, 0xa2, 0x82, 0xa4,
	0x82, 0xa6, 0x82, 0xa8, 0x82, 0xa9, 0x82, 0xab,
	0x82, 0xad, 0x82, 0xaf, 0x82, 0xb1, 0x82, 0xb3,
	0x82, 0xb5, 0x82, 0xb7, 0x82, 0xb9, 0x82, 0xbb,
	0x82, 0xbd, 0x82, 0xbf, 0x82, 0xc2, 0x82, 0xc4,
	0x82, 0xc6, 0x82, 0xc8, 0x82, 0xc9, 0x82, 0xca,
	0x82, 0xcb, 0x82, 0xcc, 0x82, 0xcd, 0x82, 0xd0,
	0x82, 0xd3, 0x82, 0xd6, 0x82, 0xd9, 0x82, 0xdc,
	0x82, 0xdd, 0x82, 0xde, 0x82, 0xdf, 0x82, 0xe0,
	0x82, 0xe2, 0x82, 0xe4, 0x82, 0xe6, 0x82, 0xe7,
	0x82, 0xe8, 0x82, 0xe9, 0x82, 0xea, 0x82, 0xeb,
	0x82, 0xed, 0x82, 0xf1, 0x81, 0x4a, 0x81, 0x4b
};

static const char zenkaku_digits[][3] = {
	0x82,0x4f,0x00, /* 0 */
	0x82,0x50,0x00, /* 1 */
	0x82,0x51,0x00, /* 2 */
	0x82,0x52,0x00, /* 3 */
	0x82,0x53,0x00, /* 4 */
	0x82,0x54,0x00, /* 5 */
	0x82,0x55,0x00, /* 6 */
	0x82,0x56,0x00, /* 7 */
	0x82,0x57,0x00, /* 8 */
	0x82,0x58,0x00, /* 9 */
	0x81,0x40,0x00, /* space */
};

BYTE *zen2han(const BYTE *src) {
	BYTE c0, c1;
	char *dst, *_dst;
	
	dst = _dst = malloc(strlen(src) + 1);
	if (dst == NULL) {
		fprintf(stderr, "zen2han(): Out of Memory (size %zu)", strlen(src) + 1);
		return NULL;
	}
	
	while(0 != (c0 = *src++)) {
		if (c0 < 0x81) {
			*dst++ = c0;
		} else if (c0 <= 0x83) {
			c1 = *src++;
			BYTE h = hankakutable[c0 - 0x81][c1 - 0x40];
			if (h) {
				*dst++ = h;
			} else {
				*dst++ = c0; *dst++ = c1;
			}
		} else if (c0 < 0xa0){
			*dst++ = c0; *dst++ = *src++;
		} else if (c0 < 0xe0){
			*dst++ = c0;
		} else {
			*dst++ = c0; *dst++ = *src++;
		}
	}
	*dst = 0;
	return _dst;
}

BYTE *han2zen(const BYTE *src) {
	BYTE c0;
	BYTE *dst, *_dst;
	dst = _dst = malloc(strlen(src) * 2 + 1);
	
	if (dst == NULL) {
		fprintf(stderr, "han2zen(): Out of Memory (size %zu)", strlen(src) *2 + 1);
		return NULL;
	}
	
	while(0 != (c0 = *src++)) {
		if (c0 == 0x20) {
			*dst++ = 0x81; *dst++ = 0x40;
		} else if (c0 < 0x80) {
			*dst++ = c0;
		} else if (c0 >= 0xe0) {
			*dst++ = (char)c0; *dst++ = *src++;
		} else if (c0 >= 0xa0) {
			const BYTE *kindex = kanatbl[c0 - 0xa0];
			*dst++ = *kindex; *dst++ = *(kindex+1);
		} else {
			*dst++ = (char)c0; *dst++ = *src++;
		}
	}
	*dst = 0;
	return _dst;
}

char *format_number(int n, int width, char *buf) {
	if (width) {
		sprintf(buf, "%*d", width, n);
		return buf + strlen(buf) - width;
	}
	sprintf(buf, "%d", n);
	return buf;
}

char *format_number_zenkaku(int n, int width, char *buf) {
	char work[256];
	char *dst = buf;
	for (char *s = format_number(n, width, work); *s; s++) {
		const char *p = zenkaku_digits[*s == ' ' ? 10 : *s - '0'];
		while (*p)
			*dst++ = *p++;
	}
	*dst = '\0';
	return buf;
}
