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
#include <sys/time.h>
#include <unistd.h>

#include "portab.h"
#include "sact.h"
#include "sacttimer.h"

/*
  sact timer subsystem 初期化
*/
int stimer_init() {
	stimer_reset(0, 0);
	return OK;
}

// 指定IDのタイマーのリセット
int stimer_reset(int id, int val) {
	gettimeofday(&(sact.timer[id].tv_base), NULL);
	sact.timer[id].val = val;
	return OK;
}

// 指定IDのタイマーの取得
int stimer_get(int id) {
	long sec, usec, usec2;
	struct timeval tv;
	struct timeval tv_base = sact.timer[id].tv_base;
	int division = 10;
	
	gettimeofday(&tv, NULL);
	sec  = tv.tv_sec - tv_base.tv_sec;
	usec = tv.tv_usec - tv_base.tv_usec;
	usec2 = sec * (1000l/division)+ usec / 1000l /division;
	return sact.timer[id].val + usec2;
}
