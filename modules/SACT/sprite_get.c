/*
 * sprite_get.c: ゲットA/Bスプライト特有の処理
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
/* $Id: sprite_get.c,v 1.1 2003/04/22 16:29:52 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <glib.h>

#include "portab.h"
#include "ags.h"
#include "sact.h"
#include "sprite.h"
#include "sactsound.h"

static void cb_defocused_swsp(gpointer s, gpointer data);
static int eventCB_GET(sprite_t *sp, agsevent_t *e);
static void cb_remove(sprite_t *sp);

/*
  説明スプライトの消去
    dragが始まったら説明スプライトは消去する
*/
static void cb_defocused_swsp(gpointer s, gpointer data) {
	sprite_t *sp = (sprite_t *)s;
	int *update = (int *)data;
	boolean oldstate = sp->show;
	
	sp->show = FALSE;
	if (oldstate != sp->show) {
		(*update)++;
		sp_updateme(sp);
	}
}


//ゲットスプライトのイベント処理
static int eventCB_GET(sprite_t *sp, agsevent_t *e) {
	int update = 0;
	
	switch(e->type) {
	case AGSEVENT_BUTTON_PRESS:
		if (e->d3 != AGSEVENT_BUTTON_LEFT) break;
		
		// drag開始時のマウスの位置記録
		sp->u.get.dragging = TRUE;
		sp->u.get.dragstart.x = e->d1;
		sp->u.get.dragstart.y = e->d2;
		
		if (sp->cg3) {
			sp->curcg = sp->cg3;
			update++;
			sp_updateme(sp);
		}
		
		// スプライトの表示を一番前に持って来る
		sact.draggedsp = sp;
		sact.dropped = FALSE;
		
		// 説明スプライトがある場合は、それを非表示にする
		if (sp->expsp) {
			g_slist_foreach(sp->expsp, cb_defocused_swsp, &update);
		}
		
		// SpriteSoundがあれば、それを鳴らす
		if (sp->numsound2) {
			ssnd_play(sp->numsound2);
		}
		
		break;
		
	case AGSEVENT_BUTTON_RELEASE:
		// どのボタンでもドラッグ中止|終了
		
		if (!sp->u.get.dragging) break;
		
		sact.dropped = TRUE;
		break;

	case AGSEVENT_MOUSE_MOTION:
	{
		int newx, newy;
		
		// MOUSE MOTION は dragg中にしか呼ばれないから
		// これは不要
		// if (!sp->u.get.dragging) break;
		
		// マウスの現在位置により新しい場所を計算
		newx = sp->loc.x + (e->d1 - sp->u.get.dragstart.x);
		newy = sp->loc.y + (e->d2 - sp->u.get.dragstart.y);
		if (newx != sp->cur.x || newy != sp->cur.y) {
			sp_updateme(sp);
			sp->cur.x = newx;
			sp->cur.y = newy;
			update++;
			sp_updateme(sp);
		}
		break;
	}}
	
	return update;
}

// スプライト削除時の処理
static void cb_remove(sprite_t *sp) {
	spev_remove_eventlistener(sp);
}

/*
  sp_new の時にスプライトの種類毎の初期化
  @param sp: 初期化するスプライト
*/
int sp_get_setup(sprite_t *sp) {
	spev_add_eventlistener(sp, eventCB_GET);
	sp->remove = cb_remove;
	
	return OK;
}

