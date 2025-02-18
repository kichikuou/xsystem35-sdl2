/*
 * sprite.c: スプライト基本各種処理
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
/* $Id: sprite.c,v 1.5 2003/11/16 15:29:52 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>

#include "portab.h"
#include "system.h"
#include "list.h"
#include "ngraph.h"
#include "ags.h"
#include "nact.h"
#include "sact.h"
#include "sprite.h"
#include "surface.h"
#include "sactcg.h"
#include "sactsound.h"

static int compare_spriteno_smallfirst(const void *a, const void *b);


#define SP_ASSERT_NO(no) do {                                        \
  if ((no) >= SPRITEMAX) {                                           \
    WARNING("no is too large (should be %d < %d)", no, SPRITEMAX); \
    return;                                                       \
  }                                                                  \
} while (0)


// スプライトの番号順に更新するためにリストに順番に要れるためのcallbck
static int compare_spriteno_smallfirst(const void *a, const void *b) {
	sprite_t *sp1 = (sprite_t *)a;
	sprite_t *sp2 = (sprite_t *)b;
	
	if (sp1->no < sp2->no) {
		return -1;
	}
	if (sp1->no > sp2->no) {
		return 1;
	}
	return 0;
}

// デフォルトの壁紙update
static void sp_draw_wall(sprite_t *sp) {
	int sx, sy, w, h;
	
	sx = sact.updaterect.x;
	sy = sact.updaterect.y;
	w = sact.updaterect.w;
	h = sact.updaterect.h;
	gr_fill(sf0, sx, sy, w, h, 0, 0, 0);
	
	SACT_DEBUG("do update no=%d, sx=%d, sy=%d, w=%d, h=%d",
		sp->no, sx, sy, w, h);
}

/**
 * sprite 関連の初期化
 */
void sp_init(void) {
	int i;
	
	// DLL用メッセージ表示
	nact->msgout = smsg_add;

	// mouse/key event handler
	nact->ags.eventcb = spev_callback;

	// main callback
	nact->callback = spev_main;
	
	// いろいろな理由から全てのスプライトをあらかじめ作成しておく
	for (i = 0; i < SPRITEMAX; i++) {
		sact.sp[i] = calloc(1, sizeof(sprite_t));
		sact.sp[i]->no   = i;
		sact.sp[i]->type = SPRITE_NONE;
		sact.sp[i]->show = false;
	}
	
	// 壁紙(スプライト番号０)はデフォルトへ
	sp_set_wall_paper(0);
	
	// 壁紙を updateリストに追加
	sact.updatelist = slist_append(sact.updatelist, sact.sp[0]);
}

void sp_reset(void) {
	sp_clear_zkey_hidesprite_all();
	sp_clear_quakesprite_all();
	sp_free_all();
}

/**
 * 新規スプライトの作成
 * @param no: スプライト番号
 * @param cg1: 1枚目のCG
 * @param cg2: 2枚目のCG (ない場合は0)
 * @param cg3: 3枚目のCG (ない場合は0)
 * @param type: スプライトの種類
 */
