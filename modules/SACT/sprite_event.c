/*
 * sprite_event.c: SACT内の mouse/key イベントのハンドラ
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
/* $Id: sprite_event.c,v 1.5 2003/11/09 15:06:13 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <glib.h>

#include "portab.h"
#include "system.h"
#include "counter.h"
#include "menu.h"
#include "imput.h"
#include "nact.h"
#include "key.h"
#include "sact.h"
#include "sprite.h"
#include "sactsound.h"
#include "sactlog.h"

/*
  SACT内のSpriteKey待ちやメッセージKey待ちなどを実現するため、X|SDL から
  Key/Mouse イベントがあったときに、nact->ags.eventcb()によって他のモジュール
  にイベントを送出できるようにした。

  SACTでは最初に spev_callback でイベントを受け取るようにし、現在のキー待ちの
  状態(sact.waittype)によってそれぞれのイベントハンドラを呼び出している。
  
  現在のところ、1) メッセージキー待ち、2) 単純キー待ち、3) メニューキー待ち、
  4) スプライトキー 5) バックログキー待ちの５つがある。

  1)メッセージキー待ち
    何かキーが押されるまで待つ。Zキースプライトの消去ができる。

  2)単純キー待ち
    1)と同じ

  3)メニューキー待ち
    メニューオープン時の処理。マウスが動いた時と、キー(orボタン)が離れたときに
    いくつかの処理を行う。

  4)スプライト待ち
    SW/GETA/GETB/PUT/SWPUTスプライトの各処理を行う。
    スプライトは番号の小さいものから順に上に重ねて表示し、下になっている
    スプライトにイベントが伝わらないようにする
    -> SpriteKey待ちの直前にスプライトの番号による depth map を作成し、
       マウスの位置にあるスプライトの番号をで取り出す。
       AlphaマスクつきのSpriteの場合、それを反映し、表示している場所のみに
       depthmapを反映。

      drag中のスプライトは常に最上位にくるように表示し、マウス移動等のイベント
      は最初に処理する。

  5)バックログ表示中のキー待ち
    オリジナルと同じキー操作のはず
*/


static void cb_focused_swsp(gpointer s, gpointer data);
static void cb_defocused_swsp(gpointer s, gpointer data);
static int  cb_focused(sprite_t *sp);
static int  cb_defocused(sprite_t *sp);
static void cb_waitkey_simple(agsevent_t *e);
static void cb_waitkey_sprite(agsevent_t *e);
static void cb_waitkey_selection(agsevent_t *e);

/*
 フォーカスを得たスプライトに説明スプライトが登録されていた場合の
 説明スプライトの表示ON
*/
static void cb_focused_swsp(gpointer s, gpointer data) {
	sprite_t *sp = (sprite_t *)s;
	int *update  = (int *)data;
	boolean oldstate = sp->show;
	
	WARNING("show up spex %d\n", sp->no);

	sp->show = TRUE;
	if (oldstate != sp->show) {
		(*update)++;
		sp_updateme(sp);
	}
}

/*
 フォーカスを得たスプライトに説明スプライトが登録されていた場合の
 説明スプライトの表示OFF
*/
static void cb_defocused_swsp(gpointer s, gpointer data) {
	sprite_t *sp = (sprite_t *)s;
	int *update  = (int *)data;
	boolean oldstate = sp->show;
	
	WARNING("hide spex %d\n", sp->no);
	
	sp->show = FALSE;
	if (oldstate != sp->show) {
		(*update)++;
		sp_updateme(sp);
	}
}

// zkey hide off
static void cb_focused_zkey(gpointer s, gpointer data) {
	sprite_t *sp = (sprite_t *)s;
	int *update  = (int *)data;
	boolean oldstate = sp->show;
	
	sp->show = sp->show_save;
	if (oldstate != sp->show) {
		(*update)++;
		sp_updateme(sp);
	}
}

// zkey hide on
static void cb_defocused_zkey(gpointer s, gpointer data) {
	sprite_t *sp = (sprite_t *)s;
	int *update  = (int *)data;
	boolean oldstate = sp->show;
	
	sp->show = FALSE;
	if (oldstate != sp->show) {
		(*update)++;
		sp_updateme(sp);
		sp->show_save = oldstate;
	}
}

