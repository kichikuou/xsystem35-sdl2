/*
 * Gpx.c  Graphics 汎用関数
 *
 *      OnlyYou -リ・クスル
 *      system3.9化 鬼畜王ランス
 *      大悪司
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
/* $Id: Gpx.c,v 1.11 2003/04/22 16:29:52 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#include "portab.h"
#include "system.h"
#include "xsystem35.h"
#include "modules.h"
#include "nact.h"
#include "ags.h"
#include "cg.h"
#include "image.h"
#include "input.h"
// #include "alpha_plane.h"
#include "surface.h"
#include "graph.h"
#include "graph_blend_amap.h"

#include "effectcopy.h"
#include "ngraph.h"

#define MAX_SURFACE 256




static int pre_freesurfno; /* 直前に開放した surface no */
static surface_t *suf[MAX_SURFACE];

static int find_null_surface() {
	int i;

	if (suf[pre_freesurfno] == NULL) return pre_freesurfno;

	for (i = 1; i < MAX_SURFACE; i++) {
		if (suf[i] == NULL) return i;
	}
	
	SYSERROR("no free surface");
	return 0;
}

static void sf_free_one(int no) {
	if (no == 0 || !suf[no]) return;
	sf_free(suf[no]);
	suf[no] = NULL;
	pre_freesurfno = no;
}

static void sf_free_all() {
	for (int i = 1; i < MAX_SURFACE; i++) {
		sf_free(suf[i]);
		suf[i] = NULL;
	}
	pre_freesurfno = 1;
}

static surface_t *sf_get(int no) {
	if (no == 0) {
		return nact->ags.dib;
	}
	return suf[no];
}

static int load_cg_main(int no) {
	surface_t *sf = sf_loadcg_no(no);
	int sno;

	if (sf == NULL) {
		WARNING("load fail(cg==NULL,no=%d)", no);
		return 0;
	}
	
	sno = find_null_surface();
	
	suf[sno] = sf;
	
	return sno;
}

static void Init() {
	/*
	  Gpx.Init(): Gpx モジュールの初期化
	*/
	int p1 = getCaliValue(); /* ISys3x */
	
	suf[0] = NULL;
	gr_init();
	
	// surface0 を pre_freesurfno として返さないように。
	pre_freesurfno = 1;
	
	TRACE("Gpx.Init %d:", p1);
}

static void Gpx_reset(void) {
	sf_free_all();
}

static void Create() {
	/*
	  Gpx.Create(): 新規 surface の作成(PixelとAlphaマップの両方)
	  
	   var   : 作成した surface の番号を返す変数
	           作成に失敗した場合は 0 を返す
	   width : surface の幅
	   height: surface の高さ
	   bpp   : surface の深さ(24bppのみサポート)
	*/
	int *var   = getCaliVariable();
	int width  = getCaliValue();
	int height = getCaliValue();
	int bpp    = getCaliValue();

	surface_t *s = sf_create_surface(width, height);

	if (s == NULL) {
		*var = 0;
	} else {
		int no = find_null_surface();
		*var = no;
		suf[no] = s;
	}
	
	TRACE("Gpx.Create %p,%d,%d,%d:", var, width, height, bpp);
}

static void CreatePixelOnly() {
	/*
	  Gpx.CreatePixelOnly(): 新規 surface の作成(Pixelのみ)
	  
	   var   : 作成した surface の番号を返す変数
	   width : surface の幅
	   height: surface の高さ
	   bpp   : surface の深さ(24bpp only)
	*/
	int *var   = getCaliVariable();
	int width  = getCaliValue();
	int height = getCaliValue();
	int bpp    = getCaliValue();

	surface_t *s = sf_create_pixel(width, height);

	if (s == NULL) {
		*var = 0;
	} else {
		int no = find_null_surface();
		*var = no;
		suf[no] = s;
	}
	
	TRACE("Gpx.CreatePixelOnly %d,%d,%d,%d:", *var, width, height, bpp);
}

