/*
 * cmdt.c  SYSTEM35 T command
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
/* $Id: cmdt.c,v 1.8 2006/04/21 16:40:48 chikama Exp $ */

#include <stdio.h>
#include "portab.h"
#include "xsystem35.h"
#include "scenario.h"
#include "message.h"

void commandT() {
	/* 文字の表示開始座標を指定する */
	int x = getCaliValue();
	int y = getCaliValue();
	msg_setMessageLocation(x, y);
	
	TRACE("T %d,%d:",x,y);
}

void commandTOC() {
	/* テキストカラーをスタックからポップして設定する */
	sl_popState(STACK_TEXTCOLOR);
	TRACE("TOC:");
}

void commandTOS() {
	/* テキストフォントサイズをスタックからポップして設定する */
	sl_popState(STACK_TEXTSIZE);
	TRACE("TOS:");
}

void commandTPC() {
	/* 現在のテキストカラーをスタックにプッシュする */
	int type = getCaliValue();
	switch (type) {
	case 0: sl_pushTextColor(type, nact->msg.MsgFontColor); break;
	case 1: sl_pushTextColor(type, nact->sel.MsgFontColor); break;
	default: WARNING("TPC: unknown type %d", type); break;
	}
	TRACE("TPC %d", exp);
}

void commandTPS() {
	/* 現在のテキストフォントサイズをスタックにプッシュする */
	int type = getCaliValue();
	switch (type) {
	case 0: sl_pushTextSize(type, nact->msg.MsgFontSize); break;
	case 1: sl_pushTextSize(type, nact->sel.MsgFontSize); break;
	default: WARNING("TPS: unknown type %d", type); break;
	}
	TRACE("TPS %d", exp);
}

void commandTOP() {
	/* テキスト表示位置をスタックからポップして設定する */
	sl_popState(STACK_TEXTLOC);
	TRACE("TOP:");
}

void commandTPP() {
	/* 現在のテキスト表示位置をスタックにプッシュする */
	SDL_Point loc;
	msg_getMessageLocation(&loc);
	sl_pushTextLoc(loc.x, loc.y);
	TRACE("TPP:");
}

void commandTAA() {
	/* アンチエイリアシング付きテキスト描画のフラグ設定 */
	int exp = getCaliValue();

	ags_setAntialiasedStringMode(exp == 1);

	TRACE("TAA %d:", exp);
}

void commandTAB() {
	/* アンチエイリアシング付きテキスト描画のフラグ取得 */
	int *var = getCaliVariable();

	*var = ags_getAntialiasedStringMode() ? 1 : 0;

	TRACE("TAB %d:", *var);
}
