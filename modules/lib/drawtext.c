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

static FontType ftype;
static int fsize;  // フォントの大きさ

/**
 * 次に描く文字のフォントの種類と大きさを設定
 * 
 * @param type: フォント種類 (FONT_MINCHO, FONT_GOTHIC)
 * @param size: フォントサイズ
 */
void dt_setfont(FontType type, int size) {
#ifdef __EMSCRIPTEN__
	if (type == FONT_MINCHO) {
		if (!load_mincho_font())
			type = FONT_GOTHIC;
	}
#endif

	ftype = type;
	fsize = size;
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
int dt_drawtext(SDL_Surface *sf, int x, int y, char *buf) {
	ags_setFont(ftype, fsize);
	SDL_Surface *glyph = ags_drawStringToSurface(buf);
	if (glyph == NULL) return 0;

	SDL_Rect r = {x, y, glyph->w, glyph->h};
	SDL_BlitSurface(glyph, NULL, sf, &r);
	SDL_FreeSurface(glyph);
	return glyph->w;
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
	int sx, sy, sw, sh;

	ags_setFont(ftype, fsize);
	SDL_Surface *glyph = ags_drawStringToSurface(buf);
	if (glyph == NULL) return 0;

	sx = x;	sy = y;
	sw = glyph->w;
	sh = glyph->h;
	if (!gr_clip_xywh(sf, &sx, &sy, &sw, &sh)) return 0;
	
	// alpha map に文字そのものを描く
	gr_draw_amap(sf, sx, sy, glyph->pixels, sw, sh, glyph->pitch);

	// pixel map には色情報を矩形で描く
	gr_fill(sf, sx, sy, sw, sh, r, g, b);

	SDL_FreeSurface(glyph);
	return sw;
}
