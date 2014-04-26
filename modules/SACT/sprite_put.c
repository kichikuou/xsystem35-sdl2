/*
 * sprite_put.c: プット・スイッチプットスプライト特有の処理
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
/* $Id: sprite_put.c,v 1.1 2003/04/22 16:29:52 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <glib.h>

#include "portab.h"
#include "ags.h"
#include "sact.h"
#include "sactsound.h"
#include "sprite.h"

static int eventCB_PUT(sprite_t *sp, agsevent_t *e);
static void cb_remove(sprite_t *sp);

// プットスプライトのイベント処理
static int eventCB_PUT(sprite_t *sp, agsevent_t *e) {
	int update = 0;
	
	switch(e->type) {
	case AGSEVENT_BUTTON_PRESS:
		if (e->d3 != AGSEVENT_BUTTON_LEFT) return 0;
		
		// ボタン押下時のスプライトがあれば、それを表示
		if (sp->cg3) {
			sp->curcg = sp->cg3;
			update++;
		}
		
		sp->pressed = TRUE;
		break;
		
	case AGSEVENT_BUTTON_RELEASE:
		if (sact.draggedsp == NULL && sp->type == SPRITE_PUT) return 0;
		
		if (sact.dropped) {
			sact.sp_result_sw = sp->no;
			sact.waitkey = sp->no;
			if (sp->numsound3) {
				ssnd_play(sp->numsound3);
			}
		} else {
			// スイッチプットスプライトの決定の処理
			if (sp->cg2) {
				sp->curcg = sp->cg2;
				update++;
			}
			
			// pressされたスプライトとreleaseされたスプライト
			// が同じ場合のみ、そのスプライトが押されたと判断
			if (sp->pressed) {
				sact.sp_result_sw = sp->no;
				sact.waitkey = sp->no;
				if (sp->numsound2) {
					ssnd_play(sp->numsound2);
				}
			}
			sp->pressed = FALSE;
		}
		break;
	}
	
	if (update) {
		sp_updateme(sp);
	}
	
	return update;
}

// スプライト削除時の処理
static void cb_remove(sprite_t *sp) {
	// event listener の削除
	spev_remove_eventlistener(sp);
}

/*
  sp_new の時にスプライトの種類毎の初期化
  @param sp: 初期化するスプライト
*/
int sp_put_setup(sprite_t *sp) {
	spev_add_eventlistener(sp, eventCB_PUT);
	sp->remove = cb_remove;
	
	return OK;
}
