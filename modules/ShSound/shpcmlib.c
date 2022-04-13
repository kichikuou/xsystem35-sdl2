/*
 * shpcmlib.c ShSound用 pcmlib
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
/* $Id: shpcmlib.c,v 1.2 2003/08/02 13:10:32 chikama Exp $ */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <SDL_mixer.h>

#include "portab.h"

/*
  左右のチャンネルの入れ換え
*/
void pcmlib_reverse_pan_memory(Mix_Chunk *chunk) {
	if (chunk == NULL) return;

	short *buf = (short*)chunk->abuf;
	int len = chunk->alen / 4;
	for (int i = 0; i < len; i++) {
		short tmp = buf[0];
		buf[0] = buf[1];
		buf[1] = tmp;
		buf += 2;
	}
}

/*
  メモリ上の PCM データにフェード効果をかける
*/
void pcmlib_fade_volume_memory(Mix_Chunk *chunk, int start, int range) {
	if (chunk == NULL) return;

	start *= 441;  // 10ms -> sample
	range *= 441;  // 10ms -> sample

	if (chunk->alen / 4 < start + range)
		return;

	short *buf = (short*)chunk->abuf;
	buf += start * 2;

	// 指定の場所から徐々に音量を下げる
	for (int i = range; i > 0; i--) {
		buf[0] = buf[0] * i / range;
		buf[1] = buf[1] * i / range;
		buf += 2;
	}

	// 残りは無音
	memset(buf, 0, chunk->abuf + chunk->alen - (Uint8*)buf);
}