static void CreateAMapOnly() {
	/*
	  Gpx.CreateAMapOnly(): 新規 surface の作成(AlphaMapのみ)
	  
	   var   : 作成した surface の番号を返す変数
	   width : surface の幅
	   height: surface の高さ
	*/
	int *var   = getCaliVariable();
	int width  = getCaliValue();
	int height = getCaliValue();
	surface_t *s;
	
	s = sf_create_alpha(width, height);
	
	if (s == NULL) {
		*var = 0;
	} else {
		int no = find_null_surface();
		*var = no;
		suf[no] = s;
	}
	
	TRACE("Gpx.CreateAMapOnly %p,%d,%d:", var, width, height);
}

static void IsSurface() {
	/*
	  Gpx.IsSurface(): 指定の番号が surface かどうかを調べる
	  
	   p1  : surface 番号
	   var : 結果を返す変数。surface ならば 1, !surface ならば 0
	*/
	int p1   = getCaliValue();
	int *var = getCaliVariable();

	*var = sf_get(p1) ? 1 : 0;

	TRACE("Gpx.IsSurface %d,%p:", p1, var);
}

static void IsPixel() {
	/*
	  Gpx.IsPixel(): 指定の番号の surface が pixelデータかどうかを調べる
	  
	   p1  : surface 番号
	   var : 結果を返す変数。pixel ならば 1, !pixel ならば 0
	*/
	int p1   = getCaliValue();
	int *var = getCaliVariable();
	surface_t *s;
	
	s = sf_get(p1);
	
	if (s == NULL) {
		*var = 0;
	} else {
		*var = s->sdl_surface ? 1 : 0;
	}
	
	TRACE("Gpx.IsPixel %d,%p:", p1, var);
}

static void IsAlpha() {
	/*
	  Gpx.IsAlpha(): 指定の番号の surface が alpha mapかどうかを調べる
	  
	   p1  : surface 番号
	   var : 結果を返す変数。alpha ならば 1, !alpha ならば 0
	*/
	int p1   = getCaliValue();
	int *var = getCaliVariable();
	surface_t *s;
	
	s = sf_get(p1);
	
	if (s == NULL) {
		*var = 0;
	} else {
		*var = s->alpha ? 1 : 0;
	}
	
	TRACE("Gpx.IsAlpha %d,%p:", p1, var);
}

static void GetWidth() {
	/*
	  Gpx.GetWidth(): 指定の番号の surface の幅を取得する
	  
	   p1  : surface 番号
	   var : 結果を返す変数。
	*/
	int p1   = getCaliValue();
	int *var = getCaliVariable();
	surface_t *s;
	
	s = sf_get(p1);
	
	if (s == NULL) {
		*var = 0;
	} else {
		*var = s->width;
	}
	
	TRACE("Gpx.GetWidth %d,%d:", p1, *var);
}

static void GetHeight() {
	/*
	  Gpx.GetWidth(): 指定の番号の surface の高さを取得する
	  
	   p1  : surface 番号
	   var : 結果を返す変数。
	*/
	int p1   = getCaliValue();
	int *var = getCaliVariable();
	surface_t *s;
	
	s = sf_get(p1);
	
	if (s == NULL) {
		*var = 0;
	} else {
		*var = s->height;
	}
	
	TRACE("Gpx.GetHeight %d,%d:", p1, *var);
}

static void GetCreatedSurface() { /* not used ? */
	int *var = getCaliVariable();
	
	TRACE_UNIMPLEMENTED("Gpx.GetCreatedSurface %p:", var);
}

static void LoadCG() {
	/*
	  Gpx.LoadCG(): 新規 surface を作成してその上に CG を load
	  
	   var : 作成した surface の番号を返す変数
	   p1  : 読み込む CG の番号
	*/
	int *var = getCaliVariable();
	int p1   = getCaliValue();
	
	*var = load_cg_main(p1 -1);
	
	TRACE("Gpx.LoadCG %p,%d (%d):", var, p1, *var);
}

static void GetCGPosX() { /* not useed ? */
	int *var = getCaliVariable();
	int p1 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("Gpx.GetCgPosX %p,%d:", var, p1);
}

static void GetCGPosY() { /* not useed ? */
	int *var = getCaliVariable();
	int p1 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("Gpx.GetCgPosY %p,%d:", var, p1);
}

static void Free() {
	/*
	  Gpx.Free(): 指定の surface を開放する
	  
	   p1: 開放する surface の番号
	*/
	int p1 = getCaliValue();
	
	TRACE("Gpx.Free %d:", p1);
	
	if (p1 != 0) {
		sf_free_one(p1);
	}
}

