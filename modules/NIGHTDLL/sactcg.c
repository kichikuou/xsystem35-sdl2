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
#include <glib.h>

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

 
/*
  cgの読み込み

    指定の番号のCGをリンクファイルから読み込んだり、
    CG_xxxで作成したCGを参照する
    
  @param no: 読み込むCG番号
  @param refinc: 参照カウンタを増やすかどうか。
                 spriteから参照されるときは増やし、CG_xxxを作る時に
                 参照されるときは増やさない。
*/
cginfo_t *scg_loadcg_no(int no, boolean refinc) {
	cginfo_t *i;
	
	if (no >= (CGMAX -1)) {
		WARNING("no is too large (should be %d < %d)\n", (no), CGMAX);
		return NULL;
	}
	
	// すでに ロードされているか、CG_xxx で作成ずみの場合は
	// 参照カウンタを増やす
	if (cgs[no] != NULL) {
		if (refinc) {
			cgs[no]->refcnt++;
		}
		return cgs[no];
	}
	
	i = g_new(cginfo_t, 1);
	i->type = CG_LINKED;
	i->no   = no;
	i->refcnt = (refinc ? 1 : 0);
	i->sf   = sf_loadcg_no(no -1);
	if (i->sf == NULL) {
		WARNING("load fail (%d)\n", no -1);
		g_free(i);
		return NULL;
	}
	
	cgs[no] = i;
	
	return i;
}

//  指定の大きさ、色の矩形の CG を作成
int scg_create(int wNumCG, int wWidth, int wHeight, int wR, int wG, int wB, int wBlendRate) {
	cginfo_t *i;
	
	spcg_assert_no(wNumCG);
	
	i = g_new(cginfo_t, 1);
	i->type = CG_SET;
	i->no   = wNumCG;
	i->refcnt = 1;
	i->sf = sf_create_surface(wWidth, wHeight, sf0->depth);
	gr_fill(i->sf, 0, 0, wWidth, wHeight, wR, wG, wB);
	gr_fill_alpha_map(i->sf, 0, 0, wWidth, wHeight, wBlendRate);
	
	// もし前に作成したものがあり、未開放の場合は開放
	scg_free(wNumCG);
	
	cgs[wNumCG] = i;
	
	return OK;
}

// 指定のCGを反転させたCGを作成
int scg_create_reverse(int wNumCG, int wNumSrcCG, int wReverseX, int wReverseY) {
	cginfo_t *i, *srccg;
	surface_t *src;
	
	spcg_assert_no(wNumCG);
	spcg_assert_no(wNumSrcCG);
	
	// 元にするCGを参照 (LINKCGなら読み込み)
	if (NULL == (srccg = scg_loadcg_no(wNumSrcCG, FALSE))) {
		return NG;
	}
	
	i = g_new(cginfo_t, 1);
	i->type = CG_REVERSE;
	i->no   = wNumCG;
	i->refcnt = 0;
	
	src = srccg->sf;
	i->sf = stretch(src, src->width, src->height, (wReverseX << 1) | wReverseY);
	// もし前に作成したものがあり、未開放の場合は開放
	scg_free(wNumCG);
	
	cgs[wNumCG] = i;
	
	return OK;
}

// 指定のCGを拡大/縮小したCGを作成
int scg_create_stretch(int wNumCG, int wWidth, int wHeight, int wNumSrcCG) {
	cginfo_t *i, *srccg;
	surface_t *src;
	
	spcg_assert_no(wNumCG);
	spcg_assert_no(wNumSrcCG);

	// 元にするCGを参照 (LINKCGなら読み込み)
	if (NULL == (srccg = scg_loadcg_no(wNumSrcCG, FALSE))) {
		return NG;
	}
	
	i = g_new(cginfo_t, 1);
	i->type = CG_STRETCH;
	i->no   = wNumCG;
	i->refcnt = 0;
	
	src = srccg->sf;
	i->sf = stretch(src, wWidth, wHeight, 0);
	
	// もし前に作成したものがあり、未開放の場合は開放
	scg_free(wNumCG);
	
	cgs[wNumCG] = i;
	
	return OK;
}

