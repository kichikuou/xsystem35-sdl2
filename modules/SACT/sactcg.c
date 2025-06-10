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

#include <assert.h>
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
#include "cg.h"
#include "gfx.h"

#define SPCG_ASSERT_NO(no) \
  if ((no) > (CGMAX -1)) { \
    WARNING("no is too large (should be %d < %d)", (no), CGMAX); \
    return; \
  } \

static cginfo_t *scg_new(enum cgtype type, int no, SDL_Surface *sf) {
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

	SDL_Surface *sf = cg_load_as_sdlsurface(no - 1);
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
		SDL_FreeSurface(cg->sf);
	free(cg);
}

//  指定の大きさ、色の矩形の CG を作成
void scg_create(int wNumCG, int wWidth, int wHeight, int wR, int wG, int wB, int wBlendRate) {
	SPCG_ASSERT_NO(wNumCG);

	SDL_Surface *sf = SDL_CreateRGBSurfaceWithFormat(0, wWidth, wHeight, 32, SDL_PIXELFORMAT_ARGB8888);
	SDL_FillRect(sf, NULL, SDL_MapRGBA(sf->format, wR, wG, wB, wBlendRate));
	scg_new(CG_SET, wNumCG, sf);
}

// 指定のCGを反転させたCGを作成
void scg_create_reverse(int wNumCG, int wNumSrcCG, int wReverseX, int wReverseY) {
	SPCG_ASSERT_NO(wNumCG);
	SPCG_ASSERT_NO(wNumSrcCG);
	
	cginfo_t *src = scg_get(wNumSrcCG);
	if (!src)
		return;

	SDL_Surface *sf = SDL_ConvertSurface(src->sf, src->sf->format, 0);
	if (wReverseX)
		gfx_FlipSurfaceHorizontal(sf);
	if (wReverseY)
		gfx_FlipSurfaceVertical(sf);
	scg_new(CG_REVERSE, wNumCG, sf);
}

// 指定のCGを拡大/縮小したCGを作成
void scg_create_stretch(int wNumCG, int wWidth, int wHeight, int wNumSrcCG) {
	SPCG_ASSERT_NO(wNumCG);
	SPCG_ASSERT_NO(wNumSrcCG);
	
	cginfo_t *src = scg_get(wNumSrcCG);
	if (!src)
		return;

	SDL_Surface *sf = SDL_CreateRGBSurfaceWithFormat(0, wWidth, wHeight, src->sf->format->BitsPerPixel, src->sf->format->format);
	SDL_BlitScaled(src->sf, NULL, sf, NULL);
	scg_new(CG_STRETCH, wNumCG, sf);
}

static SDL_Surface *blend(SDL_Surface *base, int x, int y, SDL_Surface *blend, int mode) {
	assert(blend->format->format == SDL_PIXELFORMAT_ARGB8888 || blend->format->format == SDL_PIXELFORMAT_XRGB8888);
	bool src_has_alpha = SDL_ISPIXELFORMAT_ALPHA(blend->format->format);
	SDL_Surface *dst = SDL_ConvertSurfaceFormat(base, SDL_PIXELFORMAT_ARGB8888, 0);
	SDL_Rect rect = {x, y, blend->w, blend->h};
	SDL_IntersectRect(&(SDL_Rect){0, 0, dst->w, dst->h}, &rect, &rect);

	for (int i = 0; i < rect.h; i++) {
		uint8_t *dstp = (uint8_t *)dst->pixels + (i + rect.y) * dst->pitch + rect.x * 4;
		uint8_t *srcp = (uint8_t *)blend->pixels + i * blend->pitch;
		for (int j = 0; j < rect.w; j++) {
			uint8_t *dstpp = dstp + j * 4;
			uint8_t *srcpp = srcp + j * 4;
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
			// Use base's alpha map if blend has no alpha map
			uint8_t alpha = src_has_alpha ? srcpp[3] : dstpp[3];
			if (alpha == 0) {
				// do nothing
			} else if (alpha == 255) {
				memcpy(dstpp, srcpp, 3);
			} else {
				for (int k = 0; k < 3; k++) {
					dstpp[k] = (dstpp[k] * (255 - alpha) + srcpp[k] * alpha) / 255;
				}
			}
			if (mode) {
				int dsta = dstpp[3] + srcpp[3];
				dstpp[3] = dsta > 255 ? 255 : dsta;
			}
#else
			// Use base's alpha map if blend has no alpha map
			uint8_t alpha = src_has_alpha ? srcpp[0] : dstpp[0];
			if (alpha == 0) {
				// do nothing
			} else if (alpha == 255) {
				memcpy(dstpp + 1, srcpp + 1, 3);
			} else {
				for (int k = 1; k < 4; k++) {
					dstpp[k] = (dstpp[k] * (255 - alpha) + srcpp[k] * alpha) / 255;
				}
			}
			if (mode) {
				int dsta = dstpp[0] + srcpp[0];
				dstpp[0] = dsta > 255 ? 255 : dsta;
			}
#endif
		}
	}

	return dst;
}