void sp_new(int no, int cg1, int cg2, int cg3, int type) {
	sprite_t *sp;
	
	SP_ASSERT_NO(no);

	sp = sact.sp[no];

	if (sp->type != SPRITE_NONE) {
		sp_free(no);
	}
	
	// 更新リストに登録
	sact.updatelist = slist_insert_sorted(sact.updatelist, sp, compare_spriteno_smallfirst);
	
	sp->type = type;
	sp->no   = no;
	
	// set時点でのcgが使用される(draw時ではない)
	sp->cg1 = cg1 ? scg_addref(cg1) : NULL;
	sp->cg2 = cg2 ? scg_addref(cg2) : NULL;
	sp->cg3 = cg3 ? scg_addref(cg3) : NULL;
	
	//初期のcurcgはcg1
	sp->curcg = sp->cg1;
	
	sp->show = true; // 初期状態は表示
	sp->blendrate = 255; // ブレンド無し
	sp->loc.x = 0;   // 初期表示位置は(0,0)
	sp->loc.y = 0;
	sp->cur = sp->loc;
	
	// cg1の大きさをスプライトの大きさとする
	if (sp->curcg == NULL) {
		sp->cursize.width = 0;
		sp->cursize.height = 0;
	} else {
		sp->cursize.width = sp->curcg->sf->w;
		sp->cursize.height = sp->curcg->sf->h;
	}
	
	sp->freezed_state = 0; // 状態固定は無し
	sp->update = DEFAULT_UPDATE;  // default の updateルーチン
	
	// 各スプライトタイプ毎の初期化
	switch(type) {
	case SPRITE_SWITCH:
		sp_sw_setup(sp);
		break;
		
	case SPRITE_GETA:
	case SPRITE_GETB:
		sp_get_setup(sp);
		break;
		
	case SPRITE_PUT:
	case SPRITE_SWPUT:
		sp_put_setup(sp);
		break;
		
	case SPRITE_ANIME:
		sp_anime_setup(sp);
		break;
	}
}

// メッセージスプライトの作成
void sp_new_msg(int no, int x, int y, int width, int height) {
	sprite_t *sp;
	
	SP_ASSERT_NO(no);
	
	sp = sact.sp[no];
	
	if (sp->type != SPRITE_NONE) {
		sp_free(no);
	}
	// 更新リストに登録
	sact.updatelist = slist_insert_sorted(sact.updatelist, sp, compare_spriteno_smallfirst);
	
	
	sp->type = SPRITE_MSG;
	sp->no   = no;
	sp->show = true; // 初期状態は表示
	sp->blendrate = 255; // ブレンド無し
	sp->freezed_state = 0; // 状態固定無し
	sp->loc.x = x - sact.origin.x; // 初期表示位置
	sp->loc.y = y - sact.origin.y;
	sp->u.msg.dspcur.x = 0; // 文字描画開始位置
	sp->u.msg.dspcur.y = 0;
	sp->cursize.width = width;  // スプライトの大きさ
	sp->cursize.height = height;
	sp->cur = sp->loc;
	sp->u.msg.buf = NULL;

	// canvas for drawing text
	sp->u.msg.canvas = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_ARGB8888);

	// スプライト再描画用コールバック
	sp->update = smsg_update;
}

// 壁紙の設定
void sp_set_wall_paper(int no) {
	sprite_t *sp = sact.sp[0];
	
	if (sp->curcg)
		scg_deref(sp->curcg);
	
	sp->curcg = no ? scg_addref(no) : NULL;

	if (sp->curcg) { // display specified CG
		sp->update = DEFAULT_UPDATE;
		sp->cursize.width  = sp->curcg->sf->w;
		sp->cursize.height = sp->curcg->sf->h;
	} else { // Black
		sp->cursize.height = sf0->height;
		sp->curcg = NULL;
		sp->update = sp_draw_wall;
	}
	
	sp->type = SPRITE_WP;
	sp->show = true;
	sp->blendrate = 255;
	sp->cur.x = 0;
	sp->cur.y = 0;
}

// 全ての sprite を消去
void sp_free_all(void) {
	int i;
	
	for (i = 1; i < SPRITEMAX; i++) {
		sp_free(i);
	}
}

