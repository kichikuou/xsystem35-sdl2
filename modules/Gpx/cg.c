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
#include <stdlib.h>

#include "portab.h"
#include "system.h"
#include "surface.h"
#include "cg.h"
#include "ags.h"
#include "pms.h"
#include "qnt.h"
#include "gfx.h"
#include "ngraph.h"
#include "dri.h"
#include "ald_manager.h"

/**
 * バッファのデータがどのCG形式かを最初の数バイトをみてチェック
 *
 * @param data: データ列
 * @return CGの種類(QNT/PMS8/PMS16のいずれか)
 */
static CG_TYPE check_cgformat(uint8_t *data) {
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
static surface_t *sf_getcg(void *b, size_t size) {
	surface_t *sf;
	cgdata *cg;

	switch(check_cgformat(b)) {
	case ALCG_PMS8:
		// Load as an alpha map
		cg = pms256_extract(b);
		cg->alpha = cg->pic;
		cg->pic = NULL;
		sf = sf_create_alpha(cg->width, cg->height);
		gfx_drawImageAlphaMap(cg, sf, 0, 0);
		break;
	case ALCG_PMS16:
		cg = pms64k_extract(b);
		sf = cg->alpha
			? sf_create_surface(cg->width, cg->height)
			: sf_create_pixel(cg->width, cg->height);
		gfx_drawImage16(cg, sf, 0, 0, 255, false);
		break;
	case ALCG_QNT:
		cg = qnt_extract(b);
		sf = cg->alpha
			? sf_create_surface(cg->width, cg->height)
			: sf_create_pixel(cg->width, cg->height);
		gfx_drawImage24(cg, sf, 0, 0, 255);
		break;
	default:
		WARNING("Unknown Cg Type");
		return NULL;
	}

	cgdata_free(cg);
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
	
	sf = sf_getcg(dfile->data, dfile->size);
	
	ald_freedata(dfile);

	return sf;
}
