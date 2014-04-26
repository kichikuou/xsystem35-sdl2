/*
 * sprite_update.c: spriteの通常更新いろいろ
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
/* $Id: sprite_update.c,v 1.1 2003/04/22 16:29:52 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <glib.h>

#include "portab.h"
#include "system.h"
#include "counter.h"
#include "ags.h"
#include "graphics.h"
#include "sact.h"
#include "surface.h"
#include "ngraph.h"
#include "sprite.h"

// スプライト再描画間の間に変更のあったスプライトの領域の和
static GSList *updatearea;

static void intersection(MyRectangle *r1, MyRectangle *r2, MyRectangle *rst);
static void disjunction(gpointer region, gpointer data);
static void get_updatearea();
static void do_update_each(gpointer data, gpointer userdata);

// 領域１と領域２の積を計算
static void intersection(MyRectangle *r1, MyRectangle *r2, MyRectangle *rst) {
        int x1 = max(r1->x, r2->x);
        int x2 = min(r1->x + r1->width, r2->x + r2->width);
        int y1 = max(r1->y, r2->y);
        int y2 = min(r1->y + r1->height, r2->y + r2->height);
	
        rst->x = x1;
	rst->y = y1;
	rst->width  = x2 - x1;
	rst->height = y2 - y1;
}

// 領域１と領域２をすべて含む矩形領域を計算
static void disjunction(gpointer region, gpointer data) {
	MyRectangle *r1 = (MyRectangle *)region;
	MyRectangle *r2 = (MyRectangle *)data;
	int x1, x2, y1, y2;
	
	//WARNING("r1x=%d,r1y=%d,r1w=%d,r1h=%d\n", r1->x, r1->y, r1->width, r1->height);
	//WARNING("r2x=%d,r2y=%d,r2w=%d,r2h=%d\n", r2->x, r2->y, r2->width, r2->height);
	
	if (r2->width == 0) {
		r2->x = r1->x;
		r2->y = r1->y;
		r2->width = r1->width;
		r2->height = r1->height;
		return;
	}
	
	x1 = min(r1->x, r2->x);
	x2 = max(r1->x + r1->width,  r2->x + r2->width);
	y1 = min(r1->y, r2->y);
	y2 = max(r1->y + r1->height, r2->y + r2->height);
	
	r2->x = x1;
	r2->y = y1;
	r2->width  = x2 - x1;
	r2->height = y2 - y1;
	
	//WARNING("res:r2x=%d,r2y=%d,r2w=%d,r2h=%d\n", r2->x, r2->y, r2->width, r2->height);
}

// 更新の必要なスプライトの領域の和をとってクリッピングする
static void get_updatearea() {
	MyRectangle clip = {0, 0, 0, 0};
	MyRectangle rsf0 = {0, 0, sf0->width, sf0->height};
	
	g_slist_foreach(updatearea, disjunction, &clip);
	
	g_slist_free(updatearea);
	updatearea = NULL;
	
	// surface0との領域の積をとる
	intersection(&rsf0, &clip, &sact.updaterect);
	
	WARNING("clipped area x=%d y=%d w=%d h=%d\n",
		sact.updaterect.x, sact.updaterect.y,
		sact.updaterect.width, sact.updaterect.height);
	
	return;
}

// updatelist に登録してあるすべてのスプライトを更新
static void do_update_each(gpointer data, gpointer userdata) {
	sprite_t *sp = (sprite_t *)data;
	
	// 非表示の場合はなにもしない
	if (!sp->show) return;
	
	if (sp == sact.draggedsp) return; // drag中のスプライトは最後に表示
		
	// スプライト毎のupdateルーチンの呼び出し
	if (sp->update) {
		sp->update(sp);
	}
}

/*
  画面全体の更新
  @param syncscreen: surface0 に描画したものを Screen に反映させるかどうか
 */
int sp_update_all(boolean syncscreen) {

	// スプライト移動がある場合は移動開始
	if (sact.movelist) {
		// 移動開始時間を合わせる
		sact.movestarttime = get_high_counter(SYSTEMCOUNTER_MSEC);
		g_slist_foreach(sact.movelist, spev_move_setup, NULL);
		g_slist_free(sact.movelist);
		sact.movelist = NULL;
	}

	// 画面全体を更新領域に
	sact.updaterect.x = 0;
	sact.updaterect.y = 0;
	sact.updaterect.width  = sf0->width;
	sact.updaterect.height = sf0->height;
	
	// updatelistに登録してあるスプライトを再描画
	// updatelistはスプライトの番号順に並んでいる
	g_slist_foreach(sact.updatelist, do_update_each, NULL);

	// このルーチンが呼ばれるときはスプライトはドラッグ中ではない
	
	// screenと同期は必要なときは画面全体をWindowへ転送
	if (syncscreen) {
		ags_updateFull();
	}
	
	// 移動中のすべてのスプライトが移動終了するまで待つ
	// こうしないと動きが同期しない 
	spev_wait4moving_sp();
	
	return OK;
}

/*
  画面の一部を更新
   updateme(_part)で登録した更新が必要なspriteの和の領域をupdate
*/
int sp_update_clipped() {
	// 更新領域の確定
	get_updatearea();
	
	// 幅または高さが 0 の時はなにもしない
	if (sact.updaterect.width == 0 || sact.updaterect.height == 0) {
		return OK;
	}

	// 更新領域に入っているスプライトの再描画
	g_slist_foreach(sact.updatelist, do_update_each, NULL);
	
	// drag中のスプライトを最後に描画
	if (sact.draggedsp) {
		sact.draggedsp->update(sact.draggedsp);
	}
	
	// 更新領域を Window に転送
	ags_updateArea(sact.updaterect.x, sact.updaterect.y, sact.updaterect.width, sact.updaterect.height);
	
	return OK;
}

/*
  sprite全体の更新を登録
  @param sp: 更新するスプライト
*/
int sp_updateme(sprite_t *sp) {
	MyRectangle *r;
	
	if (sp == NULL) return NG;
	if (sp->cursize.width == 0 || sp->cursize.height == 0) return NG;
	
	r = g_new(MyRectangle, 1);
	r->x = sp->cur.x;
	r->y = sp->cur.y;
	r->width = sp->cursize.width;
	r->height = sp->cursize.height;
	
	updatearea = g_slist_append(updatearea, r);
	
	WARNING("x = %d, y = %d, spno = %d w=%d,h=%d\n",
		r->x, r->y, sp->no, r->width, r->height);
	
	return OK;
}

/*
  spriteの一部更新を登録
  @param sp: 更新するスプライト
  @param x: 更新領域Ｘ座標
  @param y: 更新領域Ｙ座標
  @param w: 更新領域幅
  @param h: 更新領域高さ
*/
int sp_updateme_part(sprite_t *sp, int x, int y, int w, int h) {
	MyRectangle *r;
	
	if (sp == NULL) return NG;
	if (w == 0 || h == 0) return NG;
	
	r = g_new(MyRectangle, 1);
	r->x = sp->cur.x + x;
	r->y = sp->cur.y + y;
	r->width = w;
	r->height = h;
	
	updatearea = g_slist_append(updatearea, r);
	
	WARNING("x = %d, y = %d, spno = %d w=%d,h=%d\n",
		r->x, r->y, sp->no, r->width, r->height);
	
	return OK;
}