// 指定のsprite を消去
void sp_free(int no) {
	sprite_t *sp;
	
	SP_ASSERT_NO(no);
	
	sp = sact.sp[no];

	// 移動開始していない場合はリストから削除
	if (!sp->move.moving) {
		sact.movelist = slist_remove(sact.movelist, sp);
	}
	
	// CGオブジェクトの削除
	if (sp->cg1) scg_deref(sp->cg1);
	if (sp->cg2) scg_deref(sp->cg2);
	if (sp->cg3) scg_deref(sp->cg3);
	sp->cg1 = sp->cg2 = sp->cg3 = NULL;
	
	// remove時の処理があれば実行
	if (sp->remove) {
		sp->remove(sp);
	}
	
	// 説明スプライトの削除
	//   ここで消しちゃまずいかも
	slist_free(sp->expsp);
	sp->expsp = NULL;
	
	if (sp->type == SPRITE_MSG) {
		slist_free(sp->u.msg.buf);
		SDL_FreeSurface(sp->u.msg.canvas);
	}
	sact.updatelist = slist_remove(sact.updatelist, sp);
	
	// SACT.Numeral_XXX は残しておく
	{
		sprite_t back;
		memcpy(&(back.numeral), &(sp->numeral), sizeof(sp->numeral));
		memset(sp, 0, sizeof(sprite_t));
		sp->type = SPRITE_NONE;
		sp->no = no;
		sp->show = false;
		memcpy(&(sp->numeral), &(back.numeral), sizeof(sp->numeral));
	}
}

// 表示状態の変更
void sp_set_show(int wNum, int wCount, int sShow) {
	int i;
	bool oldstate;
	sprite_t *sp;
	
	SP_ASSERT_NO(wNum);
	
	for (i = wNum; i < (wNum + wCount); i++) {
		if (i >= (SPRITEMAX -1)) break;
		sp = sact.sp[i];
		oldstate = sp->show;
		
		sp->show = sShow == 1;
	}
}

// 表示位置の設定
void sp_set_pos(int wNum, int wX, int wY) {
	sprite_t *sp;
	
	SP_ASSERT_NO(wNum);
	
	sp = sact.sp[wNum];
	sp->loc.x = wX - sact.origin.x;
	sp->loc.y = wY - sact.origin.y;
	sp->cur.x = sp->loc.x;
	sp->cur.y = sp->loc.y;
}

// スプライトの移動
void sp_set_move(int wNum, int wX, int wY) {
	sprite_t *sp;
	
	SP_ASSERT_NO(wNum);
	
	sp = sact.sp[wNum];
	sp->move.to.x = wX - sact.origin.x;
	sp->move.to.y = wY - sact.origin.y;
	
	if (!sp->move.time && !sp->move.speed)
		sp->move.speed = 100;
	
	sp->cur = sp->loc;
	
	// moveするスプライトリストに登録
	// 実際に move を開始するのは ~SP_DRAW(sp_update_all)が呼ばれたとき
	sact.movelist = slist_append(sact.movelist, sp);
}

// スプライト移動時間の設定
void sp_set_movetime(int wNum, int wTime) {
	SP_ASSERT_NO(wNum);
	
	sact.sp[wNum]->move.time = wTime * 10;
	sact.sp[wNum]->move.speed = 0;
}

// スプライト移動速度の設定
void sp_set_movespeed(int wNum, int wSpeed) {
	SP_ASSERT_NO(wNum);
	
	if (wSpeed == 0) wSpeed = 1;
	
	sact.sp[wNum]->move.speed = wSpeed;
	sact.sp[wNum]->move.time = 0;
}

// Zキーを押したときに隠すスプライトの登録
void sp_add_zkey_hidesprite(int wNum) {
	sprite_t *sp;
	
	SP_ASSERT_NO(wNum);
	sp = sact.sp[wNum];

	// 登録時点でまだ生成していないスプライトは隠さない
	//   シェルクレイルでまずいのがあったので中止
	// if (sp->type == SPRITE_NONE) return;
	
	sact.sp_zhide = slist_append(sact.sp_zhide, sp);
}

// 上で登録したスプライトの削除
void sp_clear_zkey_hidesprite_all(void) {
	slist_free(sact.sp_zhide);
	sact.sp_zhide = NULL;
}

// スプライト状態の固化
void sp_freeze_sprite(int wNum, int wIndex) {
	sprite_t *sp;
	void *oldstate;
	
	SP_ASSERT_NO(wNum);
	
	sp = sact.sp[wNum];
	sp->freezed_state = wIndex;

	oldstate = (void *)sp->curcg;
	switch(wIndex) {
	case 1:
		sp->curcg = sp->cg1; break;
	case 2:
		sp->curcg = sp->cg2; break;
	case 3:
		sp->curcg = sp->cg3; break;
	}
}

