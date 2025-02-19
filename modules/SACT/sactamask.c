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
#include "pms.h"
#include "ngraph.h"
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

// Scale the alpha value in `mask` and write it to the alpha channel of `out`
static void smask_update_alpha(SDL_Surface *out, cgdata *mask, int val) {
	uint8_t *src = mask->pic;

	for (int y = 0; y < mask->height; y++) {
		uint8_t *dst = (uint8_t*)out->pixels + y * out->pitch + (SDL_BYTEORDER == SDL_LIL_ENDIAN ? 3 : 0);
		for (int x = 0; x < mask->width; x++) {
			int i = 255 - (*src - val) * 16;
			*dst = min(255, max(0, i));
			src++; dst += 4;
		}
	}
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

	SDL_Surface *sf_old = SDL_ConvertSurface(main_surface, main_surface->format, 0);
	sp_update_all(false);
	SDL_Surface *sf_new = SDL_ConvertSurfaceFormat(main_surface, SDL_PIXELFORMAT_ARGB8888, 0);
	SDL_SetSurfaceBlendMode(sf_new, SDL_BLENDMODE_BLEND);
	SDL_BlitSurface(sf_old, NULL, main_surface, NULL);

	ecp.sttime = ecp.curtime = sdl_getTicks();
	ecp.edtime = ecp.curtime + time*10;
	ecp.oldstep = 0;

	while ((ecp.curtime = sdl_getTicks()) < ecp.edtime) {
		int curstep = 255 * (ecp.curtime - ecp.sttime)/ (ecp.edtime - ecp.sttime);
		smask_update_alpha(sf_new, mask, curstep);

		SDL_BlitSurface(sf_old, NULL, main_surface, NULL);
		SDL_BlitSurface(sf_new, NULL, main_surface, NULL);
		ags_updateFull();

		int key = sys_keywait(10, cancel ? KEYWAIT_CANCELABLE : KEYWAIT_NONCANCELABLE);
		if (cancel && key) break;
	}
	SDL_SetSurfaceBlendMode(sf_new, SDL_BLENDMODE_NONE);
	SDL_BlitSurface(sf_new, NULL, main_surface, NULL);
	ags_updateFull();
	SDL_FreeSurface(sf_new);
	SDL_FreeSurface(sf_old);

	cgdata_free(mask);
}
