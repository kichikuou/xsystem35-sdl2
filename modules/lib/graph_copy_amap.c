/*
 * graph_copy_amap.c  alpha map のコピー
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
/* $Id: graph_copy_amap.c,v 1.2 2003/04/25 17:23:55 chikama Exp $ */

#include <string.h>

#include "portab.h"
#include "system.h"
#include "surface.h"
#include "ngraph.h"
#include "ags.h"

/**
 * 指定の surface の alphamap を別の surface の指定領域にコピーする
 *
 * @param dst: 転送先 surface
 * @param dx:  転送先Ｘ座標
 * @param dy:  転送先Ｙ座標
 * @param src: 転送元surface
 * @param sx:  転送元Ｘ座標
 * @param sy:  転送元Ｙ座標
 * @param sw:  転送幅
 * @param sh:  転送高さ
 */
int gr_copy_alpha_map(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int sw, int sh) {
	BYTE *sp, *dp;
	
	if (!gr_clip(src, &sx, &sy, &sw, &sh, dst, &dx, &dy)) return NG;
	
	sp = GETOFFSET_ALPHA(src, sx, sy);
	dp = GETOFFSET_ALPHA(dst, dx, dy);

	if (sp == NULL) {
		WARNING("src alpha NULL\n");
		return NG;
	}
	
	if (dp == NULL) {
		WARNING("dst alpha NULL\n");
		return NG;
	}
	
	if (src == dst) {
		if (sy <= dy && dy < (sy + sh)) {
			sp += (sh -1) * src->width;
			dp += (sh -1) * dst->width;
			while(sh--) {
				memmove(dp, sp, sw);
				sp -= src->width;
				dp -= dst->width;
			}
		} else {
			while(sh--) {
				memmove(dp, sp, sw);
				sp += src->width;
				dp += dst->width;
			}
		}
	} else {
		while(sh--) {
			memcpy(dp, sp, sw);
			sp += src->width;
			dp += dst->width;
		}
	}
	
	return OK;
}
