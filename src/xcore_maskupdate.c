/*
 * xcore_maskupdate.c  X11 mask 付き update
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
/* $Id: xcore_maskupdate.c,v 1.4 2002/05/02 17:21:32 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "portab.h"
#include "xcore_private.h"

static Pixmap maskpix;  /* clipmask 用 Bitmap */
static Pixmap clippix;  /* copy 元 Pixmap */
static Pixmap savepix;  /* copy 先 Pixmap */
static Pixmap doublepix;/* double buffer 用 Pixmap */
static GC     maskgc;   /* clipmask Bitmap 用 GC */

static void draw_5star(double mul, double rad, int off_x, int off_y);
static void draw_6star(double mul, double rad, int off_x, int off_y);
static void draw_windwheel_90(double rad, int r, int off_x, int off_y);
static void draw_windwheel_180(double rad, int r, int off_x, int off_y);
static void draw_windwheel_360(double rad, int r, int off_x, int off_y);

/*
 * 五芒星を描く
 *  mul  : 倍率 (1 で 100 * 100) の星が描かれる
 *  rad  : 回転角度(radian)
 *  off_x: 中心座標の x 方向 offset
 *  off_y: 中心座標の y 方向 offset
 */
static void draw_5star(double mul, double rad, int off_x, int off_y) {
	static XPoint pt[] = {
		{  0, -50},
		{-29,  40},
		{ 47, -15},
		{-47, -15},
		{ 29,  40}
	};
	XPoint p[5];
	int i;
	
	/*
	 *   x'   cos(X) -sin(X)   mul  0       x
	 *     =                 *           * 
	 *   y'   sin(X)  cos(X)   0    mul     y
	 */
	for (i = 0; i < 5; i++) {
		p[i].x = mul * (pt[i].x * cos(rad) - pt[i].y * sin(rad)) + off_x;
		p[i].y = mul * (pt[i].x * sin(rad) + pt[i].y * cos(rad)) + off_y;
	}
	
	XFillPolygon(x11_display, maskpix, maskgc, p, 5, Complex, CoordModeOrigin);
}

/*
 * 六芒星を描く
 *  mul  : 倍率 (1 で 100 * 100) の星が描かれる
 *  rad  : 回転角度(radian)
 *  off_x: 中心座標の x 方向 offset
 *  off_y: 中心座標の y 方向 offset
 */
static void draw_6star(double mul, double rad, int off_x, int off_y) {
	static XPoint pt1[] = {
		{  0, -50},
		{-43,  25},
		{ 43,  25}
	};
	static XPoint pt2[] = { 
		{-43, -25},
		{  0,  50},
		{ 43, -25}
	};
	XPoint p[3];
	int i;
	
	for (i = 0; i < 3; i++) {
		p[i].x = mul * (pt1[i].x * cos(rad) - pt1[i].y * sin(rad)) + off_x;
		p[i].y = mul * (pt1[i].x * sin(rad) + pt1[i].y * cos(rad)) + off_y;
	}

	XFillPolygon(x11_display, maskpix, maskgc, p, 3, Convex, CoordModeOrigin);
	
	for (i = 0; i < 3; i++) {
		p[i].x = mul * (pt2[i].x * cos(rad) - pt2[i].y * sin(rad)) + off_x;
		p[i].y = mul * (pt2[i].x * sin(rad) + pt2[i].y * cos(rad)) + off_y;
	}
	
	XFillPolygon(x11_display, maskpix, maskgc, p, 3, Convex, CoordModeOrigin);
}

/*
 * 扇型の中心角 (てきとー)
 *   0.2 radian
 */
#define WHEELDELTA 0.2

/*
 * 風車９０°
 *  rad  : 回転角 (radian) 0 - pi/2
 *  r    : 半径
 *  off_x: 中心座標の x 方向 offset
 *  off_y: 中心座標の y 方向 offset
 */
