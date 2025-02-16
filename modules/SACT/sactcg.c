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
/* $Id: sactcg.c,v 1.7 2004/10/31 04:18:02 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>

#include "portab.h"
#include "nact.h"
#include "system.h"
#include "ags.h"
#include "sact.h"
#include "sactcg.h"
#include "surface.h"
#include "ngraph.h"

#include "sactcg_stretch.c"
#include "sactcg_blend.c"

#define SPCG_ASSERT_NO(no) \
  if ((no) > (CGMAX -1)) { \
    WARNING("no is too large (should be %d < %d)", (no), CGMAX); \
    return; \
  } \

static cginfo_t *scg_new(enum cgtype type, int no, surface_t *sf) {
	cginfo_t *info = calloc(1, sizeof(cginfo_t));
	info->type = type;
	info->no = no;
	info->sf = sf;
	info->refcnt = 1;

	scg_free(no);
	sact.cg[no] = info;

	return info;
}

/*
  cgの読み込み

    指定の番号のCGをリンクファイルから読み込んだり、
    CG_xxxで作成したCGを参照する
    
  @param no: 読み込むCG番号
*/
static cginfo_t *scg_get(int no) {
	if (no >= (CGMAX -1)) {
		WARNING("no is too large (should be %d < %d)", (no), CGMAX);
		return NULL;
	}
	
	if (sact.cg[no] != NULL)
		return sact.cg[no];

	surface_t *sf = sf_loadcg_no(no - 1);
	if (!sf) {
		WARNING("load fail (%d)", no -1);
		return NULL;
	}
	return scg_new(CG_LINKED, no, sf);
}

cginfo_t *scg_addref(int no) {
	cginfo_t *info = scg_get(no);
	if (info)
		info->refcnt++;
	return info;
}

void scg_deref(cginfo_t *cg) {
	if (--cg->refcnt > 0)
		return;

	if (cg->sf)
		sf_free(cg->sf);
	free(cg);
}

//  指定の大きさ、色の矩形の CG を作成
void scg_create(int wNumCG, int wWidth, int wHeight, int wR, int wG, int wB, int wBlendRate) {
	SPCG_ASSERT_NO(wNumCG);

	surface_t *sf = sf_create_surface(wWidth, wHeight, sf0->depth);
	gr_fill(sf, 0, 0, wWidth, wHeight, wR, wG, wB);
	gr_fill_alpha_map(sf, 0, 0, wWidth, wHeight, wBlendRate);
	scg_new(CG_SET, wNumCG, sf);
}

// 指定のCGを反転させたCGを作成
void scg_create_reverse(int wNumCG, int wNumSrcCG, int wReverseX, int wReverseY) {
	SPCG_ASSERT_NO(wNumCG);
	SPCG_ASSERT_NO(wNumSrcCG);
	
	cginfo_t *src = scg_get(wNumSrcCG);
	if (!src)
		return;

	surface_t *sf = stretch(src->sf, src->sf->width, src->sf->height, (wReverseX << 1) | wReverseY);
	scg_new(CG_REVERSE, wNumCG, sf);
}

// 指定のCGを拡大/縮小したCGを作成
void scg_create_stretch(int wNumCG, int wWidth, int wHeight, int wNumSrcCG) {
	SPCG_ASSERT_NO(wNumCG);
	SPCG_ASSERT_NO(wNumSrcCG);
	
	cginfo_t *src = scg_get(wNumSrcCG);
	if (!src)
		return;
	
	surface_t *sf = stretch(src->sf, wWidth, wHeight, 0);
	scg_new(CG_STRETCH, wNumCG, sf);
}

// ベースCGの上にブレンドCGを重ねた CG を作成
void scg_create_blend(int wNumDstCG, int wNumBaseCG, int wX, int wY, int wNumBlendCG, int wAlphaMapMode) {
	SPCG_ASSERT_NO(wNumDstCG);
	SPCG_ASSERT_NO(wNumBaseCG);
	SPCG_ASSERT_NO(wNumBlendCG);

	cginfo_t *basecg  = scg_get(wNumBaseCG);
	cginfo_t *blendcg = scg_get(wNumBlendCG);
	if (!basecg || !blendcg) return;

	surface_t *sf = blend(basecg->sf, wX, wY, blendcg->sf, wAlphaMapMode);
	scg_new(CG_SET, wNumDstCG, sf);
}

// 指定の文字列のCGを作成
void scg_create_text(int wNumCG, int wSize, int wR, int wG, int wB, int wText) {
	SPCG_ASSERT_NO(wNumCG);
	
	// 勝手に出ていいのかな？
	if (svar_length(wText) == 0) return;

	ags_setFont(FONT_GOTHIC, wSize);
	SDL_Surface *glyph = ags_drawStringToSurface(svar_get(wText));

	surface_t *sf = sf_create_surface(glyph->w, wSize, nact->ags.dib->depth);
	gr_fill(sf, 0, 0, glyph->w, wSize, wR, wG, wB);
	gr_draw_amap(sf, 0, 0, glyph->pixels, glyph->w, wSize, glyph->pitch);
	SDL_FreeSurface(glyph);

	scg_new(CG_SET, wNumCG, sf);
}

