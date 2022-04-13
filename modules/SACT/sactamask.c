/*
 * sactamask.c: SACTEFAM.KLD 展開
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
/* $Id: sactamask.c,v 1.1 2003/04/22 16:29:52 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "portab.h"
#include "system.h"
#include "LittleEndian.h"
#include "input.h"
#include "sact.h"
#include "surface.h"
#include "ngraph.h"
#include "sprite.h"
#include "sdl_core.h"

static surface_t *smask_get(int no);
static surface_t *smask_mul(surface_t *sf, int val);



struct ecopyparam {
	int sttime;
	int curtime;
	int edtime;
	int curstep;
	int oldstep;
};
typedef struct ecopyparam ecopyparam_t;
static ecopyparam_t ecp;

// SACTEFAM.KLD の読み込み
int smask_init(char *path) {
	int i;
	SACTEFAM_t *am;

	mmap_t *m = map_file(path);
	if (!m)
		return NG;
	am = &sact.am;
	am->mmap = m;
	am->datanum = LittleEndian_getDW(m->addr, 0);
	am->no = malloc(sizeof(int) * am->datanum);
	am->offset = malloc(sizeof(int) * am->datanum);
	
	for (i = 0; i < am->datanum; i++) {
		am->no[i] = LittleEndian_getDW(m->addr, 16 + i * 16);
		am->offset[i] = LittleEndian_getDW(m->addr, 16 + i * 16 + 8);
	}
	
	return OK;
}

// 指定番号の alphamask ファイルをよみだす
static surface_t *smask_get(int no) {
	int i;
	SACTEFAM_t *am = &sact.am;
	
	for (i = 0; i < am->datanum; i++) {
		if (am->no[i] == no) break;
	}

	if (i == am->datanum) return NULL;
	
	return sf_getcg(am->mmap->addr + am->offset[i], 0 /*FIXME*/);
}

// ベースになるマスクの alpha 値を拡大して取り出す
static surface_t *smask_mul(surface_t *sf, int val) {
	surface_t *out = sf_create_alpha(sf->width, sf->height);
	BYTE *src = sf->alpha;
	BYTE *dst = out->alpha;
	int pix = sf->width * sf->height;

	while(pix--) {
		int i = (*src - val) * 16;
		if (i < 0)        *dst = 255; // 指定値よりも大きいのはコピー
		else if (i > 255) *dst = 0;   // 指定値よりも小さいのは無視
		else              *dst = 255-i; // それ以外は値を16倍
		src++; dst++;
	}
	
	return out;
}

/**
 * マスクつき画面更新
 */
int sp_eupdate_amap(int index, int time, int cancel) {
	surface_t *mask, *mask2;
	surface_t *sfsrc, *sfdst;
	int key;
	
	mask = smask_get(index);
	if (mask == NULL) {
		sp_update_all(TRUE);
		return OK;
	}
	
	// 現在の sf0 をセーブ
	sfsrc = sf_dup(sf0);
	sp_update_all(FALSE);
	sfdst = sf_dup(sf0);
	sf_copyall(sf0, sfsrc);
	
	ecp.sttime = ecp.curtime = sdl_getTicks();
	ecp.edtime = ecp.curtime + time*10;
	ecp.oldstep = 0;
	
	while ((ecp.curtime = sdl_getTicks()) < ecp.edtime) {
		int curstep = 255 * (ecp.curtime - ecp.sttime)/ (ecp.edtime - ecp.sttime);
		// 元になるマスクのalpha値を16倍して欲しいところだけ取り出す
		mask2 = smask_mul(mask, curstep);
		
		gre_BlendUseAMap(sf0, 0, 0, sfsrc, 0, 0, sfdst, 0, 0, sfsrc->width, sfsrc->height, mask2, 0, 0, 255);
		ags_updateFull();
		
		key = sys_keywait(10, cancel ? KEYWAIT_CANCELABLE : KEYWAIT_NONCANCELABLE);
		if (cancel && key) break;
		
		// 一時マスクを削除
		sf_free(mask2);
	}
	
	sf_copyall(sf0, sfdst);
	ags_updateFull();
	sf_free(sfsrc);
	sf_free(sfdst);
	
	sf_free(mask);
	return OK;
}
