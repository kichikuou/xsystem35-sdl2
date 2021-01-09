/*
 * cmde.c  SYSTEM35 E command
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
/* $Id: cmde.c,v 1.11 2003/01/17 23:23:11 chikama Exp $ */

#include <stdio.h>
#include "portab.h"
#include "xsystem35.h"
#include "ags.h"
#include "graphics.h"

typedef struct {
	MyRectangle r;
	int pal;
} Ecomtbl;

static Ecomtbl tbl[100];

void commandES() {
	int num    = getCaliValue();
	int pal    = getCaliValue();
	int x      = getCaliValue();
	int y      = getCaliValue();
	int width  = getCaliValue();
	int height = getCaliValue();
	
	tbl[num - 1].r.x = x;
	tbl[num - 1].r.y = y;
	tbl[num - 1].r.w = width;
	tbl[num - 1].r.h = height;
	tbl[num - 1].pal = pal;
	
	DEBUG_COMMAND("ES %d,%d,%d,%d,%d,%d:\n", num, pal, x, y, width, height);
}

void commandEC() {
	int num = getCaliValue();
	Ecomtbl *e;
	MyRectangle *r;
	
	if (num == 0) {
		r = &nact->sys_view_area;
		ags_fillRectangle(r->x, r->y, r->w, r->h, 0);
		ags_updateArea(r->x, r->y, r->w, r->h);
	} else {
		int adj = (nact->patch_ec == 0) ? -1 : 0;
		e = &tbl[num - 1];
		ags_fillRectangle(e->r.x, e->r.y, e->r.w + adj, e->r.h + adj, e->pal);
		ags_updateArea(e->r.x, e->r.y, e->r.w + adj, e->r.h + adj);
	}
	DEBUG_COMMAND("EC %d:\n", num);
}

void commandEG() {
	int num     = getCaliValue();
	int *x      = getCaliVariable();
	int *y      = getCaliVariable();
	int *width  = getCaliVariable();
	int *height = getCaliVariable();
	
	*x      = tbl[num -1].r.x;
	*y      = tbl[num -1].r.y;
	*width  = tbl[num -1].r.w;
	*height = tbl[num -1].r.h;

	DEBUG_COMMAND("EG %d,%d,%d,%d,%d:\n", num, *x, *y, *width, *height);
}

void commandEM() {
	int num  = getCaliValue();
	int *var = getCaliVariable();
	int x    = getCaliValue();
	int y    = getCaliValue();

	MyPoint p = {x, y};
	MyRectangle r = tbl[num - 1].r;
	if (nact->patch_emen == 0) {
		r.w -= 1;
		r.h -= 1;
	}
	*var = SDL_PointInRect(&p, &r) ? 1 : 0;
	DEBUG_COMMAND("EM %d,%d,%d,%d:\n", num, *var, x, y);
}

void commandEN() {
	int *var = getCaliVariable();
	int min  = getCaliValue();
	int max  = getCaliValue();
	int x    = getCaliValue();
	int y    = getCaliValue();
	
	MyPoint p = {x, y};
	*var = 0;
	
	for (int i = min; i <= max; i++) {
		MyRectangle r = tbl[i - 1].r;
		if (nact->patch_emen == 0) {
			r.w -= 1;
			r.h -= 1;
		}
		if (SDL_PointInRect(&p, &r))
			*var = i;
	}
	
	DEBUG_COMMAND("EN %d,%d,%d,%d,%d:\n", *var, min, max, x, y);
}
