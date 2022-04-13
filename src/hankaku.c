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

static const WORD kanatbl[] = {
	0x8140, 0x8142, 0x8175, 0x8176, 0x8141, 0x8145, 0x82f0, 0x829f,
	0x82a1, 0x82a3, 0x82a5, 0x82a7, 0x82e1, 0x82e3, 0x82e5, 0x82c1,
	0x815b, 0x82a0, 0x82a2, 0x82a4, 0x82a6, 0x82a8, 0x82a9, 0x82ab,
	0x82ad, 0x82af, 0x82b1, 0x82b3, 0x82b5, 0x82b7, 0x82b9, 0x82bb,
	0x82bd, 0x82bf, 0x82c2, 0x82c4, 0x82c6, 0x82c8, 0x82c9, 0x82ca,
	0x82cb, 0x82cc, 0x82cd, 0x82d0, 0x82d3, 0x82d6, 0x82d9, 0x82dc,
	0x82dd, 0x82de, 0x82df, 0x82e0, 0x82e2, 0x82e4, 0x82e6, 0x82e7,
	0x82e8, 0x82e9, 0x82ea, 0x82eb, 0x82ed, 0x82f1, 0x814a, 0x814b
};

static const char zenkaku_digits[][3] = {
	{0x82, 0x4f, 0x00}, /* 0 */
	{0x82, 0x50, 0x00}, /* 1 */
	{0x82, 0x51, 0x00}, /* 2 */
	{0x82, 0x52, 0x00}, /* 3 */
	{0x82, 0x53, 0x00}, /* 4 */
	{0x82, 0x54, 0x00}, /* 5 */
	{0x82, 0x55, 0x00}, /* 6 */
	{0x82, 0x56, 0x00}, /* 7 */
	{0x82, 0x57, 0x00}, /* 8 */
	{0x82, 0x58, 0x00}, /* 9 */
	{0x81, 0x40, 0x00}, /* space */
};

