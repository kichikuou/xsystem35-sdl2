/*
 * sprite_draw.c: スプライト再描画各種
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
/* $Id: sprite_draw.c,v 1.4 2004/10/31 04:18:02 chikama Exp $ */

#include "config.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <SDL.h>

#include "portab.h"
#include "system.h"
#include "ags.h"
#include "graphics.h"
#include "sact.h"
#include "sprite.h"

// alphamapにしたがって、alpha値が0より大きいところを指定のdepthとする
static void fill_dmap_mask(SDL_Surface *src, int sx, int sy, int dx ,int dy, int w, int h, uint16_t val) {
	uint8_t *sp, *dp;
	int x, y;
	assert(src->format->Amask == 0xff000000);
	dp = PIXEL_AT(sact.dmap, dx, dy);
	sp = ALPHA_AT(src, sx, sy);
	
	for (y = 0; y < h; y++) {
		uint8_t *yls = sp + y * src->pitch;
		uint16_t *yld = (uint16_t *)(dp + y * sact.dmap->pitch);
		for (x = 0; x < w; x++) {
			if (*yls > 0) *yld = val;
			yls += 4; yld++;
		}
	}
}

/*
 指定の sprite (の現在のCG)を surface0 に書く
 @param sp: 描画するスプライト
*/
void sp_draw(sprite_t *sp) {
	if (sp == NULL) return;
	cginfo_t *cg = sp->curcg;
	if (cg == NULL) return;
	if (cg->sf == NULL) return;

	// 更新領域の確定
	SDL_Rect cg_rect = {0, 0, cg->sf->w, cg->sf->h};
	int sx = 0;
	int sy = 0;
	int dx = sp->cur.x;
	int dy = sp->cur.y;
	int w = cg->sf->w;
	int h = cg->sf->h;

	if (!ags_clipCopyRect(&cg_rect, &sact.updaterect, &sx, &sy, &dx, &dy, &w, &h)) {
		return;
	}

	if (SDL_ISPIXELFORMAT_ALPHA(cg->sf->format->format) || sp->blendrate < 255) {
		SDL_SetSurfaceBlendMode(cg->sf, SDL_BLENDMODE_BLEND);
		SDL_SetSurfaceAlphaMod(cg->sf, sp->blendrate);
	} else {
		SDL_SetSurfaceBlendMode(cg->sf, SDL_BLENDMODE_NONE);
	}
	SDL_BlitSurface(cg->sf, &(SDL_Rect){sx, sy, w, h}, main_surface, &(SDL_Rect){dx, dy, w, h});
	
	SACT_DEBUG("do update no=%d, sx=%d, sy=%d, w=%d, h=%d, dx=%d, dy=%d", sp->no, sx, sy, w, h, dx, dy);
}

/*
  スプライトキー待ち用のdepthmap を更新
*/
void sp_draw_dmap(void* data, void* userdata) {
	sprite_t *sp = (sprite_t *)data;

	// 非表示状態の時は無視
	if (!sp->show) return;
	
	// ドラッグ中のスプライトは無視
	if (sp == sact.draggedsp) return;
	
	cginfo_t *cg = sp->curcg;
	if (cg == NULL) return;
	if (cg->sf == NULL) return;
	
	// depth map を書く領域を確定
	SDL_Rect cg_rect = {0, 0, cg->sf->w, cg->sf->h};
	SDL_Rect screen_rect = {0, 0, main_surface->w, main_surface->h};
	int sx = 0;
	int sy = 0;
	int dx = sp->cur.x;
	int dy = sp->cur.y;
	int w = cg->sf->w;
	int h = cg->sf->h;

	if (!ags_clipCopyRect(&cg_rect, &screen_rect, &sx, &sy, &dx, &dy, &w, &h)) {
		return;
	}
	
	if (SDL_ISPIXELFORMAT_ALPHA(cg->sf->format->format)) {
		fill_dmap_mask(cg->sf, sx, sy, dx, dy, w, h, sp->no);
	} else {
		SDL_FillRect(sact.dmap, &(SDL_Rect){dx, dy, w, h}, sp->no);
	}
	
	return;
}