static void FreeAll() {
	/*
	  Gpx.FreeAll(): 全ての surface を開放する
	*/
	sf_free_all();
	
	TRACE("Gpx.FreeAll:");
}

static void Copy() {
	/*
	  Gpx.Copy(): 指定 surface 領域のコピー
	  
	   ds: 転送先 surface 番号
	   dx: 転送先 x 座標
	   dy: 転送先 y 座標
	   ss: 転送元 surface 番号
	   sx: 転送元 x 座標
	   sy: 転送元 y 座標
	   sw: 転送元 幅
	   sh: 転送元 高さ
	*/
	int ds = getCaliValue();
	int dx = getCaliValue();
	int dy = getCaliValue();
	int ss = getCaliValue();
	int sx = getCaliValue();
	int sy = getCaliValue();
	int sw = getCaliValue();
	int sh = getCaliValue();
	surface_t *src, *dst;
	
	TRACE("Gpx.Copy %d,%d,%d,%d,%d,%d,%d,%d:", ds, dx, dy, ss, sx, sy, sw, sh);
	
	src = sf_get(ss);
	dst = sf_get(ds);
	gr_copy(dst, dx, dy, src, sx, sy, sw, sh);
}

static void CopyBright(void) {
	int ds = getCaliValue();
	int dx = getCaliValue();
	int dy = getCaliValue();
	int ss = getCaliValue();
	int sx = getCaliValue();
	int sy = getCaliValue();
	int w = getCaliValue();
	int h = getCaliValue();
	int lv = getCaliValue();

	surface_t *dst = sf_get(ds);
	surface_t *src = sf_get(ss);
	gr_copy_bright(dst, dx, dy, src, sx, sy, w, h, lv);

	TRACE("Gpx.CopyBright %d,%d,%d,%d,%d,%d,%d,%d,%d:", ds, dx, dy, ss, sx, sy, w, h, lv);
}

static void CopyAMap() {
	/*
	  Gpx.CopyAMap(): 指定 surface の alpha map 領域のコピー
	  
	   da: 転送先 surface(alpha map) 番号
	   dx: 転送先 x 座標
	   dy: 転送先 y 座標
	   sa: 転送元 surface(alpha map) 番号
	   sx: 転送元 x 座標
	   sy: 転送元 y 座標
	   sw: 転送元 幅
	   sh: 転送元 高さ
	*/
	int da = getCaliValue();
	int dx = getCaliValue();
	int dy = getCaliValue();
	int sa = getCaliValue();
	int sx = getCaliValue();
	int sy = getCaliValue();
	int sw = getCaliValue();
	int sh = getCaliValue();
	surface_t *src, *dst;
	
	TRACE("Gpx.CopyAMap %d,%d,%d,%d,%d,%d,%d,%d:", da, dx, dy, sa, sx, sy, sw, sh);
	
	src = sf_get(sa);
	dst = sf_get(da);
	gr_copy_alpha_map(dst, dx, dy, src, sx, sy, sw, sh);
}

static void Blend(void) {
	int ds = getCaliValue();
	int dx = getCaliValue();
	int dy = getCaliValue();
	int ss = getCaliValue();
	int sx = getCaliValue();
	int sy = getCaliValue();
	int w = getCaliValue();
	int h = getCaliValue();
	int lv = getCaliValue();

	surface_t *dst = sf_get(ds);
	surface_t *src = sf_get(ss);
	gr_blend(dst, dx, dy, src, sx, sy, w, h, lv);

	TRACE("Gpx.Blend %d,%d,%d,%d,%d,%d,%d,%d,%d:", ds, dx, dy, ss, sx, sy, w, h, lv);
}

static void BlendSrcBright() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	int p9 = getCaliValue();
	int p10 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("Gpx.BlendSrcBright %d,%d,%d,%d,%d,%d,%d,%d,%d,%d:", p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
}

static void BlendAddSatur() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("Gpx.BlendAddStatur %d,%d,%d,%d,%d,%d,%d,%d:", p1, p2, p3, p4, p5, p6, p7, p8);
}