static void draw_windwheel_90(double rad, int r, int off_x, int off_y) {
	XPoint p[3];
	
	p[0].x = 0 + off_x;
	p[0].y = 0 + off_y;

	p[1].x =   r * sin(rad) + off_x;
	p[1].y =  -r * cos(rad) + off_y;

	p[2].x =   r * sin(rad + WHEELDELTA) + off_x;
	p[2].y =  -r * cos(rad + WHEELDELTA) + off_y;
	
	XFillPolygon(x11_display, maskpix, maskgc, p, 3, Convex, CoordModeOrigin);

	p[1].x =   r * cos(rad) + off_x;
	p[1].y =   r * sin(rad) + off_y;

	p[2].x =   r * cos(rad + WHEELDELTA) + off_x;
	p[2].y =   r * sin(rad + WHEELDELTA) + off_y;

	XFillPolygon(x11_display, maskpix, maskgc, p, 3, Convex, CoordModeOrigin);
	
	p[1].x = -r * sin(rad) + off_x;
	p[1].y =  r * cos(rad) + off_y;

	p[2].x = -r * sin(rad + WHEELDELTA) + off_x;
	p[2].y =  r * cos(rad + WHEELDELTA) + off_y;

	XFillPolygon(x11_display, maskpix, maskgc, p, 3, Convex, CoordModeOrigin);
	p[1].x = -r * cos(rad) + off_x;
	p[1].y = -r * sin(rad) + off_y;

	p[2].x = -r * cos(rad + WHEELDELTA) + off_x;
	p[2].y = -r * sin(rad + WHEELDELTA) + off_y;

	XFillPolygon(x11_display, maskpix, maskgc, p, 3, Convex, CoordModeOrigin);
}

/*
 * 風車１８０°
 *  rad  : 回転角 (radian) 0 - pi
 *  r    : 半径
 *  off_x: 中心座標の x 方向 offset
 *  off_y: 中心座標の y 方向 offset
 */
static void draw_windwheel_180(double rad, int r, int off_x, int off_y) {
	XPoint p[3];
	
	p[0].x = 0 + off_x;
	p[0].y = 0 + off_y;

	
	p[1].x = -r * cos(rad) + off_x;
	p[1].y = -r * sin(rad) + off_y;

	p[2].x = -r * cos(rad + WHEELDELTA) + off_x;
	p[2].y = -r * sin(rad + WHEELDELTA) + off_y;
	
	XFillPolygon(x11_display, maskpix, maskgc, p, 3, Convex, CoordModeOrigin);

	p[1].x =  r * cos(rad) + off_x;
	p[1].y =  r * sin(rad) + off_y;

	p[2].x =  r * cos(rad + WHEELDELTA) + off_x;
	p[2].y =  r * sin(rad + WHEELDELTA) + off_y;

	XFillPolygon(x11_display, maskpix, maskgc, p, 3, Convex, CoordModeOrigin);
}

/*
 * 風車１８０°
 *  rad  : 回転角 (radian) 0 - 2pi
 *  r    : 半径
 *  off_x: 中心座標の x 方向 offset
 *  off_y: 中心座標の y 方向 offset
 */
static void draw_windwheel_360(double rad, int r, int off_x, int off_y) {
	XPoint p[3];
	
	p[0].x = 0 + off_x;
	p[0].y = 0 + off_y;

	p[1].x =  -r * cos(rad) + off_x;
	p[1].y =  -r * sin(rad) + off_y;

	p[2].x =  -r * cos(rad + WHEELDELTA) + off_x;
	p[2].y =  -r * sin(rad + WHEELDELTA) + off_y;
	
	XFillPolygon(x11_display, maskpix, maskgc, p, 3, Convex, CoordModeOrigin);

}

/*
 * マスク付き領域更新
 *   sx: コピー元 x 座標
 *   sy: コピー元 y 座標
 *   w : コピー元 width
 *   h : コピー元 height
 *   dx: コピー先 x 座標
 *   dy: コピー先 y 座標
 *   func: マスク種類
 *      44: 五芒星(内->外)
 *      45: 五芒星(外->内)
 *      46: 六芒星(内->外)
 *      47: 六芒星(外->内)
 *      50: 風車９０°
 *      51: 風車１８０°
 *      52: 風車３６０°
 *   step: 呼び出し番号 0 - 256
 */