// ベースCGの上にブレンドCGを重ねた CG を作成
void scg_create_blend(int wNumDstCG, int wNumBaseCG, int wX, int wY, int wNumBlendCG, int wAlphaMapMode) {
	SPCG_ASSERT_NO(wNumDstCG);
	SPCG_ASSERT_NO(wNumBaseCG);
	SPCG_ASSERT_NO(wNumBlendCG);

	cginfo_t *basecg  = scg_get(wNumBaseCG);
	cginfo_t *blendcg = scg_get(wNumBlendCG);
	if (!basecg || !blendcg) return;

	SDL_Surface *sf = blend(basecg->sf, wX, wY, blendcg->sf, wAlphaMapMode);
	scg_new(CG_SET, wNumDstCG, sf);
}

// 指定の文字列のCGを作成
void scg_create_text(int wNumCG, int wSize, int wR, int wG, int wB, int wText) {
	SPCG_ASSERT_NO(wNumCG);
	
	// 勝手に出ていいのかな？
	if (svar_length(wText) == 0) return;

	ags_setFont(FONT_GOTHIC, wSize);
	SDL_Surface *glyph = ags_drawStringToSurface(svar_get(wText), wR, wG, wB);

	SDL_Surface *sf = SDL_CreateRGBSurfaceWithFormat(0, glyph->w, wSize, 32, SDL_PIXELFORMAT_ARGB8888);
	SDL_SetSurfaceBlendMode(glyph, SDL_BLENDMODE_NONE);
	SDL_BlitSurface(glyph, NULL, sf, NULL);
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
	SDL_Surface *glyph = ags_drawStringToSurface(s, wR, wG, wB);

	SDL_Surface *sf = SDL_CreateRGBSurfaceWithFormat(0, glyph->w, wSize, 32, SDL_PIXELFORMAT_ARGB8888);
	SDL_SetSurfaceBlendMode(glyph, SDL_BLENDMODE_NONE);
	SDL_BlitSurface(glyph, NULL, sf, NULL);
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

	scg_new(CG_SET, wNumDstCG, SDL_ConvertSurface(src->sf, src->sf->format, 0));
}

// CGの一部を切りぬいたCGを作成
void scg_cut(int wNumDstCG, int wNumSrcCG, int wX, int wY, int wWidth, int wHeight) {
	SPCG_ASSERT_NO(wNumDstCG);
	SPCG_ASSERT_NO(wNumSrcCG);

	cginfo_t *src = scg_get(wNumSrcCG);
	if (!src)
		return;

	SDL_Surface *sf = SDL_CreateRGBSurfaceWithFormat(0, wWidth, wHeight, src->sf->format->BitsPerPixel, src->sf->format->format);
	SDL_BlitSurface(src->sf, &(SDL_Rect){wX, wY, wWidth, wHeight}, sf, NULL);

	scg_new(CG_SET, wNumDstCG, sf);
}

// 元のCGの一部を切りぬいたCGを作成
void scg_partcopy(int wNumDstCG, int wNumSrcCG, int wX, int wY, int wWidth, int wHeight) {
	SPCG_ASSERT_NO(wNumDstCG);
	SPCG_ASSERT_NO(wNumSrcCG);

	cginfo_t *src = scg_get(wNumSrcCG);
	if (!src)
		return;

	SDL_Surface *sf = SDL_CreateRGBSurfaceWithFormat(0, src->sf->w, src->sf->h, src->sf->format->BitsPerPixel, src->sf->format->format);
	SDL_FillRect(sf, NULL, SDL_MapRGBA(sf->format, 0, 0, 0, 255));
	SDL_BlitSurface(src->sf, &(SDL_Rect){wX, wY, wWidth, wHeight}, sf, &(SDL_Rect){wX, wY, wWidth, wHeight});

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

	*w = sact.cg[wNumCG]->sf->w;
	*h = sact.cg[wNumCG]->sf->h;
	
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
	return sact.cg[wNumCG]->sf->format->BitsPerPixel;
}

// CGの alphamap が存在するかを取得
bool scg_existalphamap(int wNumCG) {
	if (wNumCG >= (CGMAX -1)) return false;
	if (sact.cg[wNumCG] == NULL) return false;
	if (sact.cg[wNumCG]->sf == NULL) return false;
	return SDL_ISPIXELFORMAT_ALPHA(sact.cg[wNumCG]->sf->format->format);
}
