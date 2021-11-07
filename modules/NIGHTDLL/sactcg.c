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


#define spcg_assert_no(no) \
  if ((no) > (CGMAX -1)) { \
    WARNING("no is too large (should be %d < %d)\n", (no), CGMAX); \
    return NG; \
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
		WARNING("no is too large (should be %d < %d)\n", (no), CGMAX);
		return NULL;
	}
	
	if (cgs[no] != NULL)
		return cgs[no];

	surface_t *sf = sf_loadcg_no(no - 1);
	if (!sf) {
		WARNING("load fail (%d)\n", no -1);
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
int nt_scg_create(int wNumCG, int wWidth, int wHeight, int wR, int wG, int wB, int wBlendRate) {
	spcg_assert_no(wNumCG);

	surface_t *sf = sf_create_surface(wWidth, wHeight, sf0->depth);
	gr_fill(sf, 0, 0, wWidth, wHeight, wR, wG, wB);
	gr_fill_alpha_map(sf, 0, 0, wWidth, wHeight, wBlendRate);
	nt_scg_new(CG_SET, wNumCG, sf);
	return OK;
}

// 指定のCGを反転させたCGを作成
int nt_scg_create_reverse(int wNumCG, int wNumSrcCG, int wReverseX, int wReverseY) {
	spcg_assert_no(wNumCG);
	spcg_assert_no(wNumSrcCG);
	
	cginfo_t *src = nt_scg_get(wNumSrcCG);
	if (!src)
		return NG;

	surface_t *sf = stretch(src->sf, src->sf->width, src->sf->height, (wReverseX << 1) | wReverseY);
	nt_scg_new(CG_REVERSE, wNumCG, sf);
	return OK;
}

// 指定のCGを拡大/縮小したCGを作成
int nt_scg_create_stretch(int wNumCG, int wWidth, int wHeight, int wNumSrcCG) {
	spcg_assert_no(wNumCG);
	spcg_assert_no(wNumSrcCG);

	cginfo_t *src = nt_scg_get(wNumSrcCG);
	if (!src)
		return NG;
	
	surface_t *sf = stretch(src->sf, wWidth, wHeight, 0);
	nt_scg_new(CG_STRETCH, wNumCG, sf);
	return OK;
}

// ベースCGの上にブレンドCGを重ねた CG を作成
int nt_scg_create_blend(int wNumDstCG, int wNumBaseCG, int wX, int wY, int wNumBlendCG, int wAlphaMapMode) {
	spcg_assert_no(wNumDstCG);
	spcg_assert_no(wNumBaseCG);
	spcg_assert_no(wNumBlendCG);

	cginfo_t *basecg  = nt_scg_get(wNumBaseCG);
	cginfo_t *blendcg = nt_scg_get(wNumBlendCG);
	if (!basecg || !blendcg) return NG;

	surface_t *sf = blend(basecg->sf, wX, wY, blendcg->sf, wAlphaMapMode);
	nt_scg_new(CG_SET, wNumDstCG, sf);
	return OK;
}

// 指定の文字列のCGを作成
int nt_scg_create_text(int wNumCG, int wSize, int wR, int wG, int wB, char *cText) {
	spcg_assert_no(wNumCG);
	
	// 勝手に出ていいのかな？
	if (strlen(cText) == 0) return OK;
	
	ags_setFont(FONT_GOTHIC, wSize);
	agsurface_t *glyph = ags_drawStringToSurface(cText);

	surface_t *sf = sf_create_surface(glyph->width, wSize, nact->ags.dib->depth);
	gr_fill(sf, 0, 0, glyph->width, wSize, wR, wG, wB);
	gr_draw_amap(sf, 0, 0, glyph->pixel, glyph->width, wSize, glyph->bytes_per_line);

	nt_scg_new(CG_SET, wNumCG, sf);
	return OK;
}

// 数字文字列のCGを作成
int nt_scg_create_textnum(int wNumCG, int wSize, int wR, int wG, int wB, int wFigs, int wZeroPadding, int wValue) {
	spcg_assert_no(wNumCG);

	char s[256], ss[256];
	if (wZeroPadding) {
		char *sss = "%%0%dd";
		sprintf(ss, sss, wFigs);
	} else {
		char *sss = "%%%dd";
		sprintf(ss, sss, wFigs);
	}
	sprintf(s, ss, wValue);
	
	ags_setFont(FONT_GOTHIC, wSize);
	agsurface_t *glyph = ags_drawStringToSurface(s);
	
	surface_t *sf = sf_create_surface(glyph->width, wSize, nact->ags.dib->depth);
	gr_fill(sf, 0, 0, glyph->width, wSize, wR, wG, wB);
	gr_draw_amap(sf, 0, 0, glyph->pixel, glyph->width, wSize, glyph->bytes_per_line);

	nt_scg_new(CG_SET, wNumCG, sf);
	return OK;
}

// CGを複製
int nt_scg_copy(int wNumDstCG, int wNumSrcCG) {
	spcg_assert_no(wNumDstCG);
	spcg_assert_no(wNumSrcCG);

	cginfo_t *src = nt_scg_get(wNumSrcCG);
	if (!src)
		return NG;

	nt_scg_new(CG_SET, wNumDstCG, sf_dup(src->sf));
	return OK;
}

// CGの一部を切りぬいたCGを作成
int nt_scg_cut(int wNumDstCG, int wNumSrcCG, int wX, int wY, int wWidth, int wHeight) {
	spcg_assert_no(wNumDstCG);
	spcg_assert_no(wNumSrcCG);

	cginfo_t *src = nt_scg_get(wNumSrcCG);
	if (!src)
		return NG;

	surface_t *sf = src->sf->alpha
		? sf_create_surface(wWidth, wHeight, src->sf->depth)
		: sf_create_pixel(wWidth, wHeight, src->sf->depth);

	if (src->sf->pixel)
		gr_copy(sf, 0, 0, src->sf, wX, wY, wWidth, wHeight);
	if (src->sf->alpha)
		gr_copy_alpha_map(sf, 0, 0, src->sf, wX, wY, wWidth, wHeight);

	nt_scg_new(CG_SET, wNumDstCG, sf);
	return OK;
}

// 元のCGの一部を切りぬいたCGを作成
int nt_scg_partcopy(int wNumDstCG, int wNumSrcCG, int wX, int wY, int wWidth, int wHeight) {
	spcg_assert_no(wNumDstCG);
	spcg_assert_no(wNumSrcCG);

	cginfo_t *src = nt_scg_get(wNumSrcCG);
	if (!src)
		return NG;

	surface_t *sf;
	if (src->sf->alpha) {
		sf = sf_create_surface(src->sf->width, src->sf->height, src->sf->depth);
		gr_fill_alpha_map(sf, 0, 0, src->sf->width, src->sf->height, 255);
	} else {
		sf = sf_create_pixel(src->sf->width, src->sf->height, src->sf->depth);
	}
	
	if (src->sf->pixel)
		gr_copy(sf, wX, wY, src->sf, wX, wY, wWidth, wHeight);
	if (src->sf->alpha)
		gr_copy_alpha_map(sf, wX, wY, src->sf, wX, wY, wWidth, wHeight);

	nt_scg_new(CG_SET, wNumDstCG, sf);
	return OK;
}

// 全てのCGの開放
int nt_scg_freeall() {
	int i;
	
	for (i = 1; i < CGMAX; i++) {
		nt_scg_free(i);
	}
	return OK;
}

/**
 * 指定の番号の CG をオブジェクトリストから消し、オブジェクトがどこからも参照
 * されていない(参照数が0の)場合のみ、オブジェクトを削除
 */
int nt_scg_free(int no) {
	spcg_assert_no(no);
	
	cginfo_t *cg = cgs[no];
	if (!cg) return NG;
	
	nt_scg_deref(cg);
	cgs[no] = NULL;
	
	return OK;
}

// CGの種類を取得
int nt_scg_querytype(int wNumCG, int *ret) {
	if (wNumCG >= (CGMAX -1)) goto errexit;
	if (cgs[wNumCG] == NULL) goto errexit;

	*ret = cgs[wNumCG]->type;
	
	return OK;

 errexit:
	*ret = CG_NOTUSED;
	return NG;
}

// CGの大きさを取得
int nt_scg_querysize(int wNumCG, int *w, int *h) {
	if (wNumCG >= (CGMAX -1)) goto errexit;
	if (cgs[wNumCG] == NULL) goto errexit;
	if (cgs[wNumCG]->sf == NULL) goto errexit;

	*w = cgs[wNumCG]->sf->width;
	*h = cgs[wNumCG]->sf->height;
	
	return OK;

 errexit:
	*w = *h = 0;
	return NG;
}

// CGのBPPを取得
int nt_scg_querybpp(int wNumCG, int *ret) {
	if (wNumCG >= (CGMAX -1)) goto errexit;
	if (cgs[wNumCG] == NULL) goto errexit;
	if (cgs[wNumCG]->sf == NULL) goto errexit;
	
	*ret = cgs[wNumCG]->sf->depth;

	return OK;

 errexit:
	*ret = 0;
	return NG;
}

// CGの alphamap が存在するかを取得
int nt_scg_existalphamap(int wNumCG, int *ret) {
	if (wNumCG >= (CGMAX -1)) goto errexit;
	if (cgs[wNumCG] == NULL) goto errexit;
	if (cgs[wNumCG]->sf == NULL) goto errexit;
	
	*ret = cgs[wNumCG]->sf->alpha ? 1 : 0;
	
 errexit:
	*ret = 0;
	return NG;
}
