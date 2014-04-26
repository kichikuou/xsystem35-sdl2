/*
 * sprite_anime.c: アニメーションスプライト特有の処理
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
/* $Id: sprite_anime.c,v 1.1 2003/04/22 16:29:52 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <glib.h>

#include "portab.h"
#include "system.h"
#include "ags.h"
#include "counter.h"
#include "sact.h"
#include "sprite.h"

static int eventCB_ANIM(sprite_t *sp, agsevent_t *e);
static void cb_remove(sprite_t *sp);


// アニメーションスプライト
static int eventCB_ANIM(sprite_t *sp, agsevent_t *e) {
	int now;
	
	// コールバックがタイマイベントかどうか
	if (e->type != AGSEVENT_TIMER) return 0;
	
	// 現在時刻の取得
	now = get_high_counter(SYSTEMCOUNTER_MSEC);
	
	// 指定時間までスキップ
	if ((now - sp->u.anime.starttime) < sp->u.anime.interval) return 0;
	
	// 新しい時間を保存
	sp->u.anime.starttime = now;
	
	// 次に表示するCGをセット
	switch(sp->u.anime.tick % sp->u.anime.npat) {
	case 0:
		sp->curcg = sp->cg1; break;
	case 1:
		sp->curcg = sp->cg2; break;
	case 2:
		sp->curcg = sp->cg3; break;
	}
	
	// WARNING("anime update\n");
	
	// カウントアップ
	sp->u.anime.tick++;
	
	sp_updateme(sp);
	
	return 1;
}

// スプライト削除時の処理
static void cb_remove(sprite_t *sp) {
	spev_remove_teventlistener(sp);
}

/*
  sp_new の時にスプライトの種類毎の初期化
  @param sp: 初期化するスプライト
*/
int sp_anime_setup(sprite_t *sp) {
	int n = 0;
	
	sp->u.anime.interval = 500; // デフォルトの間隔 0.5秒
	sp->u.anime.starttime = get_high_counter(SYSTEMCOUNTER_MSEC); // 開始時刻
	sp->u.anime.tick = 0;      // カウンタ初期化
	
	// アニメパターンはいくつあるか
	if (sp->cg1) n++;
	if (sp->cg2) n++;
	if (sp->cg3) n++;
	sp->u.anime.npat = n;
	
	spev_add_teventlistener(sp, eventCB_ANIM);
	sp->remove = cb_remove;
	
	return OK;
}