/*
  フォーカスを得たスプライトの処理
    cg2があればcurcgをcg2に設定

    drag中のスプライトがある場合
      -> PUT/SWPUTスプライトのみ反応
    drag中のスプライトがない場合
      -> GETA/GETB/SWPUTスプライトのみ反応
*/
static int cb_focused(sprite_t *sp) {
	int update = 0;

	if (sact.draggedsp) {
		if (sp->type != SPRITE_PUT &&
		    sp->type != SPRITE_SWPUT) return 0;
	} else {
		if (sp->type == SPRITE_PUT) return 0;
	}
	
	if (!sp->focused) {
		if (sp->cg2) {
			if (sp->curcg != sp->cg2) {
				sp_updateme(sp);
			}
			sp->curcg = sp->cg2;
			update++;
		}
		sp->focused = TRUE;
		WARNING("get forcused %d, type %d\n", sp->no, sp->type);
		if (sp->numsound1) {
			ssnd_play(sp->numsound1);
		}
	}
	
	return update;
}

/*
  フォーカスを失ったスプライトの処理
    curcg を cg1 にセット
*/
static int cb_defocused(sprite_t *sp) {
	int update = 0;
	
	if (sp->focused) {
		if (sp->curcg != sp->cg1) {
			sp_updateme(sp);
		}
		sp->curcg = sp->cg1;
		update++;
		sp->focused = FALSE;
		WARNING("lost forcused %d\n", sp->no);
	}
	
	return update;
}

/*
  WaitKeySimpleのcallback
*/
static void cb_waitkey_simple(agsevent_t *e) {
	int cur, update = 0;
	
	switch (e->type) {
	case AGSEVENT_KEY_PRESS:
		if (e->d3 == KEY_Z) {
			cur = get_high_counter(SYSTEMCOUNTER_MSEC);
			if (!sact.zhiding) {
				g_slist_foreach(sact.sp_zhide, cb_defocused_zkey, &update);
				sact.zhiding = TRUE;
				sact.zdooff = TRUE;
				sact.zofftime = cur;
			} else {
				sact.zdooff = FALSE;
			}
		}
		break;
		
	case AGSEVENT_BUTTON_RELEASE:
		// back log view mode に移行
		if (e->d3 == AGSEVENT_WHEEL_UP ||
		    e->d3 == AGSEVENT_WHEEL_DN) {
			// MessageKey 待ちのときのみ
			if (sact.waittype != KEYWAIT_MESSAGE) break;
			sblog_start();
			sact.waittype = KEYWAIT_BACKLOG;
			break;
		}
		if (sact.zhiding) {
			g_slist_foreach(sact.sp_zhide, cb_focused_zkey, &update);
			sact.zhiding = FALSE;
		}
		// fall through
		
	case AGSEVENT_KEY_RELEASE:
		switch(e->d3) {
		case KEY_Z:
			cur = get_high_counter(SYSTEMCOUNTER_MSEC);
			if (500 < (cur - sact.zofftime) || !sact.zdooff) {
				g_slist_foreach(sact.sp_zhide, cb_focused_zkey, &update);
				sact.zhiding = FALSE;
			}
			break;
		case KEY_PAGEUP:
		case KEY_PAGEDOWN:
			// MessageKey 待ちのときのみ
			if (sact.waittype != KEYWAIT_MESSAGE) break;
			sblog_start();
			sact.waittype = KEYWAIT_BACKLOG;
			break;
		default:
			sact.waitkey = e->d3;
			break;
		}
	}
	
	if (update) {
		sp_update_clipped();
	}
}

/*
  WaitKeySpriteのcallback
*/
static void cb_waitkey_sprite(agsevent_t *e) {
	GSList *node;
	sprite_t *focused_sp = NULL;   // focus を得ている sprite
	sprite_t *defocused_sp = NULL; // focus を失った sprite
	int update = 0;
	
	// キーイベントは無視
	switch(e->type) {
	case AGSEVENT_KEY_RELEASE:
	case AGSEVENT_KEY_PRESS:
		return;
	}
	
	if (sact.draggedsp) {
		// 先に drag中のspriteにイベントを送る
		update = sact.draggedsp->eventcb(sact.draggedsp, e);
	} else {
		// 右クリックキャンセル
		// drag中でない時のみ、キャンセルを受け付ける
		if (e->type == AGSEVENT_BUTTON_RELEASE &&
		    e->d3   == AGSEVENT_BUTTON_RIGHT) {
			sact.waitkey = 0;
			return;
		}
	}
	
	// forcusを得ている sprite と focusを失った sprite を探す
	for (node = sact.eventlisteners; node; node = node->next) {
		sprite_t *sp = (sprite_t *)node->data;
		
		if (sp == NULL) continue;
		if (!sp->show) continue;
		
		// freeze状態ではCGは変化しない
		if (sp->freezed_state != 0) continue;
		
		// dragg中の sprite は無視する
		if (sp == sact.draggedsp) continue;
		
		if (focused_sp == NULL && sp_is_insprite(sp, e->d1, e->d2)) {
			/*
			  focusを得ている sprite
			*/
			update += cb_focused(sp);
			focused_sp = sp;
		} else {
			/* 
			   現在のカーソル位置にはいま処理している番号のsprite
			   は存在しないので、defocus の処理を行う。
			   今回のマウス移動イベントによりfocusを失ったので
			   あれば、cb_decocused(sp)は 1 を返す
			*/ 
			int ret = cb_defocused(sp);
			if (ret > 0) defocused_sp = sp;
			update += ret;
		}
	}
	
	// focusを得た sprite に BUTTONイベントのみを送る
	if (focused_sp && e->type != AGSEVENT_MOUSE_MOTION) {
		update += focused_sp->eventcb(focused_sp, e);
	}
	
	// 範囲外(focusを得ているspriteがない場合)をクリックしたときの音
	if (!focused_sp &&
	    e->type != AGSEVENT_MOUSE_MOTION &&
	    sact.numsoundob) {
		ssnd_play(sact.numsoundob);
	}
	
	// drag中でない場合は、説明スプライトの表示、消去を行う
	if (sact.draggedsp == NULL && e->type == AGSEVENT_MOUSE_MOTION) {
		// focus を失った sprite の 説明 sprite の消去
		if (defocused_sp) {
			sprite_t *sp = defocused_sp;
			if (sp->expsp) {
				g_slist_foreach(sp->expsp, cb_defocused_swsp, &update);
			}
		}
		
		// focus を得た sprite の 説明 sprite の表示
		if (focused_sp) {
			sprite_t *sp = focused_sp;
			if (sp->expsp) {
				g_slist_foreach(sp->expsp, cb_focused_swsp, &update);
			}
		}
	}
	
	// 表示状態に変更があればその領域を更新
	if (update) {
		sp_update_clipped();
	}
}