static BYTE *zen2han_sjis(const BYTE *src) {
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

static char *zen2han_utf8(const char *src) {
	char *dst, *_dst;
	dst = _dst = malloc(strlen(src) + 1);
	if (dst == NULL) {
		fprintf(stderr, "zen2han(): Out of Memory (size %zu)", strlen(src) + 1);
		return NULL;
	}

	const char *p = src;
	while (*p) {
		const char *prev = p;
		switch (utf8_next_codepoint(&p)) {
		case L'’': *dst++ = '\''; break;
		case L'”': *dst++ = '"'; break;
		case L'　': *dst++ = ' '; break;
		case L'、': strcpy(dst, "､"); dst += 3; break;
		case L'。': strcpy(dst, "｡"); dst += 3; break;
		case L'「': strcpy(dst, "｢"); dst += 3; break;
		case L'」': strcpy(dst, "｣"); dst += 3; break;
		case L'゛': strcpy(dst, "ﾞ"); dst += 3; break;
		case L'゜': strcpy(dst, "ﾟ"); dst += 3; break;
		case L'ァ': strcpy(dst, "ｧ"); dst += 3; break;
		case L'ア': strcpy(dst, "ｱ"); dst += 3; break;
		case L'ィ': strcpy(dst, "ｨ"); dst += 3; break;
		case L'イ': strcpy(dst, "ｲ"); dst += 3; break;
		case L'ゥ': strcpy(dst, "ｩ"); dst += 3; break;
		case L'ウ': strcpy(dst, "ｳ"); dst += 3; break;
		case L'ェ': strcpy(dst, "ｪ"); dst += 3; break;
		case L'エ': strcpy(dst, "ｴ"); dst += 3; break;
		case L'ォ': strcpy(dst, "ｫ"); dst += 3; break;
		case L'オ': strcpy(dst, "ｵ"); dst += 3; break;
		case L'カ': strcpy(dst, "ｶ"); dst += 3; break;
		case L'キ': strcpy(dst, "ｷ"); dst += 3; break;
		case L'ク': strcpy(dst, "ｸ"); dst += 3; break;
		case L'ケ': strcpy(dst, "ｹ"); dst += 3; break;
		case L'コ': strcpy(dst, "ｺ"); dst += 3; break;
		case L'サ': strcpy(dst, "ｻ"); dst += 3; break;
		case L'シ': strcpy(dst, "ｼ"); dst += 3; break;
		case L'ス': strcpy(dst, "ｽ"); dst += 3; break;
		case L'セ': strcpy(dst, "ｾ"); dst += 3; break;
		case L'ソ': strcpy(dst, "ｿ"); dst += 3; break;
		case L'タ': strcpy(dst, "ﾀ"); dst += 3; break;
		case L'チ': strcpy(dst, "ﾁ"); dst += 3; break;
		case L'ッ': strcpy(dst, "ｯ"); dst += 3; break;
		case L'ツ': strcpy(dst, "ﾂ"); dst += 3; break;
		case L'テ': strcpy(dst, "ﾃ"); dst += 3; break;
		case L'ト': strcpy(dst, "ﾄ"); dst += 3; break;
		case L'ナ': strcpy(dst, "ﾅ"); dst += 3; break;
		case L'ニ': strcpy(dst, "ﾆ"); dst += 3; break;
		case L'ヌ': strcpy(dst, "ﾇ"); dst += 3; break;
		case L'ネ': strcpy(dst, "ﾈ"); dst += 3; break;
		case L'ノ': strcpy(dst, "ﾉ"); dst += 3; break;
		case L'ハ': strcpy(dst, "ﾊ"); dst += 3; break;
		case L'ヒ': strcpy(dst, "ﾋ"); dst += 3; break;
		case L'フ': strcpy(dst, "ﾌ"); dst += 3; break;
		case L'ヘ': strcpy(dst, "ﾍ"); dst += 3; break;
		case L'ホ': strcpy(dst, "ﾎ"); dst += 3; break;
		case L'マ': strcpy(dst, "ﾏ"); dst += 3; break;
		case L'ミ': strcpy(dst, "ﾐ"); dst += 3; break;
		case L'ム': strcpy(dst, "ﾑ"); dst += 3; break;
		case L'メ': strcpy(dst, "ﾒ"); dst += 3; break;
		case L'モ': strcpy(dst, "ﾓ"); dst += 3; break;
		case L'ャ': strcpy(dst, "ｬ"); dst += 3; break;
		case L'ヤ': strcpy(dst, "ﾔ"); dst += 3; break;
		case L'ュ': strcpy(dst, "ｭ"); dst += 3; break;
		case L'ユ': strcpy(dst, "ﾕ"); dst += 3; break;
		case L'ョ': strcpy(dst, "ｮ"); dst += 3; break;
		case L'ヨ': strcpy(dst, "ﾖ"); dst += 3; break;
		case L'ラ': strcpy(dst, "ﾗ"); dst += 3; break;
		case L'リ': strcpy(dst, "ﾘ"); dst += 3; break;
		case L'ル': strcpy(dst, "ﾙ"); dst += 3; break;
		case L'レ': strcpy(dst, "ﾚ"); dst += 3; break;
		case L'ロ': strcpy(dst, "ﾛ"); dst += 3; break;
		case L'ワ': strcpy(dst, "ﾜ"); dst += 3; break;
		case L'ヲ': strcpy(dst, "ｦ"); dst += 3; break;
		case L'ン': strcpy(dst, "ﾝ"); dst += 3; break;
		case L'・': strcpy(dst, "･"); dst += 3; break;
		case L'ー': strcpy(dst, "ｰ"); dst += 3; break;
		case L'！': *dst++ = '!'; break;
		case L'＃': *dst++ = '#'; break;
		case L'＄': *dst++ = '$'; break;
		case L'％': *dst++ = '%'; break;
		case L'＆': *dst++ = '&'; break;
		case L'（': *dst++ = '('; break;
		case L'）': *dst++ = ')'; break;
		case L'＊': *dst++ = '*'; break;
		case L'＋': *dst++ = '+'; break;
		case L'，': *dst++ = ','; break;
		case L'－': *dst++ = '-'; break;
		case L'．': *dst++ = '.'; break;
		case L'／': *dst++ = '/'; break;
		case L'０': *dst++ = '0'; break;
		case L'１': *dst++ = '1'; break;
		case L'２': *dst++ = '2'; break;
		case L'３': *dst++ = '3'; break;
		case L'４': *dst++ = '4'; break;
		case L'５': *dst++ = '5'; break;
		case L'６': *dst++ = '6'; break;
		case L'７': *dst++ = '7'; break;
		case L'８': *dst++ = '8'; break;
		case L'９': *dst++ = '9'; break;
		case L'：': *dst++ = ':'; break;
		case L'；': *dst++ = ';'; break;
		case L'＜': *dst++ = '<'; break;
		case L'＝': *dst++ = '='; break;
		case L'＞': *dst++ = '>'; break;
		case L'？': *dst++ = '?'; break;
		case L'＠': *dst++ = '@'; break;
		case L'Ａ': *dst++ = 'A'; break;
		case L'Ｂ': *dst++ = 'B'; break;
		case L'Ｃ': *dst++ = 'C'; break;
		case L'Ｄ': *dst++ = 'D'; break;
		case L'Ｅ': *dst++ = 'E'; break;
		case L'Ｆ': *dst++ = 'F'; break;
		case L'Ｇ': *dst++ = 'G'; break;
		case L'Ｈ': *dst++ = 'H'; break;
		case L'Ｉ': *dst++ = 'I'; break;
		case L'Ｊ': *dst++ = 'J'; break;
		case L'Ｋ': *dst++ = 'K'; break;
		case L'Ｌ': *dst++ = 'L'; break;
		case L'Ｍ': *dst++ = 'M'; break;
		case L'Ｎ': *dst++ = 'N'; break;
		case L'Ｏ': *dst++ = 'O'; break;
		case L'Ｐ': *dst++ = 'P'; break;
		case L'Ｑ': *dst++ = 'Q'; break;
		case L'Ｒ': *dst++ = 'R'; break;
		case L'Ｓ': *dst++ = 'S'; break;
		case L'Ｔ': *dst++ = 'T'; break;
		case L'Ｕ': *dst++ = 'U'; break;
		case L'Ｖ': *dst++ = 'V'; break;
		case L'Ｗ': *dst++ = 'W'; break;
		case L'Ｘ': *dst++ = 'X'; break;
		case L'Ｙ': *dst++ = 'Y'; break;
		case L'Ｚ': *dst++ = 'Z'; break;
		case L'［': *dst++ = '['; break;
		case L'］': *dst++ = ']'; break;
		case L'＾': *dst++ = '^'; break;
		case L'＿': *dst++ = '_'; break;
		case L'｀': *dst++ = '`'; break;
		case L'ａ': *dst++ = 'a'; break;
		case L'ｂ': *dst++ = 'b'; break;
		case L'ｃ': *dst++ = 'c'; break;
		case L'ｄ': *dst++ = 'd'; break;
		case L'ｅ': *dst++ = 'e'; break;
		case L'ｆ': *dst++ = 'f'; break;
		case L'ｇ': *dst++ = 'g'; break;
		case L'ｈ': *dst++ = 'h'; break;
		case L'ｉ': *dst++ = 'i'; break;
		case L'ｊ': *dst++ = 'j'; break;
		case L'ｋ': *dst++ = 'k'; break;
		case L'ｌ': *dst++ = 'l'; break;
		case L'ｍ': *dst++ = 'm'; break;
		case L'ｎ': *dst++ = 'n'; break;
		case L'ｏ': *dst++ = 'o'; break;
		case L'ｐ': *dst++ = 'p'; break;
		case L'ｑ': *dst++ = 'q'; break;
		case L'ｒ': *dst++ = 'r'; break;
		case L'ｓ': *dst++ = 's'; break;
		case L'ｔ': *dst++ = 't'; break;
		case L'ｕ': *dst++ = 'u'; break;
		case L'ｖ': *dst++ = 'v'; break;
		case L'ｗ': *dst++ = 'w'; break;
		case L'ｘ': *dst++ = 'x'; break;
		case L'ｙ': *dst++ = 'y'; break;
		case L'ｚ': *dst++ = 'z'; break;
		case L'｛': *dst++ = '{'; break;
		case L'｜': *dst++ = '|'; break;
		case L'｝': *dst++ = '}'; break;
		case L'￣': *dst++ = '~'; break;
		case L'￥': *dst++ = '\\'; break;
		default:
			while (prev < p)
				*dst++ = *prev++;
			break;
		}
	}
	*dst = 0;
	return _dst;
}

BYTE *zen2han(const BYTE *src, CharacterEncoding enc) {
	switch (enc) {
	case SHIFT_JIS:
		return zen2han_sjis(src);
	case UTF8:
		return zen2han_utf8(src);
	default:
		return (BYTE *)src;
	}
}

BYTE *han2zen(const BYTE *src, CharacterEncoding enc) {
	if (enc != SHIFT_JIS)
		return (BYTE *)src; // Not implemented

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
			const WORD kana = kanatbl[c0 - 0xa0];
			*dst++ = kana >> 8; *dst++ = kana & 0xff;
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
