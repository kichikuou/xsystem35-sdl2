/*
 * counter.c  内部カウンタ
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
/* $Id: counter.c,v 1.10 2000/11/25 13:09:03 chikama Exp $ */

#include "counter.h"
#include "sdl_core.h"
#include "nact.h"

#define NUM_COUNTER 257

static uint32_t base_ticks[NUM_COUNTER];
static int offsets[NUM_COUNTER];
static int divisions[NUM_COUNTER] = { [0 ...( NUM_COUNTER-1 )]=1};

static void counter_init(int num, int offset, int division) {
	base_ticks[num] = sdl_getTicks();
	offsets[num] = offset;
	divisions[num] = division;
}

static int get_counter_internal(int num, int division) {
	// Allow up to 10 get_counter() calls in single animation frame.
	static int frame = -1;
	static int count = 0;
	if (nact->frame_count == frame) {
		if (++count >= 10)
			nact->wait_vsync = TRUE;
	} else {
		frame = nact->frame_count;
		count = 0;
	}

	uint32_t ms = sdl_getTicks() - base_ticks[num] + offsets[num];
	return ms / division;
}

int get_counter(int division) {
	return get_counter_internal(0, division);
}

/* カウンタ〜を valでリセット */
/* 初期化時に一度呼んでおく */
void reset_counter(int val) {
	counter_init(0, val, 1);
}


int get_high_counter(int num) {
	return get_counter_internal(num, divisions[num]);
}

/* 高精度カウンタ〜 thanx tajiri@wizard */
/* カウンタ〜を valでリセット */
/* 初期化時に一度呼んでおく */
void reset_counter_high(int num, int division, int val) {
	if (num) {
		counter_init(num, val, division);
	} else {
		for (int i = 1; i <= 256; i++)
			counter_init(i, val, division);
	}
}


