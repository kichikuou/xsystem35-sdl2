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
/* $Id: sprite_update.c,v 1.1 2003/11/09 15:06:12 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>

#include "portab.h"
#include "system.h"
#include "list.h"
#include "ags.h"
#include "sprite.h"

// スプライト再描画間の間に変更のあったスプライトの領域の和
static SList *updatearea;

// 再描画するスプライトのリスト
static SList *updatelist;

static void disjunction(void* region, void* data);
static SDL_Rect get_updatearea();
static void do_update_each(void* data, void* userdata);

// 領域１と領域２をすべて含む矩形領域を計算
static void disjunction(void* region, void* data) {
	SDL_Rect *r1 = (SDL_Rect *)region;
	SDL_Rect *r2 = (SDL_Rect *)data;
	SDL_UnionRect(r1, r2, r2);
	free(r1);
}

// 更新の必要なスプライトの領域の和をとってクリッピングする
static SDL_Rect get_updatearea() {
	SDL_Rect clip = {0, 0, 0, 0};
	SDL_Rect screen = {0, 0, main_surface->w, main_surface->h};
	SDL_Rect result;
	
	slist_foreach(updatearea, disjunction, &clip);
	
	slist_free(updatearea);
	updatearea = NULL;
	
	// surface0との領域の積をとる
	SDL_IntersectRect(&screen, &clip, &result);
	
	SACT_DEBUG("clipped area x=%d y=%d w=%d h=%d",
		result.x, result.y, result.w, result.h);
	
	return result;
}

// updatelist に登録してあるすべてのスプライトを更新
static void do_update_each(void* data, void* userdata) {
	sprite_t *sp = (sprite_t *)data;
	SDL_Rect *r = (SDL_Rect *)userdata;
	
	// 非表示の場合はなにもしない
	if (!sp->show) return;
	
	// スプライト毎のupdateルーチンの呼び出し
	if (sp->update) {
		sp->update(sp, r);
	}
}

/*
  画面全体の更新
  @param syncscreen: surface0 に描画したものを Screen に反映させるかどうか
 */
void nt_sp_update_all(bool syncscreen) {
	// 画面全体を更新領域に
	SDL_Rect r = {0, 0, main_surface->w, main_surface->h };
	
	// updatelistに登録してあるスプライトを再描画
	// updatelistはスプライトの番号順に並んでいる
	slist_foreach(updatelist, do_update_each, &r);
	
	// このルーチンが呼ばれるときはスプライトはドラッグ中ではない
	
	// screenと同期は必要なときは画面全体をWindowへ転送
	if (syncscreen) {
		ags_updateFull();
	}
}

/*
  画面の一部を更新
   updateme(_part)で登録した更新が必要なspriteの和の領域をupdate
*/
void nt_sp_update_clipped() {
	SDL_Rect r;
	
	// 更新領域の確定
	r = get_updatearea();
	
	if (SDL_RectEmpty(&r))
		return;

	// 更新領域に入っているスプライトの再描画
	slist_foreach(updatelist, do_update_each, &r);
	
	// 更新領域を Window に転送
	ags_updateArea(r.x, r.y, r.w, r.h);
}

/*
  sprite全体の更新を登録
  @param sp: 更新するスプライト
*/
void nt_sp_updateme(sprite_t *sp) {
	SDL_Rect *r;
	
	if (sp == NULL) return;
	if (sp->width == 0 || sp->height == 0) return;
	
	r = malloc(sizeof(SDL_Rect));
	r->x = sp->cur.x;
	r->y = sp->cur.y;
	r->w = sp->width;
	r->h = sp->height;
	
	updatearea = slist_append(updatearea, r);
	
	SACT_DEBUG("x = %d, y = %d, spno = %d w=%d,h=%d",
		r->x, r->y, sp->no, r->w, r->h);
}

/*
  spriteの一部更新を登録
  @param sp: 更新するスプライト
  @param x: 更新領域Ｘ座標
  @param y: 更新領域Ｙ座標
  @param w: 更新領域幅
  @param h: 更新領域高さ
*/
void nt_sp_updateme_part(sprite_t *sp, int x, int y, int w, int h) {
	SDL_Rect *r;
	
	if (sp == NULL) return;
	if (w == 0 || h == 0) return;
	
	r = malloc(sizeof(SDL_Rect));
	r->x = sp->cur.x + x;
	r->y = sp->cur.y + y;
	r->w = w;
	r->h = h;
	
	updatearea = slist_append(updatearea, r);
	
	SACT_DEBUG("x = %d, y = %d, spno = %d w=%d,h=%d",
		r->x, r->y, sp->no, r->w, r->h);
}

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

void nt_sp_add_updatelist(sprite_t *sp) {
	updatelist = slist_insert_sorted(updatelist, sp, compare_spriteno_smallfirst);
}

void nt_sp_remove_updatelist(sprite_t *sp) {
	updatelist = slist_remove(updatelist, sp);
}

void nt_sp_clear_updatelist(void) {
	slist_free(updatelist);
	updatelist = NULL;
	slist_free(updatearea);
	updatearea = NULL;
}

// デフォルトの壁紙update
void nt_sp_draw_wall(sprite_t *sp, SDL_Rect *area) {
	SDL_FillRect(main_surface, area, SDL_MapRGB(main_surface->format, 0, 0, 0));
}