// ベースCGの上にブレンドCGを重ねた CG を作成
int scg_create_blend(int wNumDstCG, int wNumBaseCG, int wX, int wY, int wNumBlendCG, int wAlphaMapMode) {
	cginfo_t *i, *basecg, *blendcg;
	
	spcg_assert_no(wNumDstCG);
	spcg_assert_no(wNumBaseCG);
	spcg_assert_no(wNumBlendCG);
	
	// 元にするCGを参照 (LINKCGなら読み込み)
	basecg  = scg_loadcg_no(wNumBaseCG, FALSE);
	blendcg = scg_loadcg_no(wNumBlendCG, FALSE);
	if (basecg == NULL || blendcg == NULL) return NG;
	
	i = g_new(cginfo_t, 1);
	i->type = CG_SET;
	i->no = wNumDstCG;
	i->refcnt = 0;
	
	i->sf = blend(basecg->sf, wX , wY, blendcg->sf, wAlphaMapMode);
	
	// もし前に作成したものがあり、未開放の場合は開放
	scg_free(wNumDstCG);
	
	cgs[wNumDstCG] = i;
	
	return OK;
}

// 指定の文字列のCGを作成
int scg_create_text(int wNumCG, int wSize, int wR, int wG, int wB, char *cText) {
	cginfo_t *i;
	agsurface_t *glyph;
	FONT *font;
	
	if (0) {
		char *b = sjis2euc(cText);
		WARNING("str = '%s'\n", b);
		free(b);
	}
	
	spcg_assert_no(wNumCG);
	
	// 勝手に出ていいのかな？
	if (strlen(cText) == 0) return OK;
	
	font = nact->ags.font;
	font->sel_font(FONT_GOTHIC, wSize);
	
	glyph = font->get_glyph(cText);
	i = g_new(cginfo_t, 1);
	i->type = CG_SET;
	i->no = wNumCG;
	i->refcnt = 0;
	
	i->sf = sf_create_surface(glyph->width, wSize, nact->ags.dib->depth);
	gr_fill(i->sf, 0, 0, glyph->width, wSize, wR, wG, wB);
	gr_draw_amap(i->sf, 0, 0, glyph->pixel, glyph->width, wSize, glyph->bytes_per_line);
	
	// もし前に作成したものがあり、未開放の場合は開放
	scg_free(wNumCG);
	
	cgs[wNumCG] = i;
	
	return OK;
}

// 数字文字列のCGを作成
int scg_create_textnum(int wNumCG, int wSize, int wR, int wG, int wB, int wFigs, int wZeroPadding, int wValue) {
	cginfo_t *i;
	agsurface_t *glyph;
	FONT *font;
	char s[256], ss[256];
	
	spcg_assert_no(wNumCG);
	
	if (wZeroPadding) {
		char *sss = "%%0%dd";
		sprintf(ss, sss, wFigs);
	} else {
		char *sss = "%%%dd";
		sprintf(ss, sss, wFigs);
	}
	sprintf(s, ss, wValue);
	
	font = nact->ags.font;
	font->sel_font(FONT_GOTHIC, wSize);
	glyph = font->get_glyph(s);
	
	i = g_new(cginfo_t, 1);
	i->type = CG_SET;
	i->no = wNumCG;
	i->refcnt = 0;
	i->sf = sf_create_surface(glyph->width, wSize, nact->ags.dib->depth);
	gr_fill(i->sf, 0, 0, glyph->width, wSize, wR, wG, wB);
	gr_draw_amap(i->sf, 0, 0, glyph->pixel, glyph->width, wSize, glyph->bytes_per_line);
	
	// もし前に作成したものがあり、未開放の場合は開放
	scg_free(wNumCG);
	
	cgs[wNumCG] = i;
	
	return OK;
}

// CGを複製
int scg_copy(int wNumDstCG, int wNumSrcCG) {
	cginfo_t *i, *srccg;
	
	spcg_assert_no(wNumDstCG);
	spcg_assert_no(wNumSrcCG);
	
	// 元にするCGを参照 (LINKCGなら読み込み)
	if (NULL == (srccg = scg_loadcg_no(wNumSrcCG, FALSE))) {
		return NG;
	}
	
	i = g_new(cginfo_t, 1);
	i->type = CG_SET;
	i->no = wNumDstCG;
	i->refcnt = 0;
	i->sf = sf_dup(srccg->sf);
	
	// もし前に作成したものがあり、未開放の場合は開放
	scg_free(wNumDstCG);
	
	cgs[wNumDstCG] = i;
	
	return OK;
}

