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

#include "portab.h"
#include "system.h"
#include "ags.h"
#include "graphics.h"
#include "surface.h"
#include "graph.h"
#include "ngraph.h"
#include "sprite.h"

/*
 指定の sprite (の現在のCG)を surface0 に書く
 @param sp: 描画するスプライト
 @param r : 再描画する領域
*/
void nt_sp_draw(sprite_t *sp, MyRectangle *r) {
	if (sp == NULL) return;
	cginfo_t *cg = sp->curcg;
	if (cg == NULL) return;
	if (cg->sf == NULL) return;

	// 更新領域の確定
	surface_t update;
	update.width  = r->w;
	update.height = r->h;
	int sx = 0;
	int sy = 0;
	int dx = sp->cur.x - r->x;
	int dy = sp->cur.y - r->y;
	int w = cg->sf->width;
	int h = cg->sf->height;
	
	if (!gr_clip(cg->sf, &sx, &sy, &w, &h, &update, &dx, &dy)) {
		return;
	}
		
	dx += r->x;
	dy += r->y;
	
	if (cg->sf->alpha) {
		// alpha map がある場合
		gre_BlendUseAMap(sf0, dx, dy,
				 sf0, dx, dy,
				 cg->sf, sx, sy, w, h,
				 cg->sf, sx, sy,
				 sp->blendrate);
	} else {
		if (sp->blendrate == 255) {
			// alpha値指定が無い場合
			gr_copy(sf0, dx, dy, cg->sf, sx, sy, w, h);
		} else if (sp->blendrate > 0) {
			// alpha値指定がある場合
			gr_blend(sf0, dx, dy, cg->sf, sx, sy, w, h, sp->blendrate);
		}
	}
	
	SACT_DEBUG("do update no=%d, sx=%d, sy=%d, w=%d, h=%d, dx=%d, dy=%d",
		sp->no, sx, sy, w, h, dx, dy);
}

// BlendScreenによる描画
void nt_sp_draw_scg(sprite_t *sp, MyRectangle *r) {
	surface_t update;
	cginfo_t *cg;
	int sx, sy, w, h, dx, dy;
	
	if (sp == NULL) return;
	
	cg = sp->curcg;
	
	if (cg == NULL) return;
	if (cg->sf == NULL) return;
	
	// 更新領域の確定
	update.width  = r->w;
	update.height = r->h;
	sx = 0;
	sy = 0;
	dx = sp->cur.x - r->x;
	dy = sp->cur.y - r->y;
	w = cg->sf->width;
	h = cg->sf->height;
	
	if (!gr_clip(cg->sf, &sx, &sy, &w, &h, &update, &dx, &dy)) {
		return;
	}
		
	dx += r->x;
	dy += r->y;
	
	gr_blend_screen(sf0, dx, dy, cg->sf, sx, sy, w, h);
	
	SACT_DEBUG("do update no=%d, sx=%d, sy=%d, w=%d, h=%d, dx=%d, dy=%d",
		sp->no, sx, sy, w, h, dx, dy);
}
