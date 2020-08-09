/*
 * utfsjis.c -- utf-8/sjis related function
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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "portab.h"
#include "utfsjis.h"
#include "s2utbl.h"

char* codeconv(CharacterEncoding tocode,
			   CharacterEncoding fromcode,
			   const char *str) {
	if (tocode == fromcode)
		return strdup(str);
	if (tocode == UTF8 && fromcode == SHIFT_JIS)
		return (char *)sjis2utf((char *)str);
	if (tocode == SHIFT_JIS && fromcode == UTF8)
		return (char *)utf2sjis((char *)str);
	return NULL;
}

BYTE *sjis2utf(const BYTE *src) {
	BYTE* dst = malloc(strlen(src) * 3 + 1);
	BYTE* dstp = dst;

	while (*src) {
		if (*src <= 0x7f) {
			*dstp++ = *src++;
			continue;
		}

		int c;
		if (*src >= 0xa0 && *src <= 0xdf) {
			c = 0xff60 + *src - 0xa0;
			src++;
		} else {
			c = s2u[*src - 0x80][*(src+1) - 0x40];
			src += 2;
		}

		if (c <= 0x7f) {
			*dstp++ = c;
		} else if (c <= 0x7ff) {
			*dstp++ = 0xc0 | c >> 6;
			*dstp++ = 0x80 | (c & 0x3f);
		} else {
			*dstp++ = 0xe0 | c >> 12;
			*dstp++ = 0x80 | (c >> 6 & 0x3f);
			*dstp++ = 0x80 | (c & 0x3f);
		}
	}
	*dstp = '\0';
	return dst;
}

static int unicode_to_sjis(int u) {
	// U+30FB (KATAKANA MIDDLE DOT) is used as replacement character in
	// s2utbl.h, so needs special treatment.
	if (u == 0x30fb)
		return 0x8145;

	for (int b1 = 0x80; b1 <= 0xff; b1++) {
		if (b1 >= 0xa0 && b1 <= 0xdf)
			continue;
		for (int b2 = 0x40; b2 <= 0xff; b2++) {
			if (u == s2u[b1 - 0x80][b2 - 0x40])
				return b1 << 8 | b2;
		}
	}
	return 0;
}

BYTE *utf2sjis(const BYTE *src) {
	BYTE* dst = malloc(strlen(src) + 1);
	BYTE* dstp = dst;

	while (*src) {
		if (*src <= 0x7f) {
			*dstp++ = *src++;
			continue;
		}

		int u;
		if (*src <= 0xdf) {
			u = (src[0] & 0x1f) << 6 | (src[1] & 0x3f);
			src += 2;
		} else if (*src <= 0xef) {
			u = (src[0] & 0xf) << 12 | (src[1] & 0x3f) << 6 | (src[2] & 0x3f);
			src += 3;
		} else {
			*dstp++ = '?';
			do src++; while ((*src & 0xc0) == 0x80);
			continue;
		}

		if (u > 0xff60 && u <= 0xff9f) {
			*dstp++ = u - 0xff60 + 0xa0;
		} else {
			int c = unicode_to_sjis(u);
			if (c) {
				*dstp++ = c >> 8;
				*dstp++ = c & 0xff;
			} else {
				*dstp++ = '?';
			}
		}
	}
	*dstp = '\0';
	return dst;
}

/* src 内に半角カナもしくはASCII文字があるかどうか */
boolean sjis_has_hankaku(const BYTE *src) {
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
boolean sjis_has_zenkaku(const BYTE *src) {
	while(*src) {
		if (CHECKSJIS1BYTE(*src)) {
			return TRUE;
		}
		src++;
	}
	return FALSE;
}

int utf8_next_codepoint(const char **msg) {
	int code;
	const unsigned char *s = (const unsigned char *)*msg;

	if (*s <= 0x7f) {
		code = *s++;
	} else if (*s <= 0xbf) {
		// Invalid UTF-8 sequence
		code = '?';
		s++;
	} else if (*s <= 0xdf) {
		code = (s[0] & 0x1f) << 6 | (s[1] & 0x3f);
		s += 2;
	} else if (*s <= 0xef) {
		code = (s[0] & 0xf) << 12 | (s[1] & 0x3f) << 6 | (s[2] & 0x3f);
		s += 3;
	} else if (*s <= 0xf7) {
		code = (s[0] & 0x7) << 18 | (s[1] & 0x3f) << 12 | (s[2] & 0x3f) << 6 | (s[3] & 0x3f);
		s += 4;
	} else {
		code = 0xfffd;  // REPLACEMENT CHARACTER
		s++;
		while (0x80 <= *s && *s <= 0xbf)
			s++;
	}
	*msg = (const char *)s;
	return code;
}

char *advance_char(const char *s, CharacterEncoding e) {
	switch (e) {
	case SHIFT_JIS:
		return (char *)s + ((CHECKSJIS1BYTE(*s) && *(s + 1)) ? 2 : 1);
	case UTF8:
		while (UTF8_TRAIL_BYTE(*++s))
			;
		return (char *)s;
	}
	return NULL;
}