// CGの一部を切りぬいたCGを作成
int scg_cut(int wNumDstCG, int wNumSrcCG, int wX, int wY, int wWidth, int wHeight) {
	cginfo_t *i, *srccg;
	surface_t *dst, *src;
	
	spcg_assert_no(wNumDstCG);
	spcg_assert_no(wNumSrcCG);
	
	// 元にするCGを参照 (LINKCGなら読み込み)
	if (NULL == (srccg = scg_loadcg_no(wNumSrcCG, FALSE))) {
		return NG;
	}
	
	i = g_new(cginfo_t, 1);
	i->type = CG_SET;
	i->no = wNumDstCG;
	i->refcnt = 0;
	
	src = srccg->sf;
	if (src->has_alpha) {
		dst = sf_create_surface(wWidth, wHeight, src->depth);
	} else {
		dst = sf_create_pixel(wWidth, wHeight, src->depth);
	}
	if (src->has_pixel) {
		gr_copy(dst, 0, 0, src, wX, wY, wWidth, wHeight);
	}
	if (src->has_alpha) {
		gr_copy_alpha_map(dst, 0, 0, src, wX, wY, wWidth, wHeight);
	}
	
	i->sf = dst;
	
	// もし前に作成したものがあり、未開放の場合は開放
	scg_free(wNumDstCG);
	
	cgs[wNumDstCG] = i;
	
	return OK;
}

// 元のCGの一部を切りぬいたCGを作成
int scg_partcopy(int wNumDstCG, int wNumSrcCG, int wX, int wY, int wWidth, int wHeight) {
	cginfo_t *i, *srccg;
	surface_t *dst, *src;
	
	spcg_assert_no(wNumDstCG);
	spcg_assert_no(wNumSrcCG);
	
	// 元にするCGを参照 (LINKCGなら読み込み)
	if (NULL == (srccg = scg_loadcg_no(wNumSrcCG, FALSE))) {
		return NG;
	}
	
	i = g_new(cginfo_t, 1);
	i->type = CG_SET;
	i->no = wNumDstCG;
	i->refcnt = 0;

	src = srccg->sf;
	if (src->has_alpha) {
		dst = sf_create_surface(src->width, src->height, src->depth);
		gr_fill_alpha_map(dst, 0, 0, src->width, src->height, 255);
	} else {
		dst = sf_create_pixel(src->width, src->height, src->depth);
	}
	
	if (src->has_pixel) {
		gr_copy(dst, wX, wY, src, wX, wY, wWidth, wHeight);
	}
	if (src->has_alpha) {
		gr_copy_alpha_map(dst, wX, wY, src, wX, wY, wWidth, wHeight);
	}
	
	i->sf = dst;
	
	// もし前に作成したものがあり、未開放の場合は開放
	scg_free(wNumDstCG);
	
	cgs[wNumDstCG] = i;
	
	return OK;
}

// 全てのCGの開放
int scg_freeall() {
	int i;
	
	for (i = 1; i < CGMAX; i++) {
		scg_free(i);
	}
	return OK;
}

/**
 * 指定の番号の CG をオブジェクトリストから消し、オブジェクトがどこからも参照
 * されていない(参照数が0の)場合のみ、オブジェクトを削除
 */
int scg_free(int no) {
	cginfo_t *cg;
	
	spcg_assert_no(no);
	
	if (NULL == (cg = cgs[no])) return NG;
	
	// 参照数が0の時のみオブジェクトを開放
	if (cg->refcnt == 0) {
		scg_free_cgobj(cg);
	}
	
	// 番号で消したときはオブジェクトが開放されなくても
	// オブジェクトリストから削除
	cgs[no] = NULL;
	
	return OK;
}

/**
 * CG オブジェクトの開放
 */
int scg_free_cgobj(cginfo_t *cg) {
	if (cg == NULL) return NG;
	
	(cg->refcnt)--;
	// 他でまだ参照していれば開放しない
	if (cg->refcnt > 0) {
		return NG;
	}
	
	// CG本体の開放
	if (cg->sf) {
		sf_free(cg->sf);
	}
	
	// 削除するオブジェクトを参照しているオブジェクトリストも削除
	if (cg == cgs[cg->no]) {
		cgs[cg->no] = NULL;
	}
	
	g_free(cg);
	
	return OK;
}

// CGの種類を取得
int scg_querytype(int wNumCG, int *ret) {
	if (wNumCG >= (CGMAX -1)) goto errexit;
	if (cgs[wNumCG] == NULL) goto errexit;

	*ret = cgs[wNumCG]->type;
	
	return OK;

 errexit:
	*ret = CG_NOTUSED;
	return NG;
}

// CGの大きさを取得
int scg_querysize(int wNumCG, int *w, int *h) {
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
int scg_querybpp(int wNumCG, int *ret) {
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
int scg_existalphamap(int wNumCG, int *ret) {
	if (wNumCG >= (CGMAX -1)) goto errexit;
	if (cgs[wNumCG] == NULL) goto errexit;
	if (cgs[wNumCG]->sf == NULL) goto errexit;
	
	*ret = cgs[wNumCG]->sf->has_alpha ? 1 : 0;
	
 errexit:
	*ret = 0;
	return NG;
}
