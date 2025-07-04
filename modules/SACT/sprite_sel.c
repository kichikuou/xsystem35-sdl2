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
#include <stdlib.h>
#include <string.h>
#include <SDL.h>

#include "portab.h"
#include "nact.h"
#include "ags.h"
#include "input.h"
#include "sact.h"
#include "sprite.h"
#include "drawtext.h"

// 選択された要素(1~) キャンセルの場合は0、初期状態は -1
static int selected_item;
static int selected_item_cur;

// 前のカーソルの状態
static bool oldstate; // spriteの中か外か
static int oldindex; // 何番目の要素か(0~)


static bool sp_is_insprite2(sprite_t *sp, int x, int y, int margin);
static void cb_select_move(agsevent_t *e);
static void cb_select_release(agsevent_t *e);
static void setup_selwindow();
static void remove_selwindow();
static int sel_main();




/**
 *  内側にマージンを含む sprite内領域チェック
 *  マージン内はsprite内部とは判断しない
 */
static bool sp_is_insprite2(sprite_t *sp, int x, int y, int margin) {
	SDL_Point p = {x, y};
	SDL_Rect r = {
		sp->cur.x + margin,
		sp->cur.y + margin,
		sp->curcg->sf->w - 2 * margin,
		sp->curcg->sf->h - 2 * margin
	};
	return SDL_PointInRect(&p, &r);
}

// マウスが移動したときの callback
static void cb_select_move(agsevent_t *e) {
	int x = e->mousex, y = e->mousey;
	sprite_t *sp = sact.sp[sact.sel.spno];
	bool newstate;
	int newindex;
	
	// sprite内か？
	newstate = sp_is_insprite2(sp, x, y, sact.sel.frame_dot);
	newindex = (y - (sp->cur.y + sact.sel.frame_dot)) / (sact.sel.font_size + sact.sel.linespace);
	
	if (newstate == oldstate) {
		if (!newstate || newindex == oldindex) {
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
	int x = e->mousex, y = e->mousey;
	sprite_t *sp = sact.sp[sact.sel.spno];
	bool st;
	int iy;
	
	switch (e->code) {
	case AGSEVENT_BUTTON_LEFT:
		st = sp_is_insprite2(sp, x, y, sact.sel.frame_dot);
		iy = (y - (sp->cur.y + sact.sel.frame_dot)) / (sact.sel.font_size + sact.sel.linespace);

		// カーソルが sprite の外の場合は無視
		if (!st) {
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

static void draw_box(SDL_Surface *dst, int x, int y, int w, int h) {
	SDL_FillRect(dst, &(SDL_Rect){x, y, w, h}, SDL_MapRGB(dst->format, 0, 0, 0));
	SDL_Rect border_rects[4] = {
		{x, y, w, 1},
		{x, y + h - 1, w, 1},
		{x, y, 1, h},
		{x + w - 1, y, 1, h}
	};
	SDL_FillRects(dst, border_rects, 4, SDL_MapRGB(dst->format, 255, 255, 255));
}

// 選択ウィンドを更新するときの callback
static void update_selwindow(sprite_t *sp) {
	int selno = selected_item_cur;
	int x0, y0;

	x0 = sp->cur.x;
	y0 = sp->cur.y;
	// 背景 CG
	sp_draw(sp);
	
	// 選択されている要素
	if (selno && sact.sel.elem[selno] != NULL) {
		int w = sact.sel.charcanvas->w - 2 * sact.sel.frame_dot;
		int h = sact.sel.font_size + sact.sel.linespace;
		int x = x0 + sact.sel.frame_dot;
		int y = y0 + sact.sel.frame_dot + (selno -1) * h;
		draw_box(main_surface, x, y, w, h);
	}
	
	// 選択肢文字列
	SDL_BlitSurface(sact.sel.charcanvas, NULL, main_surface,
		&(SDL_Rect){x0, y0, sact.sel.charcanvas->w, sact.sel.charcanvas->h});
}

// 選択ウィンドの準備
static void setup_selwindow() {
	sprite_t *sp = sact.sp[sact.sel.spno];
	int i;
	
	// 選択肢文字用 canvas
	sact.sel.charcanvas = SDL_CreateRGBSurfaceWithFormat(0, sp->cg1->sf->w, sp->cg1->sf->h, 32, SDL_PIXELFORMAT_ARGB8888);
	SDL_SetSurfaceBlendMode(sact.sel.charcanvas, SDL_BLENDMODE_BLEND);
	
	dt_setfont(sact.sel.font_type, sact.sel.font_size);
	
	// 指定のスプライトに文字(選択肢を書く)
	for (i = 1; i < SEL_ELEMENT_MAX; i++) {
		int x, y;
		if (sact.sel.elem[i] == NULL) continue;
		// 文字の場所計算
		x = 0; // 行そろえは無し
		y = (i - 1) * (sact.sel.font_size + sact.sel.linespace);
		dt_drawtext_col(sact.sel.charcanvas,
			    x + sact.sel.frame_dot, y + sact.sel.frame_dot,
			    sact.sel.elem[i], 255, 255, 255);
	}
	
	// デフォルトで選択される選択肢がある場合、そこへカーソルを移動
	if (sact.sel.movecursor) {
		int x = sp->cur.x + sact.sel.frame_dot + 2;
		int y = sp->cur.y + sact.sel.frame_dot + 2 +
			(sact.sel.font_size + sact.sel.linespace) * (sact.sel.movecursor - 1);
		ags_setCursorLocation(x, y, true, true);
		selected_item = (sact.sel.movecursor -1);
		oldstate = true;
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
	if (sact.sel.charcanvas)
		SDL_FreeSurface(sact.sel.charcanvas);
	sact.sel.charcanvas = NULL;
}


// 選択メインループ
static int sel_main() {
	sact.waittype = KEYWAIT_SELECT;
	sact.waitkey = -1;

	selected_item = -1;
	
	while(selected_item == -1) {
		sys_keywait(25, KEYWAIT_CANCELABLE);
		if (nact->is_quit)
			selected_item = 0;
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

void ssel_reset(void) {
	ssel_clear();
	if (sact.sel.charcanvas) {
		SDL_FreeSurface(sact.sel.charcanvas);
		sact.sel.charcanvas = NULL;
	}
}

/*
  内部の選択肢情報をクリア
*/
void ssel_clear() {
	int i;
	
	for (i = 0; i < SEL_ELEMENT_MAX; i++) {
		free(sact.sel.elem[i]);
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
		free(sact.sel.elem[wI]);
	}
	
	sact.sel.elem[wI] = strdup(svar_get(nString));
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
	bool saveflag;
	
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
	sact.sp[wNum]->show = true;
	setup_selwindow();
	
	ret = sel_main();

	// 表示フラグを元に戻す
	sact.sp[wNum]->show = saveflag;
	
	remove_selwindow();
	
	return ret;
}
