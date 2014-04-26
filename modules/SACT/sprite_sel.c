/*
 * sprite_sel.c: 選択肢処理
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
/* $Id: sprite_sel.c,v 1.1 2003/04/22 16:29:52 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <glib.h>

#include "portab.h"
#include "nact.h"
#include "ags.h"
#include "imput.h"
#include "key.h"
#include "sact.h"
#include "sprite.h"
#include "ngraph.h"
#include "drawtext.h"

// 選択された要素(1~) キャンセルの場合は0、初期状態は -1
static int selected_item;
static int selected_item_cur;

// 選択肢を描画するsurface
static surface_t *selcanvas;

// 前のカーソルの状態
static boolean oldstate; // spriteの中か外か
static int oldindex; // 何番目の要素か(0~)


static boolean sp_is_insprite2(sprite_t *sp, int x, int y, int margin);
static void cb_select_move(agsevent_t *e);
static void cb_select_release(agsevent_t *e);
static int update_selwindow(sprite_t *sp);
static void setup_selwindow();
static void remove_selwindow();
static int sel_main();




/**
 *  内側にマージンを含む sprite内領域チェック
 *  マージン内はsprite内部とは判断しない
 */
static boolean sp_is_insprite2(sprite_t *sp, int x, int y, int margin) {
	MyRectangle r;
	cginfo_t *curcg = sp->curcg;
	
	r.x = sp->cur.x + margin;
	r.y = sp->cur.y + margin;
	r.width = curcg->sf->width   - 2 * margin;
	r.height = curcg->sf->height - 2 * margin;
	return ags_regionContains(&r, x, y);
}

// マウスが移動したときの callback
static void cb_select_move(agsevent_t *e) {
	int x = e->d1, y = e->d2;
	sprite_t *sp = sact.sp[sact.sel.spno];
	boolean newstate;
	int newindex;
	
	// sprite内か？
	newstate = sp_is_insprite2(sp, x, y, sact.sel.frame_dot);
	newindex = (y - (sp->cur.y + sact.sel.frame_dot)) / (sact.sel.font_size + sact.sel.linespace);
	
	if (newstate == oldstate) {
		if ((newstate == FALSE) || (newindex == oldindex)) {
			// 前と状態が同じでかつ、新しい状態がspriteの外か、
			// indexが変わらない場合はなにもしない。
			return;
		}
	}
	
	if (newstate) {
		// spriteの内部
		// fprintf(stderr, "in region %d\n", newindex);
		//update_selwindow(newindex + 1);
		selected_item_cur = newindex + 1;
	} else {
		// spriteの外部
		//update_selwindow(0);
		selected_item_cur = 0;
	}
	
	oldstate = newstate;
	oldindex = newindex;
	
	// 再描画
	sp_updateme(sp);
	sp_update_clipped();
}

// ボタンがリリースされたときの callback
static void cb_select_release(agsevent_t *e) {
	int x = e->d1, y = e->d2;
	sprite_t *sp = sact.sp[sact.sel.spno];
	boolean st;
	int iy;
	
	switch (e->d3) {
	case AGSEVENT_BUTTON_LEFT:
		st = sp_is_insprite2(sp, x, y, sact.sel.frame_dot);
		iy = (y - (sp->cur.y + sact.sel.frame_dot)) / (sact.sel.font_size + sact.sel.linespace);

		// カーソルが sprite の外の場合は無視
		if (st == FALSE) {
			return;
		}
		
		// 選択要素が空の場合も無視
		if (sact.sel.elem[iy + 1] == NULL) return;
		
		selected_item = iy + 1;
		break;
		
	case AGSEVENT_BUTTON_RIGHT:
		// キャンセル
		selected_item = 0;
		break;
	}
}

// 選択ウィンドを更新するときの callback
static int update_selwindow(sprite_t *sp) {
	int selno = selected_item_cur;
	int x0, y0;

	x0 = sp->cur.x;
	y0 = sp->cur.y;
	// 背景 CG
	sp_draw(sp);
	
	// 選択されている要素
	if (selno && sact.sel.elem[selno] != NULL) {
		int w = selcanvas->width - 2 * sact.sel.frame_dot;
		int h = sact.sel.font_size + sact.sel.linespace;
		int x = x0 + sact.sel.frame_dot;
		int y = y0 + sact.sel.frame_dot + (selno -1) * h;
		gr_fill(sf0, x, y, w, h, 0, 0, 0);
		gr_drawrect(sf0, x, y, w, h, 255, 255, 255);
	}
	
	// 選択肢文字列
	gr_expandcolor_blend(sf0, x0, y0, 
			     sact.sel.charcanvas, 0, 0,
			     selcanvas->width, selcanvas->height, 255, 255, 255);
	
	return OK;
}