static void BlendAMap() {
	/*
	  Gpx.BlendAMap(): 転送元の alpha map を参照して 指定領域を alpha blend
	  
	   ds: 転送先 surface 番号
	   dx: 転送先 x 座標
	   dy: 転送先 y 座標
	   ss: 転送元 surface 番号
	   sx: 転送元 x 座標
	   sy: 転送元 y 座標
	   sw: 転送元 幅
	   sh: 転送元 高さ
	*/
	int ds = getCaliValue();
	int dx = getCaliValue();
	int dy = getCaliValue();
	int ss = getCaliValue();
	int sx = getCaliValue();
	int sy = getCaliValue();
	int sw = getCaliValue();
	int sh = getCaliValue();
	surface_t *src, *dst;
	
	TRACE("Gpx.BlendAMap %d,%d,%d,%d,%d,%d,%d,%d:", ds, dx, dy, ss, sx, sy, sw, sh);
	
	src = sf_get(ss);
	dst = sf_get(ds);
	gr_blend_alpha_map(dst, dx, dy, src, sx, sy, sw, sh);
}

static void BlendAMapSrcOnly() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("Gpx.BlendAMapSrcOnly %d,%d,%d,%d,%d,%d,%d,%d:", p1, p2, p3, p4, p5, p6, p7, p8);
}

static void BlendAMapColor() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	int p9 = getCaliValue();
	int p10 = getCaliValue();
	int p11 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("Gpx.BlendAMapColor %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d:", p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11);
}

static void BlendAMapColorAlpha() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	int p9 = getCaliValue();
	int p10 = getCaliValue();
	int p11 = getCaliValue();
	int p12 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("Gpx.BlendAMapColorAlpha %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d:", p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12);
}

static void BlendAMapAlpha() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	int p9 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("Gpx.BlendAMapAlpha %d,%d,%d,%d,%d,%d,%d,%d,%d:", p1, p2, p3, p4, p5, p6, p7, p8, p9);
}

static void BlendAMapBright() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	int p9 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("Gpx.BlendAMapBright %d,%d,%d,%d,%d,%d,%d,%d,%d:", p1, p2, p3, p4, p5, p6, p7, p8, p9);
}

static void BlendAMapAlphaSrcBright() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	int p9 = getCaliValue();
	int p10 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("Gpx.BlendAMapBright %d,%d,%d,%d,%d,%d,%d,%d,%d,%d:", p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
}

static void BlendUseAMapColor() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	int p9 = getCaliValue();
	int p10 = getCaliValue();
	int p11 = getCaliValue();
	int p12 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("Gpx.BlendUseAMapColor %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d:", p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12);
}

static void BlendScreen() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("Gpx.BlendScreen %d,%d,%d,%d,%d,%d,%d,%d:", p1, p2, p3, p4, p5, p6, p7, p8);
}

static void BlendMultiply() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("Gpx.BlendMultiply %d,%d,%d,%d,%d,%d,%d,%d:", p1, p2, p3, p4, p5, p6, p7, p8);
}

static void BlendScreenAlpha() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	int p9 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("Gpx.BlendScreenAlpha %d,%d,%d,%d,%d,%d,%d,%d,%d:", p1, p2, p3, p4, p5, p6, p7, p8, p9);
}

static void Fill() {
	/*
	  Gpx.Fill(): 指定領域の塗りつぶし
	  
	   ds: 塗りつぶし surface 番号
	   dx: 領域 x 座標
	   dy: 領域 y 座標
	   dw: 領域 幅
	   dh: 領域 高さ
	   r : 塗りつぶし色 赤
	   g : 塗りつぶし色 緑
	   b : 塗りつぶし色 青
	*/
	int ds = getCaliValue();
	int dx = getCaliValue();
	int dy = getCaliValue();
	int dw = getCaliValue();
	int dh = getCaliValue();
	int r  = getCaliValue();
	int g  = getCaliValue();
	int b  = getCaliValue();
	surface_t *dst;
	
	TRACE("Gpx.Fill %d,%d,%d,%d,%d,%d,%d,%d:", ds, dx, dy, dw, dh, r, g, b);
	
	dst = sf_get(ds);
	gr_fill(dst, dx, dy, dw, dh, r, g, b);
}

