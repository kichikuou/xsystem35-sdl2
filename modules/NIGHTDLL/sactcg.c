/*
 * sactcg.c: CG作成
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
/* $Id: sactcg.c,v 1.2 2003/11/16 15:29:52 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "portab.h"
#include "nact.h"
#include "system.h"
#include "ags.h"
#include "surface.h"
#include "ngraph.h"
#include "sactcg.h"

#define CGMAX 65536
static cginfo_t *cgs[CGMAX];

#include "sactcg_stretch.c"
#include "sactcg_blend.c"


#define SPCG_ASSERT_NO(no) \
  if ((no) > (CGMAX -1)) { \
    WARNING("no is too large (should be %d < %d)", (no), CGMAX); \
    return; \
  } \

static cginfo_t *nt_scg_new(enum cgtype type, int no, surface_t *sf) {
	cginfo_t *info = calloc(1, sizeof(cginfo_t));
	info->type = type;
	info->no = no;
	info->sf = sf;
	info->refcnt = 1;

	nt_scg_free(no);
	cgs[no] = info;

	return info;
}

/*
  cgの読み込み

    指定の番号のCGをリンクファイルから読み込んだり、
    CG_xxxで作成したCGを参照する
    
  @param no: 読み込むCG番号
*/
static cginfo_t *nt_scg_get(int no) {
	if (no >= (CGMAX -1)) {
		WARNING("no is too large (should be %d < %d)", (no), CGMAX);
		return NULL;
	}
	
	if (cgs[no] != NULL)
		return cgs[no];

	surface_t *sf = sf_loadcg_no(no - 1);
	if (!sf) {
		WARNING("load fail (%d)", no -1);
		return NULL;
	}
	return nt_scg_new(CG_LINKED, no, sf);
}

cginfo_t *nt_scg_addref(int no) {
	cginfo_t *info = nt_scg_get(no);
	if (info)
		info->refcnt++;
	return info;
}

void nt_scg_deref(cginfo_t *cg) {
	if (--cg->refcnt > 0)
		return;

	if (cg->sf)
		sf_free(cg->sf);
	free(cg);
}

//  指定の大きさ、色の矩形の CG を作成
void nt_scg_create(int wNumCG, int wWidth, int wHeight, int wR, int wG, int wB, int wBlendRate) {
	SPCG_ASSERT_NO(wNumCG);

	surface_t *sf = sf_create_surface(wWidth, wHeight, sf0->depth);
	gr_fill(sf, 0, 0, wWidth, wHeight, wR, wG, wB);
	gr_fill_alpha_map(sf, 0, 0, wWidth, wHeight, wBlendRate);
	nt_scg_new(CG_SET, wNumCG, sf);
}

// CGの一部を切りぬいたCGを作成
void nt_scg_cut(int wNumDstCG, int wNumSrcCG, int wX, int wY, int wWidth, int wHeight) {
	SPCG_ASSERT_NO(wNumDstCG);
	SPCG_ASSERT_NO(wNumSrcCG);

	cginfo_t *src = nt_scg_get(wNumSrcCG);
	if (!src)
		return;

	surface_t *sf = src->sf->alpha
		? sf_create_surface(wWidth, wHeight, src->sf->depth)
		: sf_create_pixel(wWidth, wHeight, src->sf->depth);

	if (src->sf->pixel)
		gr_copy(sf, 0, 0, src->sf, wX, wY, wWidth, wHeight);
	if (src->sf->alpha)
		gr_copy_alpha_map(sf, 0, 0, src->sf, wX, wY, wWidth, wHeight);

	nt_scg_new(CG_SET, wNumDstCG, sf);
}

// 全てのCGの開放
void nt_scg_freeall(void) {
	int i;
	
	for (i = 1; i < CGMAX; i++) {
		nt_scg_free(i);
	}
}

/**
 * 指定の番号の CG をオブジェクトリストから消し、オブジェクトがどこからも参照
 * されていない(参照数が0の)場合のみ、オブジェクトを削除
 */
void nt_scg_free(int no) {
	SPCG_ASSERT_NO(no);
	
	cginfo_t *cg = cgs[no];
	if (!cg) return;
	
	nt_scg_deref(cg);
	cgs[no] = NULL;
}
