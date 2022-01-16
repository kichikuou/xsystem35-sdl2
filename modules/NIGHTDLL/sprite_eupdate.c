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
/* $Id: sprite_eupdate.c,v 1.1 2003/11/09 15:06:12 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "portab.h"
#include "system.h"
#include "ngraph.h"
#include "ags.h"
#include "nact.h"
#include "input.h"
#include "sprite.h"
#include "surface.h"
#include "sactcg.h"
#include "sdl_core.h"


static void ec6_cb(surface_t *, surface_t *);
static void ec11_cb(surface_t *, surface_t *);
static void ec12_cb(surface_t *, surface_t *);
static void ec13_cb(surface_t *, surface_t *);
static void ec_dummy_cb(surface_t *, surface_t *);

struct ecopyparam {
	int sttime;
	int curtime;
	int edtime;
	int curstep;
	int oldstep;
};
typedef struct ecopyparam ecopyparam_t;
static ecopyparam_t ecp;

// #include "sprite_eupdate_perspect.c"
// #include "sprite_eupdate_mosaic.c"
// #include "sprite_eupdate_uneune.c"
#include "sprite_eupdate_buller.c"
// #include "sprite_eupdate_zmbbul.c"

typedef void entrypoint (surface_t *, surface_t *);

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


/*
  効果つき画面更新
  @param type: 効果の種類
  @param time: 実行時間(msec)
  @param cancel: キー抜け(0:なし, 1:あり)
*/
int nt_sp_eupdate(int type, int time, int cancel) {
	surface_t *sfsrc, *sfdst;
	int key;
	entrypoint *cb;

	// 現在の sf0 をセーブ
	sfsrc = sf_dup(sf0);
	
	nt_sp_update_all(FALSE);
	
	sfdst = sf_dup(sf0);
	
	enum sdl_effect sdl_effect = from_sact_effect(type - 100);
	struct sdl_fader *fader = NULL;
	if (sdl_effect != EFFECT_INVALID) {
		SDL_Rect rect = { 0, 0, sfsrc->width, sfsrc->height };
		fader = sdl_fader_init(&rect, NULL, 0, 0, sdl_getDIB(), 0, 0, sdl_effect);
	} else {
		sf_copyall(sf0, sfsrc); // 全部の効果タイプにこの処理は要らないんだけど
	}

	ecp.sttime = ecp.curtime = sdl_getTicks();
	ecp.edtime = ecp.curtime + time;
	ecp.oldstep = 0;
	
	switch(type) {
	case 10:
		cb = ec_dummy_cb;
		break;
	case 111: // 線形ぼかし
		ec11_prepare(sfsrc, sfdst);
		cb = ec11_cb;
		break;
	case 1011:
		cb = ec_dummy_cb;
		break;
	case 1013:
		nt_sp_update_all(TRUE); return OK;
		// cb = ec_dummy_cb;
		break;
	default:
		cb = ec_dummy_cb;
		break;
	}
	
	while ((ecp.curtime = sdl_getTicks()) < ecp.edtime) {
		if (fader) {
			sdl_fader_step(fader, (double)(ecp.curtime - ecp.sttime) / (ecp.edtime - ecp.sttime));
		} else {
			cb(sfsrc, sfdst);
		}
		int rest = 15 - (sdl_getTicks() - ecp.curtime);
		key = sys_keywait(rest, cancel ? KEYWAIT_CANCELABLE : KEYWAIT_NONCANCELABLE);
		
		if (cancel && key) break;
	}
	
	sf_copyall(sf0, sfdst);
	ags_updateFull();
	sf_free(sfsrc);
	sf_free(sfdst);

	if (type == 111) {
		ec11_drain(sfsrc, sfdst);
	}
	if (fader)
		sdl_fader_finish(fader);
	
	return OK;
}
