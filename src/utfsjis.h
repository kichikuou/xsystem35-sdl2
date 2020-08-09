/*
 * utfsjis.h -- utf-8/sjis related function
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
/* $Id: eucsjis.h,v 1.2 2000/09/20 10:33:16 chikama Exp $ */

#ifndef __UTFSJIS__
#define __UTFSJIS__

#include "portab.h"

typedef enum {
	SHIFT_JIS,
	UTF8,
	CHARACTER_ENCODING_MAX = UTF8
} CharacterEncoding;

#define MAX_SJIS_BYTES_PAR_CHAR 2
#define MAX_UTF8_BYTES_PAR_CHAR 4
#define MAX_BYTES_PAR_CHAR(encoding) \
	((encoding) == SHIFT_JIS ? MAX_SJIS_BYTES_PAR_CHAR : MAX_UTF8_BYTES_PAR_CHAR)

#define UTF8_TRAIL_BYTE(b) ((signed char)(b) < -0x40)
#define CHECKSJIS1BYTE(b) ( ((b) & 0xe0) == 0x80 || ((b) & 0xe0) == 0xe0 )

extern char* codeconv(CharacterEncoding tocode,
					  CharacterEncoding fromcode,
					  const char *str);

extern BYTE*   sjis2utf(const BYTE *src);
extern BYTE*   utf2sjis(const BYTE *src);
extern boolean sjis_has_hankaku(const BYTE *src);
extern boolean sjis_has_zenkaku(const BYTE *src);
extern int     utf8_next_codepoint(const char **msg);

extern char *advance_char(const char *s, CharacterEncoding e);

#endif /* __UTFSJIS__ */
