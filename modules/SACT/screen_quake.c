/*
 * screen_quake.c: スクリーン全体を揺らす
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
/* $Id: screen_quake.c,v 1.1 2003/04/22 16:29:52 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "portab.h"
#include "system.h"
// #include "LittleEndian.h"
#include "ags.h"
#include "input.h"
#include "sact.h"
#include "sprite.h"
#include "sdl_core.h"
#include "randMT.h"

#define M_PIf ((float)M_PI)

typedef void entrypoint (float step, int p1, int p2, int *retx, int *rety);

// 上下左右方向の揺らし
static void quake0(float step, int ampx, int ampy, int *adjx, int *adjy) {
	static int i = 0;
	
	*adjx = (int)(genrand() * ampx/2);
	*adjy = (int)(genrand() * ampy/2);
	*adjx *= ((-1)*(i%2) + ((i+1)%2));
	*adjy *= ((-1)*((i+1)%2) + (i%2));
	i++;
}

// 回転の揺らし
static void quake1(float curstep, int diam, int round, int *adjx, int *adjy) {
	float R = (1 - curstep) * diam / 2;
	float th = curstep * 2 * M_PIf * round;

	*adjx = (int)(R * cosf(th));
	*adjy = (int)(R * sinf(th));
}

/*
   画面揺らし
   @param wType: 0=縦横, 1:回転
   @param wParam1: wType=0のときx方向の振幅
                   wType=1のとき振幅
   @param wParam2: wType=0のときy方向の振幅
                   wType=1のとき回転数
   @param wCount: 時間(1/100秒)
   @param nfKeyEnable: キー抜け (1で有効)
*/
void sp_quake_screen(int type, int p1, int p2, int time, int cancel) {
	int sttime, edtime, curtime;
	int key;
	entrypoint *cb[2] = {quake0, quake1};
	
	if (type > 1) return;
	
	sttime = sdl_getTicks();
	edtime = time * 10 + sttime;
	while ((curtime = sdl_getTicks()) < edtime) {
		int adjx, adjy;
		
		cb[type]((float)(curtime - sttime)/(edtime - sttime), p1, p2, &adjx, &adjy);
		ags_setViewArea(adjx, adjy, main_surface->w, main_surface->h);
		ags_updateFull();
		
		key = sys_keywait(10, cancel ? KEYWAIT_CANCELABLE : KEYWAIT_NONCANCELABLE);
		if (cancel && key) break;
	}
	
	ags_setViewArea(0, 0, main_surface->w, main_surface->h);
	ags_updateFull();
}