static void FillAlphaColor() { /* not used ? */
	int ds = getCaliValue();
	int dx = getCaliValue();
	int dy = getCaliValue();
	int dw = getCaliValue();
	int dh = getCaliValue();
	int r = getCaliValue();
	int g = getCaliValue();
	int b = getCaliValue();
	int lv = getCaliValue();
	surface_t *dst;
	
	TRACE("Gpx.FillAlphaColor %d,%d,%d,%d,%d,%d,%d,%d,%d:", ds, dx, dy, dw, dh, r, g, b, lv);

	dst = sf_get(ds);
	gr_fill_alpha_color(dst, dx, dy, dw, dh, r, g, b, lv);
}

static void FillAMap(void) {
	int ds = getCaliValue();
	int x = getCaliValue();
	int y = getCaliValue();
	int w = getCaliValue();
	int h = getCaliValue();
	int lv = getCaliValue();

	surface_t *dst = sf_get(ds);
	gr_fill_alpha_map(dst, x, y, w, h, lv);

	TRACE("Gpx.FillAMap %d,%d,%d,%d,%d,%d:", ds, x, y, w, h, lv);
}

static void FillAMapOverBorder() {
	/*
	  Gpx.FillAMapOverBorder(): 矩形領域中の 閾値以上の alpha 値を持つ
	                            ものを指定の alpha 値に置き換え。
	  
	   ds: 塗りつぶし surface 番号
	   dx: 領域 x 座標
	   dy: 領域 y 座標
	   dw: 領域 幅
	   dh: 領域 高さ
	   s : 閾値
	   d : 閾値を超えた場合の設定値
	*/
	int ds = getCaliValue();
	int dx = getCaliValue();
	int dy = getCaliValue();
	int dw = getCaliValue();
	int dh = getCaliValue();
	int s = getCaliValue();
	int d = getCaliValue();
	surface_t *dst;
	
	TRACE("Gpx.FillAMapOverBorder %d,%d,%d,%d,%d,%d,%d:", ds, dx, dy, dw, dh, s, d);

	dst = sf_get(ds);
	gr_fill_alpha_overborder(dst, dx, dy, dw, dh, s, d);
}

static void FillAMapUnderBorder() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("Gpx.FillAMapUnderBorder %d,%d,%d,%d,%d,%d,%d:", p1, p2, p3, p4, p5, p6, p7);
}

static void SaturDP_DPxSA() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("Gpx.SaturDP_DPxSA %d,%d,%d,%d,%d,%d,%d,%d:", p1, p2, p3, p4, p5, p6, p7, p8);
}

static void ScreenDA_DAxSA() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("Gpx.ScreenDA_DAxSA %d,%d,%d,%d,%d,%d,%d,%d:", p1, p2, p3, p4, p5, p6, p7, p8);
}

static void AddDA_DAxSA() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("Gpx.AddDA_DAxSA %d,%d,%d,%d,%d,%d,%d,%d:", p1, p2, p3, p4, p5, p6, p7, p8);
}

static void SpriteCopyAMap() {
	/*
	  Gpx.SpriteCopyAMap(): 指定の alpha 値以外の領域の alpha map コピー
	  
	   da: 転送先 surface(alpha map) 番号
	   dx: 転送先 x 座標
	   dy: 転送先 y 座標
	   ss: 転送元 surface(alpha map) 番号
	   sx: 転送元 x 座標
	   sy: 転送元 y 座標
	   sw: 転送元 幅
	   sh: 転送元 高さ
	   cl: 転送しない alpha 値
	*/
	int da = getCaliValue();
	int dx = getCaliValue();
	int dy = getCaliValue();
	int sa = getCaliValue();
	int sx = getCaliValue();
	int sy = getCaliValue();
	int sw = getCaliValue();
	int sh = getCaliValue();
	int cl = getCaliValue();
	surface_t *src, *dst;
	
	TRACE("Gpx.SpriteCopyAMap %d,%d,%d,%d,%d,%d,%d,%d,%d:", da, dx, dy, sa, sx, sy, sw, sh, cl);
	
	src = sf_get(sa);
	dst = sf_get(da);
	gr_copy_alpha_map_sprite(dst, dx, dy, src, sx, sy, sw, sh, cl);
	
}

