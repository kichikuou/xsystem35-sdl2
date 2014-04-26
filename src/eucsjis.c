/*
 * eucsjis.c -- euc/sjis related function
 *
 * Copyright (C) 1997 Yutaka OIWA <oiwa@is.s.u-tokyo.ac.jp>
 *
 * written for Satoshi KURAMOCHI's "eplaymidi"
 *                                   <satoshi@ueda.info.waseda.ac.jp>
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
/* $Id: eucsjis.c,v 1.2 2000/09/20 10:33:16 chikama Exp $ */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "portab.h"
#include "eucsjis.h"

/* SJIS から EUC への変換 */
BYTE *sjis2euc(BYTE *src) {
	BYTE *dst , *_dst;
		
	dst = _dst = malloc(strlen(src) * 2 + 1);
	if (dst == NULL) return NULL;
	
	while(*src) {
		if (*src < 0x81)
			*dst++ = *src++;
		else if (*src >= 0xa0 && *src <= 0xdf) {
			/* JIS X0201 katakana */
			*dst++ = 0x8e; /* ISO-2022 SS2 */
			*dst++ = *src++;
		} else {
			unsigned char c1, c2;
			c1 = *src++;
			if (!*src) {
				*dst++ = '*';
				break;
			}
			c2 = *src++;
			if (c1 >= 0xe0)
				c1 -= 0x40;
			c1 -= 0x81;
			if (c2 >= 0x80)
				c2--;
			c2 -= 0x40;
			
			if (c2 >= 94*2 || c1 > 94/2) {
				/* invalid code */
				*dst++ = '*';
				*dst++ = '*';
				continue;
			}
			c1 *= 2;
			if (c2 >= 94) {
				c2 -= 94;
				c1++;
			}
			*dst++ = 0xa1 + c1;
			*dst++ = 0xa1 + c2;
		}
	}
	*dst = '\0';
	return _dst;
}

static void _jis_shift(int *p1, int *p2) {
	unsigned char c1 = *p1;
	unsigned char c2 = *p2;
	int rowOffset = c1 < 95 ? 112 : 176;
	int cellOffset = c1 % 2 ? (c2 > 95 ? 32 : 31) : 126;
	
	*p1 = ((c1 + 1) >> 1) + rowOffset;
	*p2 += cellOffset;
}

/* EUC から SJIS への変換 */
BYTE *euc2sjis(BYTE* src) {
	BYTE *dst , *_dst;
		
	dst = _dst = malloc(strlen(src) +1);
	if (dst == NULL) return NULL;
	
	while(*src) {
		if (*src < 0x81) {
			*dst++ = *src++;
		} else if (*src == 0x8e) {
			src++;
			*dst++ = *src++;
		} else {
			int c1, c2;
			c1 = *src++;
			c2 = *src++;
			c1 -= 128;
			c2 -= 128;
			_jis_shift(&c1, &c2);
			*dst++= (char)c1;
			*dst++= (char)c2;
		}
	}
	*dst = '\0';
	return _dst;
}

/* src 内に半角カナもしくはASCII文字があるかどうか */
boolean sjis_has_hankaku(BYTE *src) {
	while(*src) {
		if (CHECKSJIS1BYTE(*src)) {
			src++;
		} else {
			return TRUE;
		}
		src++;
	}
	return FALSE;
}

/* src 内に 全角文字があるかどうか */
boolean sjis_has_zenkaku(BYTE *src) {
	while(*src) {
		if (CHECKSJIS1BYTE(*src)) {
			return TRUE;
		}
		src++;
	}
	return FALSE;
}

/* src 中の文字数を数える 全角文字も１文字 */
int sjis_count_char(BYTE *src) {
	int c = 0;
	
	while(*src) {
		if (CHECKSJIS1BYTE(*src)) {
			src++;
		}
		c++; src++;
	}
	return c;
}

/* SJIS(EUC) を含む文字列の ASCII を大文字化する */
void sjis_toupper(BYTE *src) {
	while(*src) {
		if (CHECKSJIS1BYTE(*src)) {
			src++;
		} else {
			if (*src >= 0x60 && *src <= 0x7a) {
				*src &= 0xdf;
			}
		}
		src++;
	}
}

/* SJIS を含む文字列の ASCII を大文字化する2 */
BYTE *sjis_toupper2(BYTE *src) {
	BYTE *dst;
		
	dst = malloc(strlen(src) +1);
	if (dst == NULL) return NULL;
	strcpy(dst, src);
	sjis_toupper(dst);
	return dst;
}
