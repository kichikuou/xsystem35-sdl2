/*
 * cmdp.c  SYSTEM35 P command
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
/* $Id: cmdp.c,v 1.17 2001/07/30 22:58:40 chikama Exp $ */

#include <stdio.h>
#include "portab.h"
#include "xsystem35.h"
#include "scenario.h"
#include "ags.h"

/* absolete */
void commandPN() {
	/* num 番のＣＧを表示する時、カラーパレットを展開しないようにする。*/
	int num = getCaliValue();
	
	TRACE_UNIMPLEMENTED("PN %d:",num);
}

void commandPF() {
	int p1  = sl_getc();
	int num = getCaliValue();
	bool cancel_enabled;

	if (p1 == 2 || p1 == 3) {
		cancel_enabled = getCaliValue() != 0;
	} else {
		cancel_enabled = false;
	}
	switch(p1) {
	case 0:
		/* グラフィック画面をフェードインする（黒画面→通常画面）*/
		ags_fadeIn(num, cancel_enabled); break;
	case 2:
		ags_fadeIn(num, cancel_enabled);
		sysVar[0] = nact->waitcancel_key; break;
	case 1:
		/* グラフィック画面をフェードアウトする（通常画面→黒画面）*/
		ags_fadeOut(num, cancel_enabled); break;
	case 3:
		ags_fadeOut(num, cancel_enabled);
		sysVar[0] = nact->waitcancel_key; break;
	}
	
	TRACE("PF %d,%d,%d:", p1, num, cancel_enabled);
}

void commandPW() {
	int p1  = sl_getc();
	int num = getCaliValue();
	bool cancel_enabled;
	
	if (p1 == 2 || p1 == 3) {
		cancel_enabled = getCaliValue() != 0;
	} else {
		cancel_enabled = false;
	}
	switch(p1) {
	case 0:
		/* グラフィック画面をホワイトフェードインする（白画面→通常画面） */
		ags_whiteIn(num, cancel_enabled); break;
	case 2:
		ags_whiteIn(num, cancel_enabled);
		sysVar[0] = nact->waitcancel_key; break;
	case 1:
		/* グラフィック画面をホワイトフェードアウトする（通常画面→白画面） */
		ags_whiteOut(num, cancel_enabled); break;
	case 3:
		ags_whiteOut(num, cancel_enabled);
		sysVar[0] = nact->waitcancel_key; break;
	}
	
	TRACE("PW %d,%d,%d:", p1, num, cancel_enabled);
}

void commandPS() {
	/* カラーパレットを設定する。 */
	int Plane, Red, Green, Blue;
	Plane = getCaliValue();
	Red   = getCaliValue();
	Green = getCaliValue();
	Blue  = getCaliValue();

	ags_setPalette(Plane, Red, Green, Blue);
	ags_setPaletteToSystem(Plane, 1);
	TRACE("PS %d,%d,%d,%d:", Plane, Red, Green, Blue);
}

void commandPG() { /* T2 */
	/* パレットデータを変数列に取得する */
	int *var = getCaliVariable();
	int num1 = getCaliValue();
	int num2 = getCaliValue();
	int i;
	
	for (i = 0; i < num2; i++) {
		*var++ = nact->ags.pal[num1 + i].r;
		*var++ = nact->ags.pal[num1 + i].g;
		*var++ = nact->ags.pal[num1 + i].b;
	}
	TRACE("PG %p,%d,%d:", var, num1, num2);
}

void commandPP() { /* T2 */
	/* パレットデータを変数列から書き込み */
	int *var = getCaliVariable();
	int num1 = getCaliValue();
	int num2 = getCaliValue();
	int i;

	for (i = 0; i < num2; i++) {
		ags_setPalette(num1 + i, *var, *(var +1), *(var +2)); var+=3;
	}
	ags_setPaletteToSystem(num1, num2);
	TRACE("PP %p,%d,%d:", var, num1, num2);
}

void commandPC() {
	/* Ｇコマンドの制御（パレット取得、パレット展開、ＣＧ展開）を変更する。*/
	int num = getCaliValue();

	cg_fflg = num;
	
	TRACE("PC %d:",num);
}

void commandPD() {
	/* ＣＧ展開の明度を指定する */
	int num = getCaliValue();
	
	cg_brightness = num;
	
	TRACE("PD %d:",num);
}

void commandPT0() {
	/* 指定座標に描かれているパレット番号を取得する */
	int *var = getCaliVariable();
	int x = getCaliValue();
	int y = getCaliValue();
	PixelColor cell;
	
	ags_getPixel(x, y, &cell);
	*var = cell.index;
	
	TRACE("PT0 %p,%d,%d:", var, x, y);
}

void commandPT1() {
	/* 指定座標に描かれている色を取得する */
	int *r_var = getCaliVariable();
	int *g_var = getCaliVariable();
	int *b_var = getCaliVariable();
	int x = getCaliValue();
	int y = getCaliValue();
	PixelColor cell;
	
	ags_getPixel(x, y, &cell);
	*r_var = cell.r;
	*g_var = cell.g;
	*b_var = cell.b;

	TRACE("PT1 %p,%p,%p,%d,%d:", r_var, g_var, b_var, x, y);
}

void commandPT2() {
	/* 指定座標に描かれている色を取得する */
	int *hi_var  = getCaliVariable();
	int *low_var = getCaliVariable();
	int x = getCaliValue();
	int y = getCaliValue();
	PixelColor cell;
	int r, g, b, pic;
	
	ags_getPixel(x, y, &cell);
	r = cell.r;
	g = cell.r;
	b = cell.r;

	pic = (r & 0xf8) | ((g & 0xfc) << 3) | (b  >> 3);
	
	*hi_var  = pic >> 8;
	*low_var = pic & 0xff;
	
	TRACE("PT2 %p,%p,%d,%d:", hi_var, low_var, x, y);
}