/*
  選択肢 Window Open 時の callback
*/
static void cb_waitkey_selection(agsevent_t *e) {
	switch (e->type) {
	case AGSEVENT_BUTTON_RELEASE:
		sact.sel.cbrelease(e);
		break;
		
	case AGSEVENT_MOUSE_MOTION:
		sact.sel.cbmove(e);
		break;
	}
}

/*
  バックログ参照時
*/
static void cb_waitkey_backlog(agsevent_t *e) {
	switch (e->type) {
	case AGSEVENT_KEY_RELEASE:
		switch (e->d3) {
		case KEY_ESC:
			sblog_end();
			sact.waittype = KEYWAIT_MESSAGE;
			break;
		case KEY_PAGEUP:
			sblog_pageup();
			break;
		case KEY_PAGEDOWN:
			sblog_pagedown();
			break;
		case KEY_UP:
			sblog_pagenext();
			break;
		case KEY_DOWN:
			sblog_pagepre();
			break;
		}
		break;
		
	case AGSEVENT_BUTTON_RELEASE:
		switch(e->d3) {
		case AGSEVENT_WHEEL_UP:
			sblog_pagenext();
			break;
		case AGSEVENT_WHEEL_DN:
			sblog_pagepre();
			break;
		case AGSEVENT_BUTTON_RIGHT:
			sblog_end();
			sact.waittype = KEYWAIT_MESSAGE;
			break;
		}
		break;
	}
}

/*
  X|SDL のイベントディスパッチャからくる最初の場所
*/
void spev_callback(agsevent_t *e) {
	// menu open中は無視
	if (nact->popupmenu_opened) {
		return;
	}
	
	if (sact.waittype != KEYWAIT_BACKLOG) {
		if (e->type == AGSEVENT_KEY_PRESS && e->d3 == KEY_CTRL) {
			sact.waitskiplv = 2;
			sact.waitkey = e->d3;
			return;
		}
		
		if (e->type == AGSEVENT_KEY_RELEASE && e->d3 == KEY_CTRL) {
			sact.waitskiplv = 0;
			sact.waitkey = e->d3;
			return;
		}
	}
	
	switch (sact.waittype) {
	case KEYWAIT_MESSAGE:
	case KEYWAIT_SIMPLE:
		cb_waitkey_simple(e);
		break;

	case KEYWAIT_SPRITE:
		cb_waitkey_sprite(e);
		break;
	
	case KEYWAIT_SELECT:
		cb_waitkey_selection(e);
		break;

	case KEYWAIT_BACKLOG:
		cb_waitkey_backlog(e);
		break;
		
	default:
		return;
	}
}

/*
  各スプライト毎のイベント callback の登録
*/
void spev_add_eventlistener(sprite_t *sp, int (*cb)(sprite_t *, agsevent_t *)) {
	sp->eventcb = cb;
	sact.eventlisteners = g_slist_append(sact.eventlisteners, sp);
}

/*
  上で登録した callback の削除
*/
void spev_remove_eventlistener(sprite_t *sp) {
	sact.eventlisteners = g_slist_remove(sact.eventlisteners, sp);
}