// 選択ウィンドの準備
static void setup_selwindow() {
	sprite_t *sp = sact.sp[sact.sel.spno];
	int i;
	
	//選択ウィンド作業 surfaceの生成
	selcanvas = sf_dup(sp->cg1->sf);
	
	// 選択肢文字用 canvas
	sact.sel.charcanvas = sf_create_pixel(selcanvas->width, selcanvas->height, 8);
	
	dt_setfont(sact.sel.font_type, sact.sel.font_size);
	
	// 指定のスプライトに文字(選択肢を書く)
	for (i = 1; i < SEL_ELEMENT_MAX; i++) {
		int x, y;
		if (sact.sel.elem[i] == NULL) continue;
		// 文字の場所計算
		x = 0; // 行そろえは無し
		y = (i - 1) * (sact.sel.font_size + sact.sel.linespace);
		dt_drawtext(sact.sel.charcanvas,
			    x + sact.sel.frame_dot, y + sact.sel.frame_dot,
			    sact.sel.elem[i]);
	}
	
	// デフォルトで選択される選択肢がある場合、そこへカーソルを移動
	if (sact.sel.movecursor) {
		ags_setCursorLocation(sp->cur.x + sact.sel.frame_dot + 2,
				      sp->cur.y + sact.sel.frame_dot + 2 + (sact.sel.font_size + sact.sel.linespace)*(sact.sel.movecursor -1), TRUE);
		selected_item = (sact.sel.movecursor -1);
		oldstate = TRUE;
		oldindex = selected_item -1;
	}

	// その他初期化
	selected_item_cur = 0;

	// スプライト再描画 callback の登録
	sp->update = update_selwindow;
}

// 選択ウィンドの削除
static void remove_selwindow() {
	sprite_t *sp = sact.sp[sact.sel.spno];

	// スプライト再描画 callback を元にもどす
	sp->update = sp_draw;

	// スプライトを再描画して(おそらく消す)
	sp_updateme(sp);
	sp_update_clipped();
	
	// 作業用 surface の削除
	sf_free(selcanvas);
	sf_free(sact.sel.charcanvas);
}


// 選択メインループ
static int sel_main() {
	sact.waittype = KEYWAIT_SELECT;
	sact.waitkey = -1;

	selected_item = -1;
	
	while(selected_item == -1) {
		sys_keywait(25, TRUE);
	}
	
	sact.waittype = KEYWAIT_NONE;
	
	return selected_item;
}

/*
 選択肢関連の初期化
*/
void ssel_init() {
	// callbackの設定
	sact.sel.cbmove = cb_select_move;
	sact.sel.cbrelease = cb_select_release;

	// デフォルトフォント
	sact.sel.font_type = FONT_GOTHIC;
}


/*
  内部の選択肢情報をクリア
*/
void ssel_clear() {
	int i;
	
	for (i = 0; i < SEL_ELEMENT_MAX; i++) {
		g_free(sact.sel.elem[i]);
		sact.sel.elem[i] = NULL;
	}
}

/*
  登録文字列を内部選択肢情報に追加
  @param nString: 登録文字列変数
  @param wI     : 登録位置
*/
void ssel_add(int nString, int wI) {
	if ((wI >= SEL_ELEMENT_MAX -1) || (wI <= 0)) {
		//error
		return;
	}
	if (sact.sel.elem[wI] != NULL) {
		g_free(sact.sel.elem[wI]);
	}
	
	sact.sel.elem[wI] = g_strdup(v_str(nString -1));
}

/*
  選択ウィンドを開いて選択

  @param wNum: 枠,背景とするスプライト番号
  @param wChoiceSize: 選択肢文字サイズ
  @param wMenuOutSpc: 枠スプライトの外側からのピクセル数
  @param wChoiceLineSpace: 選択肢の行間
  @param wChoiceAutoMoveCursor: オープン時に自動的に移動する選択肢の番号
  @param nAlign: 行そろえ (0:左, 1:中央, 2: 右)
*/
int ssel_select(int wNum, int wChoiceSize, int wMenuOutSpc, int wChoiceLineSpace, int wChoiceAutoMoveCursor, int nAlign) {
	int ret = 0;
	boolean saveflag;
	
	// check sprite number is sane
	if (wNum >= (SPRITEMAX-1) || wNum <= 0) return ret;
	
	// check sprite is set
	if (sact.sp[wNum] == NULL) return ret;
	
	// must be normal sprite
	if (sact.sp[wNum]->type != SPRITE_NORMAL) return ret;
	
	sact.sel.spno = wNum;
	sact.sel.font_size = wChoiceSize;
	sact.sel.frame_dot = wMenuOutSpc;
	sact.sel.linespace = wChoiceLineSpace;
	sact.sel.movecursor = wChoiceAutoMoveCursor;
	sact.sel.align      = nAlign;

	// 古い sprite の表示フラグを保存
	saveflag = sact.sp[wNum]->show;
	sact.sp[wNum]->show = TRUE;
	setup_selwindow();
	
	ret = sel_main();

	// 表示フラグを元に戻す
	sact.sp[wNum]->show = saveflag;
	
	remove_selwindow();
	
	return ret;
}
