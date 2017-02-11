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
#include <glib.h>

#include "portab.h"
#include "eucsjis.h"

BYTE *sjis2utf(BYTE *src) {
	gchar *gbuf = g_convert_with_fallback(src, -1, "utf-8", "shift-jis", NULL, NULL, NULL, NULL);
	BYTE *dst = strdup(gbuf);
	g_free(gbuf);
	return dst;
}

BYTE *utf2sjis(BYTE *src) {
	gchar *gbuf = g_convert_with_fallback(src, -1, "shift-jis", "utf-8", "?", NULL, NULL, NULL);
	BYTE *dst = strdup(gbuf);
	g_free(gbuf);
	return dst;
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
