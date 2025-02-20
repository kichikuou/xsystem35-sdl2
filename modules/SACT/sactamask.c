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
#include <SDL.h>

#include "portab.h"
#include "system.h"
#include "LittleEndian.h"
#include "ags.h"
#include "sact.h"
#include "pms.h"
#include "sprite.h"
#include "sdl_core.h"
#include "mmap.h"

// SACTEFAM を使ったマスク
typedef struct {
	mmap_t *mmap;
	int datanum;  // SACTEFAM.KLD 中のマスクファイルの数
	int *no;      // シナリオ側での番号
	int *offset;  // データへのオフセット
} SACTEFAM_t;

static SACTEFAM_t am;

// SACTEFAM.KLD の読み込み
bool smask_init(char *path) {
	if (am.mmap)
		return true;  // already loaded

	mmap_t *m = map_file(path);
	if (!m)
		return false;
	am.mmap = m;
	am.datanum = LittleEndian_getDW(m->addr, 0);
	am.no = malloc(sizeof(int) * am.datanum);
	am.offset = malloc(sizeof(int) * am.datanum);
	
	for (int i = 0; i < am.datanum; i++) {
		am.no[i] = LittleEndian_getDW(m->addr, 16 + i * 16);
		am.offset[i] = LittleEndian_getDW(m->addr, 16 + i * 16 + 8);
	}
	
	return true;
}

// 指定番号の alphamask ファイルをよみだす
static cgdata *smask_get(int no) {
	int i;
	
	for (i = 0; i < am.datanum; i++) {
		if (am.no[i] == no) break;
	}

	if (i == am.datanum) return NULL;
	
	return pms256_extract(am.mmap->addr + am.offset[i]);
}

/**
 * マスクつき画面更新
 */
void sp_eupdate_amap(int index, int time, int cancel) {
	cgdata *mask = smask_get(index);
	if (mask == NULL) {
		sp_update_all(true);
		return;
	}
	SDL_Surface *mask_sf = SDL_CreateRGBSurfaceFrom(mask->pic, mask->width, mask->height, 8, mask->width, 0, 0, 0, 0);
	sp_update_all(false);  // old = sdl_texture, new = main_surface
	struct sdl_effect *eff = sdl_effect_sactamask_init(mask_sf);
	ags_runEffect(time * 10, cancel, (ags_EffectStepFunc)sdl_effect_step, eff);
	sdl_effect_finish(eff);
	ags_updateFull();

	SDL_FreeSurface(mask_sf);
	cgdata_free(mask);
}