// 上で固化した状態の解除
void sp_thaw_sprite(int wNum) {
	SP_ASSERT_NO(wNum);
	
	sact.sp[wNum]->freezed_state = 0;
}

// SP_QUAKEで揺らすスプライトの登録
void sp_add_quakesprite(int wNum) {
	SP_ASSERT_NO(wNum);
	
	sact.sp_quake = slist_append(sact.sp_quake, sact.sp[wNum]);
}

// 上で登録したスプライトの削除
void sp_clear_quakesprite_all(void) {
	slist_free(sact.sp_quake);
	sact.sp_quake = NULL;
}

// アニメーションスプライトの間隔の設定
void sp_set_animeinterval(int wNum, int wTime) {
	SP_ASSERT_NO(wNum);

	if (sact.sp[wNum]->type != SPRITE_ANIME) return;
	
	sact.sp[wNum]->u.anime.interval = wTime * 10;
}

// スプライトのブレンド率の設定
void sp_set_blendrate(int wNum, int wCount, int rate) {
	int i;
	sprite_t *sp;
	
	SP_ASSERT_NO(wNum);
	
	for (i = wNum; i < (wNum + wCount); i++) {
		if (i >= (SPRITEMAX -1)) break;
		sp = sact.sp[i];
		sp->blendrate = rate;
	}
}

// スプライトが create されているかどうかの取得
bool sp_exists(int wNum) {
	if (wNum >= SPRITEMAX) return false;
	if (sact.sp[wNum]->type == SPRITE_NONE) return false;
	return true;
}

// スプライトのタイプと何番のCGがセットされているかの取得
bool sp_query_info(int wNum, int *vtype, int *vcg1, int *vcg2, int *vcg3) {
	sprite_t *sp;
	
	if (wNum >= SPRITEMAX) goto errexit;

	sp = sact.sp[wNum];
	if (sp->type == SPRITE_NONE) goto errexit;
	
	*vtype = sp->type;
	*vcg1 = sp->cg1 ? sp->cg1->no : 0;
	*vcg2 = sp->cg2 ? sp->cg2->no : 0;
	*vcg3 = sp->cg3 ? sp->cg3->no : 0;
	
	return true;
	
 errexit:
	*vtype = 0;
	*vcg1 = 0;
	*vcg2 = 0;
	*vcg3 = 0;
	return false;
}

// スプライトの表示状態の取得
bool sp_query_show(int wNum, int *vShow) {
	if (wNum >= SPRITEMAX) goto errexit;
	if (sact.sp[wNum]->type == SPRITE_NONE) goto errexit;

	*vShow = sact.sp[wNum]->show ? 1: 0;
	return true;
	
 errexit:
	*vShow = 0;
	return false;
}

// スプライトの表示位置の取得
bool sp_query_pos(int wNum, int *vx, int *vy) {
	if (wNum >= SPRITEMAX) goto errexit;
	if (sact.sp[wNum]->type == SPRITE_NONE) goto errexit;

	*vx = sact.sp[wNum]->loc.x;
	*vy = sact.sp[wNum]->loc.y;
	return true;

 errexit:
	*vx = 0;
	*vy = 0;
	return false;
}

// スプライトの大きさの取得
bool sp_query_size(int wNum, int *vw, int *vh) {
	sprite_t *sp;
	
	if (wNum >= SPRITEMAX) goto errexit;

	sp = sact.sp[wNum];

	if (sp->type == SPRITE_NONE) goto errexit;

	*vw = sp->cursize.width;
	*vh = sp->cursize.height;
	
	return true;
	
 errexit:
	*vw = 0;
	*vh = 0;
	return false;
}

