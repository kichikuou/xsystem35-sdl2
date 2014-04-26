/*
 * cg.c  DLL用に CGを surface に展開
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
/* $Id: cg.c,v 1.2 2003/04/22 16:29:52 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <glib.h>

#include "portab.h"
#include "system.h"
#include "surface.h"
#include "cg.h"
#include "ags.h"
#include "pms.h"
#include "qnt.h"
#include "ngraph.h"
#include "dri.h"
#include "ald_manager.h"

/**
 * バッファのデータがどのCG形式かを最初の数バイトをみてチェック
 *
 * @param data: データ列
 * @return CGの種類(QNT/PMS8/PMS16のいずれか)
 */
static CG_TYPE check_cgformat(BYTE *data) {
	if (qnt_checkfmt(data)) {
		return ALCG_QNT;
	} else if (pms256_checkfmt(data)) {
		return ALCG_PMS8;
	} else if (pms64k_checkfmt(data)) {
		return ALCG_PMS16;
	}
	return ALCG_UNKNOWN;
}

/**
 * ファイル等から読み込んだCGデータをsurfaceに展開
 *
 * @param b: データ列
 * @return CG が展開された surface
 *         未知の形式のときは NULL が返る
 */
surface_t *sf_getcg(void *b) {
	surface_t *sf = NULL;
	int type;
	cgdata *cg = NULL;
	
	type = check_cgformat(b);
	switch(type) {
	case ALCG_PMS8:
		cg = pms256_extract(b);
		break;
	case ALCG_PMS16:
		cg = pms64k_extract(b);
		break;
	case ALCG_QNT:
		cg = qnt_extract(b);
		break;
	default:
		break;
	}
	
	if (cg == NULL) {
		WARNING("Unknown Cg Type\n");
		return NULL;
	}
	
	switch(type) {
	case ALCG_PMS8:
		sf = sf_create_alpha(cg->width, cg->height);
		gr_draw_amap(sf, cg->x, cg->y, cg->pic, cg->width, cg->height, cg->width);
		break;
		
	case ALCG_PMS16:
                if (cg->alpha) {
			sf = sf_create_surface(cg->width, cg->height, sf0->depth);
			gr_drawimage16(sf, cg, cg->x, cg->y);
			gr_draw_amap(sf, cg->x, cg->y, cg->alpha, cg->width, cg->height, cg->width);
		} else {
			sf = sf_create_pixel(cg->width, cg->height, sf0->depth);
			gr_drawimage16(sf, cg, cg->x, cg->y);
		}
		break;
                
        case ALCG_QNT:
		if (cg->alpha) {
			sf = sf_create_surface(cg->width, cg->height, sf0->depth);
			gr_drawimage24(sf, cg, cg->x, cg->y);
			gr_draw_amap(sf, cg->x, cg->y, cg->alpha, cg->width, cg->height, cg->width);
		} else {
			sf = sf_create_pixel(cg->width, cg->height, sf0->depth);
			gr_drawimage24(sf, cg, cg->x, cg->y);
		}
                break;
	}
	
	if (cg->pic)   g_free(cg->pic);
	if (cg->pal)   g_free(cg->pal);
	if (cg->alpha) g_free(cg->alpha);
	g_free(cg);
	
	return sf;
}

/**
 * ALDファイルから指定の番号のCGを読み込んで surfaceに展開
 * 
 * @param no: ファイル番号 (0-)
 * @return CGが展開された surface
 */
surface_t *sf_loadcg_no(int no) {
	dridata *dfile;
	surface_t *sf;
	
	if (NULL == (dfile = ald_getdata(DRIFILE_CG, no))) {
		return NULL;
	}
	
	sf = sf_getcg(dfile->data);
	
	ald_freedata(dfile);

	return sf;
}
