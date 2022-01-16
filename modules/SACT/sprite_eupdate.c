/*
 * sprite_eupdate.c: 効果つき更新
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
/* $Id: sprite_eupdate.c,v 1.5 2003/07/20 19:30:16 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <string.h>

#include "portab.h"
#include "system.h"
#include "ngraph.h"
#include "ags.h"
#include "nact.h"
#include "image.h"
#include "input.h"
#include "sact.h"
#include "sprite.h"
#include "surface.h"
#include "sactcg.h"
#include "sdl_core.h"


static void ec6_cb(surface_t *, surface_t *);
static void ec11_cb(surface_t *, surface_t *);
static void ec12_cb(surface_t *, surface_t *);
static void ec13_cb(surface_t *, surface_t *);
static void sdlfader_cb(surface_t *, surface_t *);
static void ec_dummy_cb(surface_t *, surface_t *);

struct ecopyparam {
	int sttime;
	int curtime;
	int edtime;
	int curstep;
	int oldstep;
	struct sdl_fader *fader;
};
typedef struct ecopyparam ecopyparam_t;
static ecopyparam_t ecp;

#include "sprite_eupdate_perspect.c"
#include "sprite_eupdate_mosaic.c"
#include "sprite_eupdate_uneune.c"
#include "sprite_eupdate_buller.c"
#include "sprite_eupdate_zmbbul.c"

typedef void entrypoint (surface_t *, surface_t *);

static entrypoint *cb[39] = {
	sdlfader_cb,
	sdlfader_cb,
	sdlfader_cb,
	sdlfader_cb,
	sdlfader_cb,
	ec6_cb,
	sdlfader_cb,
	sdlfader_cb,
	sdlfader_cb,
	ec10_cb,
	ec11_cb,
	ec12_cb,
	ec13_cb,
	sdlfader_cb,
	sdlfader_cb,
	sdlfader_cb,
	sdlfader_cb,
	ec_dummy_cb,  // 欠番
	ec19_cb,
	sdlfader_cb,
	sdlfader_cb,
	sdlfader_cb,
	sdlfader_cb,
	ec_dummy_cb,
	ec25_cb,
	ec26_cb,
	ec_dummy_cb,
	ec28_cb,
	ec29_cb,
	ec_dummy_cb,
	ec31_cb,
	ec_dummy_cb,
	ec_dummy_cb,
	ec_dummy_cb,
	ec_dummy_cb,  // 35
	ec_dummy_cb,
	ec_dummy_cb,
	ec_dummy_cb,
	ec_dummy_cb
};

static void ec_dummy_cb(surface_t *sfsrc, surface_t *sfdst) {
	WARNING("NOT IMPLEMENTED\n");
}

// 上->下クロスフェード
static void ec12_cb(surface_t *src, surface_t *dst) {
#define EC12DELTA 256
	int curstep, maxstep = src->height + EC12DELTA;
	int j, st_i, ed_i, l;
	
	curstep = maxstep * (ecp.curtime - ecp.sttime)/ (ecp.edtime - ecp.sttime);
	SACT_DEBUG("step = %d\n", curstep);
	
	if (ecp.oldstep == curstep) {
		usleep(0); return;
	}
	
	st_i = max(0, curstep - EC12DELTA + 1);
	ed_i = min(src->height -1, curstep);
	l = ed_i - st_i + 1;
	for (j = st_i; j < ed_i; j++) {
		gre_Blend(sf0, 0, j, src, 0, j, dst, 0, j, src->width, 1, curstep - j);
	}
	if ((st_i - ecp.oldstep) > 1) {
		gr_copy(sf0, 0, ecp.oldstep, dst, 0, ecp.oldstep, src->width, st_i - ecp.oldstep);
		ags_updateArea(0, ecp.oldstep, src->width, st_i - ecp.oldstep);
	}
	
	ags_updateArea(0, st_i, src->width, l);
	ecp.oldstep = st_i;
}

// 下->上クロスフェード
static void ec13_cb(surface_t *src, surface_t *dst) {
#define EC13DELTA 256
	int curstep, maxstep = src->height + EC12DELTA;
	int j, st_i, ed_i, l;
	int syy1 = 0 + dst->height -1;
	int syy2 = 0 + src->height -1;
	int dyy  = 0 + sf0->height -1;
	
	curstep = maxstep * (ecp.curtime - ecp.sttime)/ (ecp.edtime - ecp.sttime);
	SACT_DEBUG("step = %d\n", curstep);
	
	if (ecp.oldstep == curstep) {
		usleep(0); return;
	}
	
	st_i = max(0, curstep - EC13DELTA + 1);
	ed_i = min(src->height -1, curstep);
	l = ed_i - st_i + 1;
	for (j = st_i; j < ed_i; j++) {
		gre_Blend(sf0, 0, dyy - j, src, 0, syy1-j, dst, 0, syy2-j, src->width, 1, curstep - j);
	}
	if ((st_i - ecp.oldstep) > 1) {
		gr_copy(sf0, 0, dyy-ecp.oldstep, dst, 0, syy2-ecp.oldstep, src->width, st_i - ecp.oldstep);
		ags_updateArea(0, dyy-ecp.oldstep, src->width, st_i - ecp.oldstep);
	}
	
	ags_updateArea(0, dyy-ed_i, src->width, l);
	ecp.oldstep = st_i;
}

static void sdlfader_cb(surface_t *sfsrc, surface_t *sfdst) {
	sdl_fader_step(ecp.fader, (double)(ecp.curtime - ecp.sttime) / (ecp.edtime - ecp.sttime));
}



//ブロックディゾルブ
static void ec24_cb(surface_t *sfsrc, surface_t *sfdst) {
}

// 振動
static void ec27_cb(surface_t *sfsrc, surface_t *sfdst) {
}

// 回転ズームブレンドブラー(SRCのみ参照
static void ec30_cb(surface_t *sfsrc, surface_t *sfdst) {
}

// ＴＶスイッチオフ(Destのみ参照)
static void ec32_cb(surface_t *sfsrc, surface_t *sfdst) {
}

// ＴＶスイッチオン(Srcのみ参照)
static void ec33_cb(surface_t *sfsrc, surface_t *sfdst) {
}

// ポリゴン爆発
static void ec34_cb(surface_t *sfsrc, surface_t *sfdst) {
}

// ノイズクロスフェード
static void ec35_cb(surface_t *sfsrc, surface_t *sfdst) {
}

// ページめくり
static void ec36_cb(surface_t *sfsrc, surface_t *sfdst) {
}

// セピアノイズクロスフェード
static void ec37_cb(surface_t *sfsrc, surface_t *sfdst) {
}

// ぐしゃぐしゃ紙右下ひっぱり
static void ec38_cb(surface_t *sfsrc, surface_t *sfdst) {
}

// 横うねうね
static void ec39_cb(surface_t *sfsrc, surface_t *sfdst) {
}

/*
  効果つき画面更新
  @param type: 効果の種類
  @param time: 実行時間
  @param cancel: キー抜け(0:なし, 1:あり)
*/
int sp_eupdate(int type, int time, int cancel) {
	surface_t *sfsrc, *sfdst;
	int key;

	if (sact.waitskiplv > 1) {
		sp_update_all(TRUE);
		return OK;
	}
	
	// 現在の sf0 をセーブ
	sfsrc = sf_dup(sf0);
	
	sp_update_all(FALSE);
	
	sfdst = sf_dup(sf0);
	
	enum sdl_effect sdl_effect = from_sact_effect(type);
	if (sdl_effect != EFFECT_INVALID) {
		SDL_Rect rect = { 0, 0, sfsrc->width, sfsrc->height };
		ecp.fader = sdl_fader_init(&rect, NULL, 0, 0, sdl_getDIB(), 0, 0, sdl_effect);
	} else {
		sf_copyall(sf0, sfsrc); // 全部の効果タイプにこの処理は要らないんだけど
	}

	// 5つを越えたら別の方法を考えよう
	if (type == 10) {
		ec10_prepare(sfsrc, sfdst);
	}
	if (type == 11) {
		ec11_prepare(sfsrc, sfdst);
	}
	if (type == 19) {
		ec19_prepare(sfsrc, sfdst);
	}
	
	ecp.sttime = ecp.curtime = sdl_getTicks();
	ecp.edtime = ecp.curtime + time*10;
	ecp.oldstep = 0;
	
	while ((ecp.curtime = sdl_getTicks()) < ecp.edtime) {
		int rest;
		
		(*cb[type-1])(sfsrc, sfdst);
		rest = 15 - (sdl_getTicks() - ecp.curtime);
		key = sys_keywait(rest, cancel ? KEYWAIT_CANCELABLE : KEYWAIT_NONCANCELABLE);
		
		if (cancel && key) break;
	}
	
	sf_copyall(sf0, sfdst);
	ags_updateFull();
	sf_free(sfsrc);
	sf_free(sfdst);

	if (type == 10) {
		ec10_drain(sfsrc, sfdst);
	}
	if (type == 11) {
		ec11_drain(sfsrc, sfdst);
	}
	if (type == 19) {
		ec19_drain(sfsrc, sfdst);
	}
	if (ecp.fader) {
		sdl_fader_finish(ecp.fader);
		ecp.fader = NULL;
	}
	
	return OK;
}
