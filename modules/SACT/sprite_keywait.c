/*
 * sprite_keywait.c: スプライトキー待ち
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
/* $Id: sprite_keywait.c,v 1.2 2003/05/09 05:14:34 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <glib.h>

#include "portab.h"
#include "system.h"
#include "counter.h"
#include "ags.h"
#include "nact.h"
#include "imput.h"
#include "ngraph.h"
#include "surface.h"
#include "sact.h"
#include "sprite.h"
#include "sactsound.h"


static void hidesprite(sprite_t *sp);
static boolean waitcond(int endtime);



// じわじわ消す
static void hidesprite(sprite_t *sp) {
	int i;
	
	for (i = 255; i >= 0; i-=32) {
		sp->blendrate = i;
		sp_updateme(sp);
		sp_update_clipped();
		sys_keywait(10, FALSE);
	}
	
	sp_updateme(sp);
	sp->show = FALSE;
	sp_update_clipped();
}

/*
  キー待ち終了の条件チェック
   @param endtime: 終了時間
   @return: 終了なら TRUE、継続なら FALSE
   
   スプライトがドロップされた時の各種処理を含む
*/
static boolean waitcond(int endtime) {
	int curtime = get_high_counter(SYSTEMCOUNTER_MSEC);
	if (curtime >= endtime) return TRUE;
	
	if (sact.dropped) {
		sact.draggedsp->u.get.dragging = FALSE;
		if (sact.waitkey != -1) {
			// dropしたスプライトをじわじわ消す 
			hidesprite(sact.draggedsp);
			sact.sp_result_get = sact.draggedsp->no;
			sact.sp_result_put = sact.waitkey;
			sp_free(sact.draggedsp->no);
			sact.draggedsp = NULL;
			sact.dropped = FALSE;
			return TRUE;
		} else {
			// PUT/SWPUTスプライト以外のところにdropした場合
			sprite_t *sp = sact.draggedsp;
			if (sact.draggedsp->type == SPRITE_GETA) {
				// もとの場所にゆっくり戻す
				spev_move_waitend(sp, sp->loc.x, sp->loc.y, 150); 
			} else {
				// その場所に留まる
				sp->loc = sp->cur;
			}
			
			// drop音があれば、鳴らす
			if (sact.draggedsp->numsound3) {
				ssnd_play(sact.draggedsp->numsound3);
			}
			sact.draggedsp = NULL;
			sact.dropped = FALSE;
		}
	}
	
	// その他キー入力があれば終了
	return sact.waitkey == -1 ? FALSE : TRUE;
}

/*
  スプライトキー待ち
  @param vOK: 0ならば右クリック 
  @param vRND: スイッチスプライトの番号
  @param vD01: ゲットスプライトの番号
  @param vD02: プットスプライトの番号
  @param vD03: タイムアウトした場合=1, しない場合=0
  @param wTime: タイムアウト時間 (1/100sec)
*/
int sp_keywait(int *vOK, int *vRND, int *vD01, int *vD02, int *vD03, int timeout) {
	int curtime, endtime;
	
	// とりあえず全更新
	sp_update_all(TRUE);
	
	// depthmap を準備
	g_slist_foreach(sact.updatelist, sp_draw_dmap, NULL);
	
	sact.waittype = KEYWAIT_SPRITE;
	sact.waitkey = -1;
	sact.sp_result_sw  = 0;
	sact.sp_result_get = 0;
	sact.sp_result_put = 0;
	sact.draggedsp = NULL;
	
	{
		// とりあえず、現在のマウス位置を送って、switch sprite の
		// 状態を更新しておく
		agsevent_t agse;
		MyPoint p;
		sys_getMouseInfo(&p, FALSE);
		agse.type = AGSEVENT_MOUSE_MOTION;
		agse.d1 = p.x;
		agse.d2 = p.y;
		agse.d3 = 0;
		nact->ags.eventcb(&agse);
	}
	
	// 終了時間の計算
	curtime = get_high_counter(SYSTEMCOUNTER_MSEC);
	endtime = timeout < 0 ? G_MAXINT: (curtime + timeout * 10);
	
	// スプライトキー待ちメイン
	while (!waitcond(endtime)) {
		sys_keywait(25, TRUE);
	}
	
	if (sact.waitkey == 0) {
		// 右クリックキャンセル
		*vOK = 0;
		if (vD03) *vD03 = 0;
	} else if (sact.waitkey == -1) {
		// timeout
		*vOK = 1;
		if (vD03) *vD03 = 1;
	} else {
		*vOK = 1;
		if (vD03) *vD03 = 0;
	}
	
	*vRND = sact.sp_result_sw;
	*vD01 = sact.sp_result_get;
	*vD02 = sact.sp_result_put;
	
	sact.waittype = KEYWAIT_NONE;
	
	return OK;
}