static void BrightDestOnly() {
	/*
	  Gpx.BrightDestOnly(): 指定領域の明るさを設定
	  
	   ds: 明るさを設定する surface 番号
	   dx: 領域 x 座標
	   dy: 領域 y 座標
	   dw: 領域 幅
	   dh: 領域 高さ
	   r : 指定明るさ (255: 明るさ 100%, pixel値そのもの)
	*/
	int ds = getCaliValue();
	int dx = getCaliValue();
	int dy = getCaliValue();
	int dw = getCaliValue();
	int dh = getCaliValue();
	int r  = getCaliValue();
	surface_t *dst;
	
	TRACE("Gpx.BrightDestOnly %d,%d,%d,%d,%d,%d:", ds, dx, dy, dw, dh, r);
	
	dst = sf_get(ds);
	
	gr_bright_dst_only(dst, dx, dy, dw, dh, r);
}

static void CopyTextureWrap() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	int p9 = getCaliValue();
	int p10 = getCaliValue();
	int p11 = getCaliValue();
	int p12 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("Gpx.CopyTextureWrap %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d:", p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12);
}

static void CopyTextureWrapAlpha() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	int p9 = getCaliValue();
	int p10 = getCaliValue();
	int p11 = getCaliValue();
	int p12 = getCaliValue();
	int p13 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("Gpx.CopyTextureWrapAlpha %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d:", p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13);
}

static void CopyStretch() {
	/*
	  Gpx.CopyStretch(): 拡大・縮小
	  
	   ds: 転送先 surface 番号
	   dx: 転送先 x 座標
	   dy: 転送先 y 座標
	   dw: 転送先 幅
	   dh: 転送先 高さ
	   ss: 転送元 surface 番号
	   sx: 転送元 x 座標
	   sy: 転送元 y 座標
	   sw: 転送元 幅
	   sh: 転送元 高さ
	*/
	int ds = getCaliValue();
	int dx = getCaliValue();
	int dy = getCaliValue();
	int dw = getCaliValue();
	int dh = getCaliValue();
	int ss = getCaliValue();
	int sx = getCaliValue();
	int sy = getCaliValue();
	int sw = getCaliValue();
	int sh = getCaliValue();
	surface_t *src, *dst;

	TRACE("Gpx.CopyStretch %d,%d,%d,%d,%d,%d,%d,%d,%d,%d:", ds, dx, dy, dw, dh, ss, sx, sy, sw, sh);
	
	src = sf_get(ss);
	dst = sf_get(ds);
	gr_copy_stretch(dst, dx, dy, dw, dh, src, sx, sy, sw, sh);
}

static void CopyStretchBlend() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	int p9 = getCaliValue();
	int p10 = getCaliValue();
	int p11 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("Gpx.CopyStretchBlend %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d:", p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11);
}

static void CopyStretchBlendAMap() {
	/*
	  Gpx.CopyStretchBlendAMap(): 拡大・縮小しながら alpha blend
	  
	   ds: 転送先 surface 番号
	   dx: 転送先 x 座標
	   dy: 転送先 y 座標
	   dw: 転送先 幅
	   dh: 転送先 高さ
	   ss: 転送元 surface 番号
	   sx: 転送元 x 座標
	   sy: 転送元 y 座標
	   sw: 転送元 幅
	   sh: 転送元 高さ
	*/
	int ds = getCaliValue();
	int dx = getCaliValue();
	int dy = getCaliValue();
	int dw = getCaliValue();
	int dh = getCaliValue();
	int ss = getCaliValue();
	int sx = getCaliValue();
	int sy = getCaliValue();
	int sw = getCaliValue();
	int sh = getCaliValue();
	surface_t *src, *dst;
	
	TRACE("Gpx.CopyStretchBlendAMap %d,%d,%d,%d,%d,%d,%d,%d,%d,%d:", ds, dx, dy, dw, dh, ss, sx, sy, sw, sh);

	src = sf_get(ss);
	dst = sf_get(ds);
	gr_copy_stretch_blend_alpha_map(dst, dx, dy, dw, dh, src, sx, sy, sw, sh);
}

static void StretchBlendScreen2x2() { /* not used ? */
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	int p3 = getCaliValue();
	int p4 = getCaliValue();
	int p5 = getCaliValue();
	int p6 = getCaliValue();
	int p7 = getCaliValue();
	int p8 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("Gpx.StretchBlendScreen2x2 %d,%d,%d,%d,%d,%d,%d,%d:", p1, p2, p3, p4, p5, p6, p7, p8);
}


