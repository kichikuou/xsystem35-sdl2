/*
 * LittleEndian.h  get little endian value
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
 * @version         0.00 97/11/06 初版
*/
/* $Id: LittleEndian.h,v 1.6 2000/11/25 13:08:56 chikama Exp $ */

#ifndef __LITTLEENDIAN__
#define __LITTLEENDIAN__

#include <string.h>
#include <SDL_endian.h>

static inline int LittleEndian_getDW(const uint8_t *b, int index) {
	uint32_t t;
	memcpy(&t, b + index, sizeof t);
	return SDL_SwapLE32(t);
}

static inline int LittleEndian_get3B(const uint8_t *b, int index) {
	uint32_t t = 0;
	memcpy(&t, b + index, 3);
	return SDL_SwapLE32(t);
}

static inline int LittleEndian_getW(const uint8_t *b, int index) {
	uint16_t t;
	memcpy(&t, b + index, sizeof t);
	return SDL_SwapLE16(t);
}

static inline void LittleEndian_putW(uint16_t num, uint8_t *b, int index) {
	num = SDL_SwapLE16(num);
	memcpy(b + index, &num, sizeof(num));
}

static inline void LittleEndian_putDW(uint32_t num, uint8_t *b, int index) {
	num = SDL_SwapLE32(num);
	memcpy(b + index, &num, sizeof(num));
}

#endif /* !__LITTLEENDIAN__ */
