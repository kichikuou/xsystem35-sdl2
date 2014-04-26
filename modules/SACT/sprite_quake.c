/*
 * sprite_quake.c: スプライトを揺らす
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
/* $Id: sprite_quake.c,v 1.1 2003/04/22 16:29:52 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <glib.h>

#include "portab.h"
#include "system.h"
#include "imput.h"
#include "sact.h"
#include "surface.h"
#include "ngraph.h"
#include "sprite.h"
#include "counter.h"
#include "randMT.h"

/*
   QuakeSpriteAddで設定したスプライトを揺らす
   
   @param wType: 0:縦横(全てのスプライトを同じように揺らす)
                 1:縦横(全てのスプライトをバラバラに揺らす)
   @param wAmplitudeX: Ｘ方向の振幅
   @param wAmplitudeY: Ｙ方向の振幅
   @param wCount: 時間(1/100秒)
   @param cancel: キーキャンセルあり(=1)
*/
int sp_quake_sprite(int wType, int wAmplitudeX, int wAmplitudeY, int wCount, int cancel) {
	int edtime, curtime;
	int i = 0, key;
	GSList *node;
	
	edtime = wCount * 10 + get_high_counter(SYSTEMCOUNTER_MSEC);
	
	while ((curtime = get_high_counter(SYSTEMCOUNTER_MSEC)) < edtime) {
		if (wType == 0) { // 全てのスプライトを同じように動かす
			int adjx = (int)(genrand() * wAmplitudeX/2);
			int adjy = (int)(genrand() * wAmplitudeY/2);
			adjx *= ((-1)*(i%2) + ((i+1)%2));
			adjy *= ((-1)*((i+1)%2) + (i%2));
			for (node = sact.sp_quake; node; node = node->next) {
				sprite_t *sp = (sprite_t *)node->data;
				if (sp == NULL) continue;
				sp_updateme(sp);
				sp->cur.x = sp->loc.x + adjx;
				sp->cur.y = sp->loc.y + adjy;
				sp_updateme(sp);
			}
		} else { //  全てのスプライトを別々に動かす
			for (node = sact.sp_quake; node; node = node->next) {
				sprite_t *sp = (sprite_t *)node->data;
				int adjx = (int)(genrand() * wAmplitudeX/2);
				int adjy = (int)(genrand() * wAmplitudeY/2);
				if (sp == NULL) continue;
				adjx *= ((-1)*(i%2) + ((i+1)%2));
				adjy *= ((-1)*((i+1)%2) + (i%2));
				sp_updateme(sp);
				sp->cur.x = sp->loc.x + adjx;
				sp->cur.y = sp->loc.y + adjy;
				sp_updateme(sp);
			}
		}
		sp_update_clipped();
		i++;
		
		// ウェイトとキャンセルチェック
		key = sys_keywait(10, cancel);
		if (cancel && key != 0) break;
	}
	
	// 元のあった場所に戻す
	for (node = sact.sp_quake; node; node = node->next) {
		sprite_t *sp = (sprite_t *)node->data;
		if (sp == NULL) continue;
		sp->cur = sp->loc;
		sp_updateme(sp);
	}
	sp_update_clipped();
	return OK;
}
