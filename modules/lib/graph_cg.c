/*
 * graph_cg.c  surface に 実際にcgデータを描く
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
/* $Id: graph_cg.c,v 1.2 2003/08/30 21:29:16 chikama Exp $ */

#include "config.h"

#include <stdlib.h>
#include <string.h>

#include "portab.h"
#include "surface.h"
#include "cg.h"
#include "ags.h"
#include "ngraph.h"

/**
 * surface に 24bpp CG (QNT)を描画
 * 
 * @param ds: 描画 surface
 * @param cg: 描画するCGデータ
 * @param x: 描画Ｘ座標
 * @param y: 描画Ｙ座標
 */
void gr_drawimage24(surface_t *ds, cgdata *cg, int x, int y) {
	int dx, dy, dw, dh;
	BYTE *sp, *dp, r, g, b;
	
	dx = x;
	dy = y;
	dw = cg->width;
	dh = cg->height;

	if (!gr_clip_xywh(ds, &dx, &dy, &dw, &dh)) return;
	
	cg->data_offset = (abs(dy - y) * cg->width + abs(dx - x)) * 3;
	
	sp = (BYTE *)(cg->pic + cg->data_offset);
	dp = GETOFFSET_PIXEL(ds, dx, dy);
	
	switch(ds->depth) {
	case 15:
	{
		WORD *yl;
		
		for (y = 0; y < dh; y++) {
			yl = (WORD *)(dp + y * ds->bytes_per_line);
			for (x = 0; x < dw; x++) {
				r = *sp;
				g = *(sp +1);
				b = *(sp +2);
				*yl = PIX15(r, g, b);
				yl++; sp += 3;
			}
			sp += ((cg->width - dw) * 3);
		}
		break;
	}
	case 16:
	{
		WORD *yl;
		
		for (y = 0; y < dh; y++) {
			yl = (WORD *)(dp + y * ds->bytes_per_line);
			for (x = 0; x < dw; x++) {
				r = *sp;
				g = *(sp +1);
				b = *(sp +2);
				*yl = PIX16(r, g, b);
				yl++; sp += 3;
			}
			sp += ((cg->width - dw) * 3);
		}
		break;
		
	}
	case 24:
	case 32:
	{
		DWORD *yl;
		
		for (y = 0; y < dh; y++) {
			yl = (DWORD *)(dp + y * ds->bytes_per_line);
			for (x = 0; x < dw; x++) {
				r = *sp;
				g = *(sp +1);
				b = *(sp +2);
				*yl = PIX24(r, g, b);
				yl++; sp += 3;
			}
			sp += ((cg->width - dw) * 3);
		}
		break;
	}}
}

/**
 * surface に 16bpp CG (PMS16/BMP)を描画
 * 
 * @param ds: 描画 surface
 * @param cg: 描画するCGデータ
 * @param x: 描画Ｘ座標
 * @param y: 描画Ｙ座標
 */
void gr_drawimage16(surface_t *ds, cgdata *cg, int x, int y) {
	int dx, dy, dw, dh;
	BYTE *dp;
	WORD pic16, *sp;
	
	dx = x;
	dy = y;
	dw = cg->width;
	dh = cg->height;
	
	if (!gr_clip_xywh(ds, &dx, &dy, &dw, &dh)) return;
	
	cg->data_offset = (abs(dy - y) * cg->width + abs(dx - x)) * 2;
	
	sp = (WORD *)(cg->pic + cg->data_offset);
	dp = GETOFFSET_PIXEL(ds, dx, dy);
	
	switch(ds->depth) {
	case 15:
	{
		WORD *yl;
		
		for (y = 0; y < dh; y++) {
			yl = (WORD *)(dp + y * ds->bytes_per_line);
			for (x = 0; x < dw; x++) {
				pic16 = *sp;
				*yl = PIX15(RGB_PIXR16(pic16), RGB_PIXG16(pic16), RGB_PIXB16(pic16));
				yl++; sp ++;
			}
			sp += (cg->width - dw);
		}
		break;
	}
	case 16:
	{
		for (y = 0; y < dh; y++) {
			memcpy(dp, sp, dw * 2);
			sp += cg->width;
			dp += ds->bytes_per_line;
		}
		break;
		
	}
	case 24:
	case 32:
	{
		DWORD *yl;
		
		for (y = 0; y < dh; y++) {
			yl = (DWORD *)(dp + y * ds->bytes_per_line);
			for (x = 0; x < dw; x++) {
				pic16 = *sp;
				*yl = PIX24(RGB_PIXR16(pic16), RGB_PIXG16(pic16), RGB_PIXB16(pic16));
				yl++; sp++;
			}
			sp += (cg->width - dw);
		}
		break;
	}}
}
