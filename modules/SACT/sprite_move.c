/*
 * sprite_move.c: スプライトの移動に関する各種処理
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
/* $Id: sprite_move.c,v 1.1 2003/04/22 16:29:52 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <math.h>
#include <glib.h>

#include "portab.h"
#include "system.h"
#include "imput.h"
#include "nact.h"
#include "sact.h"
#include "sprite.h"
#include "counter.h"

/*

  SP_MOVE によるスプライトの移動は、SACT.Drawが発行されると同時に
  移動を開始し、全ての移動が終了するまで待つ。

  移動が開始されるまでに削除、または新規に作成されたスプライトは
  移動しないので注意
  
*/


static void move_drain(sprite_t *sp);
static int move_cb(sprite_t *sp, agsevent_t *e);

// SP_MOVEコマンドの後始末
static void move_drain(sprite_t *sp) {
	// 古い場所のupdate
	sp_updateme(sp);

	// 最終移動場所にスプライト位置をセット
	sp->cur = sp->loc = sp->move.to;

	// あたらしい場所のupdate
	sp_updateme(sp);
	
	// 後で、movelist から外してもらうための処理
	sact.teventremovelist = g_slist_append(sact.teventremovelist, sp);
	
	sp->move.moving = FALSE;
	sp->move.time = 0; // 移動時間の初期化
}

// SP_MOVE の timer event callback
static int move_cb(sprite_t *sp, agsevent_t *e) {
	int t, update = 0;
	int now, newx, newy;

	// 現在時刻の取得
	now = sact.movecurtime;
	
	WARNING("no = %d now = %d st = %d, ed = %d\n",
		sp->no, now, sp->move.starttime, sp->move.endtime);
	
	if (now >= sp->move.endtime) {
		// 時間オーバーなら、最終位置に移動してMOVE終了
		move_drain(sp);
		return 1;
	}
	
	// 経過時間
	t = now - sp->move.starttime;
	
	newx = sp->loc.x + t * (sp->move.to.x - sp->loc.x) / sp->move.time;
	newy = sp->loc.y + t * (sp->move.to.y - sp->loc.y) / sp->move.time;
	
	// 移動していたら新しい位置を記録して書き換えを指示
	if (newx != sp->cur.x || newy != sp->cur.y) {
		// 古い場所のupdate
		sp_updateme(sp);
		sp->cur.x = newx;
		sp->cur.y = newy;
		// 新しい場所のupdate
		sp_updateme(sp);
		update++;
	} else {
		usleep(1);
	}
	
	return update;
}

/*
 SP_MOVEコマンド、移動の準備
 @param data: sprite
 @param userdata: 未使用
*/
void spev_move_setup(gpointer data, gpointer userdata) {
	sprite_t *sp = (sprite_t *)data;
	
	// 非表示のものは移動しない(いいのかな)
	if (!sp->show) return;
	
	// move 開始時刻の記録
	sp->move.starttime = sact.movestarttime;
	sp->move.moving = TRUE;
	
	// MOVE_SPEED で設定した場合は、移動量を考慮して移動時間を決定
	if (sp->move.time == -1) {
		// speed から timeへ
		int dx = sp->move.to.x - sp->loc.x;
		int dy = sp->move.to.y - sp->loc.y;
		int d = (int)sqrt(dx*dx+dy*dy);
		sp->move.time = d * 100 / sp->move.speed;
	}
	
	// move 終了予定時刻
	sp->move.endtime = sp->move.starttime + sp->move.time;
	
	// タイマコールバック登録
	spev_add_teventlistener(sp, move_cb);
	
	WARNING("no=%d,from(%d,%d@%d)to(%d,%d@%d),time=%d\n", sp->no,
		sp->cur.x, sp->cur.y, sp->move.starttime,
		sp->move.to.x, sp->move.to.y, sp->move.endtime,
		sp->move.time);

}

/*
  スプライトの移動先を指定し、移動先にくるまで待つ
  @param sp: 対象スプライト
  @param dx: 移動先Ｘ座標
  @param dy: 移動先Ｙ座標
  @param time: 移動速度
*/
void spev_move_waitend(sprite_t *sp, int dx, int dy, int time) {
	sp->loc = sp->cur;
	sp->move.to.x = dx;
	sp->move.to.y = dy;
	sp->move.speed = time;
	sp->move.time = -1;
	
	sact.movelist = g_slist_append(sact.movelist, sp);
	sact.movestarttime = get_high_counter(SYSTEMCOUNTER_MSEC);
	g_slist_foreach(sact.movelist, spev_move_setup, NULL);
	g_slist_free(sact.movelist);
	sact.movelist = NULL;
	
	while (sp->move.moving) {
		nact->callback();
	}
}

/*
  全ての移動中のスプライトが移動完了するのを待つ
*/
void spev_wait4moving_sp() {
	GSList *node;
	
	// 移動中のスプライトは sact.updatelist にあるはずだから
	// そのなかのスプライトについて、移動中かどうかのフラグをチェック
	for (node = sact.updatelist; node; node = node->next) {
		sprite_t *sp = (sprite_t *)node->data;
		if (sp == NULL) continue;
		if (!sp->show)  continue;
		
		while (sp->move.moving) {
			nact->callback();
		}
	}
}
