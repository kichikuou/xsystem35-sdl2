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
/* $Id: sprite_draw.c,v 1.1 2003/11/09 15:06:12 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <SDL.h>

#include "portab.h"
#include "system.h"
#include "ags.h"
#include "sprite.h"

/*
 指定の sprite (の現在のCG)を surface0 に書く
 @param sp: 描画するスプライト
 @param r : 再描画する領域
*/
void nt_sp_draw(sprite_t *sp, SDL_Rect *r) {
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
	
	if (!ags_clipCopyRect(&cg_rect, r, &sx, &sy, &dx, &dy, &w, &h)) {
		return;
	}

	if (SDL_ISPIXELFORMAT_ALPHA(cg->sf->format->format) || sp->blendrate < 255) {
		SDL_SetSurfaceBlendMode(cg->sf, SDL_BLENDMODE_BLEND);
		SDL_SetSurfaceAlphaMod(cg->sf, sp->blendrate);
	} else {
		SDL_SetSurfaceBlendMode(cg->sf, SDL_BLENDMODE_NONE);
	}
	SDL_BlitSurface(cg->sf, &(SDL_Rect){sx, sy, w, h}, main_surface, &(SDL_Rect){dx, dy, w, h});
	
	SACT_DEBUG("do update no=%d, sx=%d, sy=%d, w=%d, h=%d, dx=%d, dy=%d",
		sp->no, sx, sy, w, h, dx, dy);
}

// BlendScreenによる描画
void nt_sp_draw_scg(sprite_t *sp, SDL_Rect *r) {
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
	
	if (!ags_clipCopyRect(&cg_rect, r, &sx, &sy, &dx, &dy, &w, &h)) {
		return;
	}
	
	SDL_SetSurfaceBlendMode(cg->sf, SDL_BLENDMODE_ADD);
	SDL_BlitSurface(cg->sf, &(SDL_Rect){sx, sy, w, h}, main_surface, &(SDL_Rect){dx, dy, w, h});
	
	SACT_DEBUG("do update no=%d, sx=%d, sy=%d, w=%d, h=%d, dx=%d, dy=%d",
		sp->no, sx, sy, w, h, dx, dy);
}
