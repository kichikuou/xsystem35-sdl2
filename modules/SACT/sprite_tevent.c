/*
 * sprite_tevent.c: アニメーションスプライトとスプライトの移動
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
/* $Id: sprite_tevent.c,v 1.2 2003/04/25 17:23:55 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <glib.h>

#include "portab.h"
#include "system.h"
#include "menu.h"
#include "imput.h"
#include "nact.h"
#include "sact.h"
#include "sprite.h"
#include "counter.h"

/*

 タイマイベントによるスプライトの移動とアニメーションスプライトの更新

 実際には、nact->callback()を全力で呼び出しでいることがほとんどで、
 タイマイベントが欲しいスプライト内部のコールバックで経過時間を計算し、
 状態を更新している。

 他には system35 のメインループ nact_main() から呼ばれることや、
 X|SDLのキー待ち中等に呼ばれる。
 
 スプライトの移動は SACT.Draw が呼ばれてから移動を開始し、移動終了まで
 待っているので、nact_mainから呼ばれるときはアニメーションスプライト
 の更新処理だけ。
 
*/

static void tevent_callback(agsevent_t *e);


/*
 タイマイベント callback メイン
*/
static void tevent_callback(agsevent_t *e) {
	GSList *node;
	int update = 0;
	
	// SP_MOVE の同期移動のためのカウンタの読み込み
	sact.movecurtime = get_high_counter(SYSTEMCOUNTER_MSEC);
	
	for (node = sact.teventlisteners; node; node = node->next) {
		sprite_t *sp = (sprite_t *)node->data;
		if (sp == NULL) continue;
		if (sp->teventcb == NULL) continue;
		
		// 非表示ではイベントに反応しない
		if (!sp->show) continue;

		// スプライト毎のタイマイベントハンドラの呼び出し
		update += sp->teventcb(sp, e);
	}
	
	// 変更があれば画面を更新
	if (update) {
		sp_update_clipped();
	}
	
	// timer event litener の削除 (上のループ内で削除できないので)
	for (node = sact.teventremovelist; node; node = node->next) {
		sprite_t *sp = (sprite_t *)node->data;
		if (sp == NULL) continue;
		sact.teventlisteners = g_slist_remove(sact.teventlisteners, sp);
	}
	g_slist_free(sact.teventremovelist);
	sact.teventremovelist = NULL;
}

/*
  タイマイベント callback の登録
  @param sp: 登録するスプライト
  @param cb: 呼び出されるcallback
*/
void spev_add_teventlistener(sprite_t *sp, int (*cb)(sprite_t *, agsevent_t *)) {
	sp->teventcb = cb;
	sact.teventlisteners = g_slist_append(sact.teventlisteners, sp);
}

/*
  上で登録した callback の削除
  @param sp: 削除するスプライト
*/
void spev_remove_teventlistener(sprite_t *sp) {
	sact.teventlisteners = g_slist_remove(sact.teventlisteners, sp);
}

/*
  system35のメインループからで呼ばれるコールバック
*/
void spev_main() {
	agsevent_t e;
	
	e.type = AGSEVENT_TIMER;
	tevent_callback(&e);

	// デフォルトのコールバックのうち、ここで必要なものだけ
	// 処理。(VAコマンドcallbackはなし)
	if (nact->popupmenu_opened) {
		menu_gtkmainiteration();
		if (nact->is_quit) sys_exit(0);
        }
}