void Xcore_maskupdate(int sx, int sy, int w, int h, int dx, int dy, int func, int step) {
	GC gc;

	dx -= view_x;
	dy -= view_y;

	if (step == 0) {
		/* 最初にいろいろと準備 */
		XSync(x11_display, False);
		/* コピー元を DIB から pixmap に持って来る */
		clippix = x11_clip_from_DIB(sx, sy, w, h);

		/* clip mask 用 Pixmap の生成 */
		maskpix = XCreatePixmap(x11_display, x11_window, w, h, 1);
		maskgc = XCreateGC(x11_display, maskpix, None, NULL);
		
		/* clipmask 用 Pixmap の初期化 */
		XSetForeground(x11_display, maskgc, 0);
		XFillRectangle(x11_display, maskpix, maskgc, 0, 0, w, h);
		
		/* 線が交差するときの塗りつぶしのルールを設定 */
		XSetFillRule(x11_display, maskgc, WindingRule);	

		/* 更新先の Window の領域を保存 */
		savepix = XCreatePixmap(x11_display, x11_window, w, h, WIN_DEPTH);
		XCopyArea(x11_display, x11_window, savepix, x11_gc_pix,
			  winoffset_x + dx, winoffset_y + dy, w, h, 0, 0);
		
		/* ダブルバッファ用に Pixmap を確保 */
		doublepix = XCreatePixmap(x11_display, x11_window, w, h, WIN_DEPTH);
		XCopyArea(x11_display, x11_window, doublepix, x11_gc_pix,
			  winoffset_x + dx, winoffset_y + dy, w, h, 0, 0);
		
		XSync(x11_display, False);
		return;
	} else if (step == 256) {
		/* 最後に後始末 */
#if 0
		XCopyArea(x11_display, clippix, x11_window, x11_gc_pix,
			  0, 0, w, h, winoffset_x + dx, winoffset_y + dy);
#endif
#if 0
		XCopyArea(x11_display, clippix, x11_pixmap, x11_gc_pix,
			  0, 0, w, h, winoffset_x + dx, winoffset_y + dy);
		ags_updateFull();
#endif
		ags_copyArea(sx, sy, w, h, dx + view_x, dy + view_y);
		ags_updateArea(dx + view_x, dy + view_y, w, h);
		XFreePixmap(x11_display, maskpix);
		
		XFreePixmap(x11_display, clippix);
		XFreePixmap(x11_display, savepix);
		XFreePixmap(x11_display, doublepix);
		XFreeGC(x11_display, maskgc);
		
		return;
	}
	
	/* clipmask 用の GC を生成 (毎回生成しなくても良いかも？) */
	gc = XCreateGC(x11_display, clippix, None, NULL);
	XSetClipMask(x11_display, gc, maskpix);
	XSetClipOrigin(x11_display, gc, 0, 0);
	
	switch(func) {
	case 44:	
	case 46:
		/* 内から外へ */
		XSetForeground(x11_display, maskgc, 0);
		XFillRectangle(x11_display, maskpix, maskgc, 0, 0, w, h);
		XSetForeground(x11_display, maskgc, 1);
		break;
	case 45:
	case 47:
		/* 外から内へ */
		XSetForeground(x11_display, maskgc, 1);
		XFillRectangle(x11_display, maskpix, maskgc, 0, 0, w, h);
		XSetForeground(x11_display, maskgc, 0);
		break;
	default:
		XSetForeground(x11_display, maskgc, 1);
	}

	/* mask を描画 */
	switch(func) {
	case 44:
		draw_5star(1.5 * (max(w,h) * step) / (256 * 50.0),
			   1.0 * M_PI * step / 256, w/2, h/2);
		break;
		
	case 45:
		draw_5star(1.5 * (max(w,h) * (256-step)) / (256 * 50.0),
			   1.0 * M_PI * step / 256, w/2, h/2);
		break;
		
	case 46:
		draw_6star((max(w,h) * step) / (256 * 50.0),
			   1.0 * M_PI * step / 256, w/2, h/2);
		break;
		
	case 47:
		draw_6star((max(w,h) * (256-step)) / (256 * 50.0),
			   1.0 * M_PI * step / 256, w/2, h/2);
		break;
		
	case 50:
		draw_windwheel_90(0.5 * M_PI * step / 256, sqrt(w*w+h*h)/2, 
			       w/2, h/2);
		break;
	case 51:
		draw_windwheel_180(1.0 * M_PI * step / 256, sqrt(w*w+h*h)/2, 
				   w/2, h/2);
		break;
	case 52:
		draw_windwheel_360(2.0 * M_PI * step / 256, sqrt(w*w+h*h)/2, 
				   w/2, h/2);
		break;
	}
	
	/* double buffer へ 保存した window の内容をコピー */
	XCopyArea(x11_display, savepix, doublepix, x11_gc_win,
		  0, 0, w, h, 0, 0);
	
	/* コピー元の内容を mask付きで double buffer へコピー */
	XCopyArea(x11_display, clippix, doublepix, gc,
		  0, 0, w, h, 0, 0);
	
	/* double buffer から window へコピー */
	XCopyArea(x11_display, doublepix, x11_window, x11_gc_win,
		  0, 0, w, h, winoffset_x + dx, winoffset_y + dy);
	
	/* 同期 */
	XSync(x11_display, False);
	
	/* clipmask 用 gc の破棄 */
	XFreeGC(x11_display, gc);
}
