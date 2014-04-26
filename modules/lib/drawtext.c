/*
 * drawtext.c  DLL用に surface 上に文字を描く
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
/* $Id: drawtext.c,v 1.2 2003/07/21 23:06:47 chikama Exp $ */

#include "config.h"

#include <stdio.h>

#include "portab.h"
#include "system.h"
#include "nact.h"
#include "ags.h"
#include "font.h"
#include "surface.h"
#include "ngraph.h"

static int ftype;  // フォントの種類
static int fsize;  // フォントの大きさ

/**
 * 次に描く文字のフォントの種類と大きさを設定
 * 
 * @param type: フォント種類 (FONT_MINCHO, FONT_GOTHIC)
 * @param size: フォントサイズ
 */
int dt_setfont(int type, int size) {
	ftype = type;
	fsize = size;
	return OK;
}

/**
 * surface にモノクロ文字を描画。アンチエイリアスされており256階調
 * 書き出すsurfaceは8ビットである必要あり
 *
 * @param sf: 描画する surface
 * @param x: 描画位置Ｘ座標
 * @param y: 描画位置Ｙ座標
 * @param buf: 描画文字列 (SJIS)
 * @return: 実際に描画した幅
*/
int dt_drawtext(surface_t *sf, int x, int y, char *buf) {
	agsurface_t *glyph;
	int sx, sy, sw, sh;
	FONT *font = nact->ags.font;
	
	font->sel_font(ftype, fsize);
	
	glyph = font->get_glyph(buf);
	if (glyph == NULL) return 0;
	
	sx = x;	sy = y;
	sw = glyph->width;
	sh = glyph->height;
	if (!gr_clip_xywh(sf, &sx, &sy, &sw, &sh)) return 0;
	
	gr_copy(sf, sx, sy, glyph, 0, 0, sw, sh);
	
	return sw;
}

/**
 * surface にカラー文字を描画。alphamap に256階調のアンチエイリアスされた
 * 文字を描き、pixelmap には、色情報を矩形で描く
 * 
 * @param sf: 描画する surface
 * @param x: 描画位置Ｘ座標
 * @param y: 描画位置Ｙ座標
 * @param buf: 描画文字列(SJIS)
 * @param r: 文字色赤
 * @param g: 文字色緑
 * @param b: 文字色青
 * @return: 実際に描画した幅
 */ 
int dt_drawtext_col(surface_t *sf, int x, int y, char *buf, int r, int g, int b) {
	agsurface_t *glyph;
	int sx, sy, sw, sh;
	FONT *font = nact->ags.font;
	
	font->sel_font(ftype, fsize);
	
	glyph = font->get_glyph(buf);
	if (glyph == NULL) return 0;

	sx = x;	sy = y;
	sw = glyph->width;
	sh = glyph->height;
	if (!gr_clip_xywh(sf, &sx, &sy, &sw, &sh)) return 0;
	
	// alpha map に文字そのものを描く
	gr_draw_amap(sf, sx, sy, glyph->pixel, sw, sh, glyph->bytes_per_line);
	
	// pixel map には色情報を矩形で描く
	gr_fill(sf, sx, sy, sw, sh, r, g, b);
	
	return sw;
}