// 数字文字列のCGを作成
void scg_create_textnum(int wNumCG, int wSize, int wR, int wG, int wB, int wFigs, int wZeroPadding, int wValue) {
	SPCG_ASSERT_NO(wNumCG);

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
	SDL_Surface *glyph = ags_drawStringToSurface(s);

	surface_t *sf = sf_create_surface(glyph->w, wSize, nact->ags.dib->depth);
	gr_fill(sf, 0, 0, glyph->w, wSize, wR, wG, wB);
	gr_draw_amap(sf, 0, 0, glyph->pixels, glyph->w, wSize, glyph->pitch);
	SDL_FreeSurface(glyph);

	scg_new(CG_SET, wNumCG, sf);
}

// CGを複製
void scg_copy(int wNumDstCG, int wNumSrcCG) {
	SPCG_ASSERT_NO(wNumDstCG);
	SPCG_ASSERT_NO(wNumSrcCG);

	cginfo_t *src = scg_get(wNumSrcCG);
	if (!src)
		return;

	scg_new(CG_SET, wNumDstCG, sf_dup(src->sf));
}

// CGの一部を切りぬいたCGを作成
void scg_cut(int wNumDstCG, int wNumSrcCG, int wX, int wY, int wWidth, int wHeight) {
	SPCG_ASSERT_NO(wNumDstCG);
	SPCG_ASSERT_NO(wNumSrcCG);

	cginfo_t *src = scg_get(wNumSrcCG);
	if (!src)
		return;

	surface_t *sf = src->sf->alpha
		? sf_create_surface(wWidth, wHeight, src->sf->depth)
		: sf_create_pixel(wWidth, wHeight, src->sf->depth);

	if (src->sf->pixel)
		gr_copy(sf, 0, 0, src->sf, wX, wY, wWidth, wHeight);
	if (src->sf->alpha)
		gr_copy_alpha_map(sf, 0, 0, src->sf, wX, wY, wWidth, wHeight);

	scg_new(CG_SET, wNumDstCG, sf);
}

// 元のCGの一部を切りぬいたCGを作成
void scg_partcopy(int wNumDstCG, int wNumSrcCG, int wX, int wY, int wWidth, int wHeight) {
	SPCG_ASSERT_NO(wNumDstCG);
	SPCG_ASSERT_NO(wNumSrcCG);

	cginfo_t *src = scg_get(wNumSrcCG);
	if (!src)
		return;

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

	scg_new(CG_SET, wNumDstCG, sf);
}

// 全てのCGの開放
void scg_freeall(void) {
	int i;
	
	for (i = 1; i < CGMAX; i++) {
		scg_free(i);
	}
}

/**
 * 指定の番号の CG をオブジェクトリストから消し、オブジェクトがどこからも参照
 * されていない(参照数が0の)場合のみ、オブジェクトを削除
 */
void scg_free(int no) {
	SPCG_ASSERT_NO(no);
	
	cginfo_t *cg = sact.cg[no];
	if (!cg) return;
	
	scg_deref(cg);
	sact.cg[no] = NULL;
}

// CGの種類を取得
int scg_querytype(int wNumCG) {
	if (wNumCG >= (CGMAX -1)) return CG_NOTUSED;
	if (sact.cg[wNumCG] == NULL) return CG_NOTUSED;
	return sact.cg[wNumCG]->type;
}

// CGの大きさを取得
bool scg_querysize(int wNumCG, int *w, int *h) {
	if (wNumCG >= (CGMAX -1)) goto errexit;
	if (sact.cg[wNumCG] == NULL) goto errexit;
	if (sact.cg[wNumCG]->sf == NULL) goto errexit;

	*w = sact.cg[wNumCG]->sf->width;
	*h = sact.cg[wNumCG]->sf->height;
	
	return true;

 errexit:
	*w = *h = 0;
	return false;
}

// CGのBPPを取得
int scg_querybpp(int wNumCG) {
	if (wNumCG >= (CGMAX -1)) return 0;
	if (sact.cg[wNumCG] == NULL) return 0;
	if (sact.cg[wNumCG]->sf == NULL) return 0;
	
	return sact.cg[wNumCG]->sf->depth;
}

// CGの alphamap が存在するかを取得
bool scg_existalphamap(int wNumCG) {
	if (wNumCG >= (CGMAX -1)) return false;
	if (sact.cg[wNumCG] == NULL) return false;
	if (sact.cg[wNumCG]->sf == NULL) return false;
	
	return sact.cg[wNumCG]->sf->alpha;
}
