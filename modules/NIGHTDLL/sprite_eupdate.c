/*
 * sprite_eupdate.c: 効果つき更新
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
/* $Id: sprite_eupdate.c,v 1.1 2003/11/09 15:06:12 chikama Exp $ */

#include "config.h"

#include "portab.h"
#include "system.h"
#include "ags.h"
#include "input.h"
#include "sprite.h"
#include "gfx.h"
#include "effect.h"

/*
  効果つき画面更新
  @param no: 効果の種類
  @param time: 実行時間(msec)
  @param cancel: キー抜け(0:なし, 1:あり)
*/
void nt_sp_eupdate(int no, int time, int cancel) {
	if (no == 1013) {
		nt_sp_update_all(true);
		return;
	}

	nt_sp_update_all(false);
	
	enum effect_type type = from_sact_effect(no - 100);
	if (type == EFFECT_INVALID) {
		WARNING("Unimplemented effect %d", no);
		type = EFFECT_CROSSFADE;
	}
	SDL_Rect rect = { 0, 0, main_surface->w, main_surface->h };
	struct effect *eff = effect_init(&rect, NULL, 0, 0, gfx_getDIB(), 0, 0, type);

	int sttime, curtime;
	sttime = curtime = sys_get_ticks();
	int edtime = curtime + time;

	while ((curtime = sys_get_ticks()) < edtime) {
		effect_step(eff, (float)(curtime - sttime) / (edtime - sttime));
		int rest = 16 - (sys_get_ticks() - curtime);
		int key = sys_keywait(rest, cancel ? KEYWAIT_CANCELABLE : KEYWAIT_NONCANCELABLE);
		if (cancel && key)
			break;
	}
	effect_finish(eff);
	ags_updateFull();
}