static void StretchBlendScreen2x2WDS() {
	/*
	  Gpx.StretchBlenfScreen2x2WDS(): ２枚の surface を縦横２倍に拡大しつつ
                                         alpha blend (飽和加算)する
	  
	   ds: 転送先 surface 番号
	   dx: 転送先 x 座標
	   dy: 転送先 y 座標
	   ss1: 転送元 surface 番号 (拡大元 surface) ss1の方が小さい
	   sx1: 転送元 x 座標
	   sy1: 転送元 y 座標
	   ss2: 転送元 surface 番号 (重ね先の surface)
	   sx2: 転送元 x 座標
	   sy2: 転送元 y 座標
	   sw: 転送元 幅 (sw*2, sh*2)の大きさに拡大
	   sh: 転送元 高さ
	*/
	int ds  = getCaliValue();
	int dx  = getCaliValue();
	int dy  = getCaliValue();
	int ss1 = getCaliValue();
	int sx1 = getCaliValue();
	int sy1 = getCaliValue();
	int ss2 = getCaliValue();
	int sx2 = getCaliValue();
	int sy2 = getCaliValue();
	int sw  = getCaliValue();
	int sh  = getCaliValue();
	surface_t *src1, *src2, *dst;
	
	TRACE("Gpx.StretchBlendScreen2x2WDS %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d:", ds, dx, dy, ss1, sx1, sy1, ss2, sx2, sy2, sw, sh);
	
	src1 = sf_get(ss1);
	src2 = sf_get(ss2);
	dst  = sf_get(ds);
	gr_blend_alpha_wds_stretch2x2(src1, sx1, sy1, src2, sx2, sy2, sw, sh, dst, dx, dy);
}

static void BlendScreenWDS() {
	/*
	  Gpx.BlendScreenWDS(): 飽和加算 alpha blend
	  
	   ds : 転送先 surface 番号
	   dx : 転送先 x 座標
	   dy : 転送先 y 座標
	   ss1: 転送元 surface 番号 (重ね元の surface)
	   sx1: 転送元 x 座標
	   sy1: 転送元 y 座標
	   ss2: 転送元 surface 番号 (重ね先の surface)
	   sx2: 転送元 x 座標
	   sy2: 転送元 y 座標
	   sw : 転送元 幅
	   sh : 転送元 高さ
	*/
	int ds  = getCaliValue();
	int dx  = getCaliValue();
	int dy  = getCaliValue();
	int ss1 = getCaliValue();
	int sx1 = getCaliValue();
	int sy1 = getCaliValue();
	int ss2 = getCaliValue();
	int sx2 = getCaliValue();
	int sy2 = getCaliValue();
	int sw  = getCaliValue();
	int sh  = getCaliValue();
	surface_t *src1, *src2, *dst;
	
	TRACE("Gpx.BlendScreenWDS %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d:", ds, dx, dy, ss1, sx1, sy1, ss2, sx2, sy2, sw, sh);
	
	src1 = sf_get(ss1);
	src2 = sf_get(ss2);
	dst  = sf_get(ds);
	gr_blend_alpha_wds(src1, sx1, sy1, src2, sx2, sy2, sw, sh, dst, dx, dy);
}