// テキストスプライトの現在の文字表示位置の取得
bool sp_query_textpos(int wNum, int *vx, int *vy) {
	if (wNum >= SPRITEMAX) goto errexit;
	sprite_t *sp = sact.sp[wNum];
	if (sp->type != SPRITE_MSG) goto errexit;

	*vx = sp->u.msg.dspcur.x + sp->loc.x;
	*vy = sp->u.msg.dspcur.y + sp->loc.y;
	return true;
	
 errexit:
	*vx = 0;
	*vy = 0;
	return false;
}

// NumeralXXXのCGのセット
void sp_num_setcg(int nNum, int nIndex, int nCG) {
	SP_ASSERT_NO(nNum);

	sact.sp[nNum]->numeral.cg[nIndex] = nCG;
}

// NumeralXXXのCGの取得
void sp_num_getcg(int nNum, int nIndex, int *vCG) {
	SP_ASSERT_NO(nNum);
	
	*vCG = sact.sp[nNum]->numeral.cg[nIndex];
}

// NumeralXXXの位置のセット
void sp_num_setpos(int nNum, int nX, int nY) {
	SP_ASSERT_NO(nNum);

	sact.sp[nNum]->numeral.pos.x = nX;
	sact.sp[nNum]->numeral.pos.y = nY;
}

// NumeralXXXの位置の取得
void sp_num_getpos(int nNum, int *vX, int *vY) {
	SP_ASSERT_NO(nNum);
	
	*vX = sact.sp[nNum]->numeral.pos.x;
	*vY = sact.sp[nNum]->numeral.pos.y;
}

// NumeralXXXのスパンのセット
void sp_num_setspan(int nNum, int nSpan) {
	SP_ASSERT_NO(nNum);
	
	sact.sp[nNum]->numeral.span = nSpan;
}

// NumeralXXXのスパンの取得
void sp_num_getspan(int nNum, int *vSpan) {
	SP_ASSERT_NO(nNum);

	*vSpan = sact.sp[nNum]->numeral.span;
}

// すべての説明スプライトの削除
void sp_exp_clear(void) {
	SList *node;
	
	for (node = sact.updatelist; node; node = node->next) {
		sprite_t *sp = (sprite_t *)node->data;
		if (sp == NULL) continue;
		sp_exp_del(sp->no);
	}
}

// 説明スプライトの登録
void sp_exp_add(int nNumSP1, int nNumSP2) {
	sprite_t *swsp, *expsp;
	SP_ASSERT_NO(nNumSP1);
	SP_ASSERT_NO(nNumSP2);

	swsp  = sact.sp[nNumSP1];
	expsp = sact.sp[nNumSP2];
	
	swsp->expsp = slist_append(swsp->expsp, expsp);
}

// 説明スプライトの削除
void sp_exp_del(int nNum) {
	sprite_t *sp;
	
	SP_ASSERT_NO(nNum);
	
	sp  = sact.sp[nNum];
	
	slist_free(sp->expsp);
	sp->expsp = NULL;
}

// スプライトサウンドのセット
void sp_sound_set(int wNumSP, int wNumWave1, int wNumWave2, int wNumWave3) {
	sprite_t *sp;
	
	SP_ASSERT_NO(wNumSP);

	sp  = sact.sp[wNumSP];
	sp->numsound1 = wNumWave1;
	sp->numsound2 = wNumWave2;
	sp->numsound3 = wNumWave3;
}

// すべてのスプライトサウンドの終了を待つ
void sp_sound_wait(void) {
	WARNING("NOT IMPLEMENTED");
}

// 範囲外をクリックしたときのサウンドの設定
void sp_sound_ob(int wNumWave) {
	sact.numsoundob = wNumWave;
}

/**
 * 指定の座標が現在のスプライトの位置の範囲に入っているか？
 * @param sp: 調べる対象のスプライト
 * @param x,y: 座標
 * @return: true:入っている, false: 入っていない
 */
bool sp_is_insprite(sprite_t *sp, int x, int y) {
	uint8_t *dp;
	
	if (x < 0 || y < 0 || x >= sf0->width || y >= sf0->height) return false;
	
	dp = GETOFFSET_PIXEL(sact.dmap, x, y);
	return (*(uint16_t *)dp == sp->no);
}
