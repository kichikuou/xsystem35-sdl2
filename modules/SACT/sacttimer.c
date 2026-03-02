/*
 * sacttimer.c: SACTのタイマ関連
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
/* $Id: sacttimer.c,v 1.1 2003/04/22 16:29:52 chikama Exp $ */

#include <stdlib.h>
#include <string.h>

#include "portab.h"
#include "sacttimer.h"
#include "system.h"

#define MAX_TIMER 10
#define TICKS_PER_CENTISECOND 10

uint32_t ticks_base[MAX_TIMER];

void stimer_init(void) {
	memset(ticks_base, 0, sizeof(ticks_base));
}

void stimer_reset(int id, int val) {
	if (id < 0 || id >= MAX_TIMER) {
		WARNING("Invalid timer ID: %d", id);
		return;
	}
	ticks_base[id] = sys_get_ticks() - (val * TICKS_PER_CENTISECOND);
}

int stimer_get(int id) {
	if (id < 0 || id >= MAX_TIMER) {
		WARNING("Invalid timer ID: %d", id);
		return 0;
	}
	uint32_t ticks = sys_get_ticks();
	uint32_t diff = ticks - ticks_base[id];
	return diff / TICKS_PER_CENTISECOND;
}