static void EffectCopy() {
	/*
	  Gpx.EffectCopy(): 効果つき領域コピー
	  
	   no: 効果番号
	   dx: 転送先 x 座標
	   dy: 転送先 y 座標
	   ss1: 転送元 surface 番号 (重ね元の surface)
	   sx1: 転送元 x 座標
	   sy1: 転送元 y 座標
	   ss2: 転送元 surface 番号 (重ね先の surface)
	   sx2: 転送元 x 座標
	   sy2: 転送元 y 座標
	   sw: 転送元 幅
	   sh: 転送元 高さ
	   time: 実行時間(ms) (0の場合はそれぞのれ効果毎のデフォルト値)
	   var: キー入力があったかないか？(?????)


	  effect no:
	    1: CE31 (cross fade)
	    2: CE29 (fade out)
	    3: CE27 (fade in)
	    4: CE30 (white out)
	    5: CE28 (white in)
	    7: CE11 (すだれ落ち)
	    11: CE53 (線形ぼかし)
	    12: CE35 (上->下クロスフェード)
	    13: CE36 (下->上クロスフェード)
	*/
	int no   = getCaliValue();
	int dx   = getCaliValue();
	int dy   = getCaliValue();
	int ss1  = getCaliValue();
	int sx1  = getCaliValue();
	int sy1  = getCaliValue();
	int ss2  = getCaliValue();
	int sx2  = getCaliValue();
	int sy2  = getCaliValue();
	int sw   = getCaliValue();
	int sh   = getCaliValue();
	int time = getCaliValue();
	int *var = getCaliVariable();

	TRACE("Gpx.EffectCopy %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%p:", no, dx, dy, ss1, sx1, sy1, ss2, sx2, sy2, sw, sh, time, var);

	surface_t *dst = sf_get(ss1);
	surface_t *src = sf_get(ss2);
	gpx_effect(no, dx, dy, dst, sx1, sy1, src, sx2, sy2, sw, sh, time, var);
	
}

static void SetClickCancelFlag() { /* not used ? */
	int p1 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("Gpx.SetClikCancelFlag %d:", p1);
}

static const ModuleFunc functions[] = {
	{"AddDA_DAxSA", AddDA_DAxSA},
	{"Blend", Blend},
	{"BlendAMap", BlendAMap},
	{"BlendAMapAlpha", BlendAMapAlpha},
	{"BlendAMapAlphaSrcBright", BlendAMapAlphaSrcBright},
	{"BlendAMapBright", BlendAMapBright},
	{"BlendAMapColor", BlendAMapColor},
	{"BlendAMapColorAlpha", BlendAMapColorAlpha},
	{"BlendAMapSrcOnly", BlendAMapSrcOnly},
	{"BlendAddSatur", BlendAddSatur},
	{"BlendMultiply", BlendMultiply},
	{"BlendScreen", BlendScreen},
	{"BlendScreenAlpha", BlendScreenAlpha},
	{"BlendScreenWDS", BlendScreenWDS},
	{"BlendSrcBright", BlendSrcBright},
	{"BlendUseAMapColor", BlendUseAMapColor},
	{"BrightDestOnly", BrightDestOnly},
	{"Copy", Copy},
	{"CopyAMap", CopyAMap},
	{"CopyBright", CopyBright},
	{"CopyStretch", CopyStretch},
	{"CopyStretchBlend", CopyStretchBlend},
	{"CopyStretchBlendAMap", CopyStretchBlendAMap},
	{"CopyTextureWrap", CopyTextureWrap},
	{"CopyTextureWrapAlpha", CopyTextureWrapAlpha},
	{"Create", Create},
	{"CreateAMapOnly", CreateAMapOnly},
	{"CreatePixelOnly", CreatePixelOnly},
	{"EffectCopy", EffectCopy},
	{"Fill", Fill},
	{"FillAMap", FillAMap},
	{"FillAMapOverBorder", FillAMapOverBorder},
	{"FillAMapUnderBorder", FillAMapUnderBorder},
	{"FillAlphaColor", FillAlphaColor},
	{"Free", Free},
	{"FreeAll", FreeAll},
	{"GetCGPosX", GetCGPosX},
	{"GetCGPosY", GetCGPosY},
	{"GetCreatedSurface", GetCreatedSurface},
	{"GetHeight", GetHeight},
	{"GetWidth", GetWidth},
	{"Init", Init},
	{"IsAlpha", IsAlpha},
	{"IsPixel", IsPixel},
	{"IsSurface", IsSurface},
	{"LoadCG", LoadCG},
	{"SaturDP_DPxSA", SaturDP_DPxSA},
	{"ScreenDA_DAxSA", ScreenDA_DAxSA},
	{"SetClickCancelFlag", SetClickCancelFlag},
	{"SpriteCopyAMap", SpriteCopyAMap},
	{"StretchBlendScreen2x2", StretchBlendScreen2x2},
	{"StretchBlendScreen2x2WDS", StretchBlendScreen2x2WDS},
};

const Module module_Gpx = {"Gpx", functions, sizeof(functions) / sizeof(ModuleFunc), Gpx_reset};
