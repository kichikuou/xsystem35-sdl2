/*
 * cmdi.c  SYSTEM35 I command
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
/* $Id: cmdi.c,v 1.30 2001/03/30 19:16:37 chikama Exp $ */

#include <stdio.h>
#include <limits.h>
#include "config.h"
#include "portab.h"
#include "xsystem35.h"
#include "scenario.h"
#include "ags.h"
#include "input.h"
#include "msgskip.h"

#define REPEAT_RATE_FAST 60 
#define REPEAT_RATE_SLOW 600

static int repeating = 0;
static int ik_key = 0;

void commandIK() {
	/* キー入力関連のコマンド */
	int num = sl_getc();
	int key;

	TRACE("IK %d:",num);
	
	/* to be fix
	 *   IK0/1 is affected by ZI
	 *   need cleanup
	 */
	switch(num) {
	case 0:
		sysVar[0] = 0;
		key = sys_getInputInfo();
		if (ik_key != key) repeating = 0;
		key = sys_keywait(INT_MAX, KEYWAIT_CANCELABLE | KEYWAIT_SKIPPABLE);
		if (msgskip_isSkipping()) break;
		
		if (repeating == 1) {
			sys_keywait(REPEAT_RATE_SLOW, KEYWAIT_NONCANCELABLE);
		}
		repeating++;
		sys_key_releasewait(key, true);
		sysVar[0] = ik_key = key;
		break;
	case 1: 
		sysVar[0] = 0;
		key = sys_getInputInfo();
		if (ik_key != key) repeating = 0;
		key = sys_keywait(INT_MAX, KEYWAIT_CANCELABLE | KEYWAIT_SKIPPABLE);
		if (msgskip_isSkipping()) break;
		
		if (repeating == 1) {
			sys_keywait(REPEAT_RATE_FAST, KEYWAIT_NONCANCELABLE);
		}
		repeating++;
		sys_key_releasewait(key, true);
		sysVar[0] = ik_key = key;
		break;
	case 2:
		sysVar[0] = sys_getMouseInfo(NULL, true);
		break;
	case 3:
		sysVar[0] = sys_getKeyInfo();
		break;
	case 4:
		sysVar[0] = sys_getJoyInfo();
		break;
	case 5:
		sysVar[0] = 0;
		break;
	case 6:
		sysVar[0] = sys_getInputInfo();
		break;
	default:
		WARNING("commandIK(): Unknown Command %d", num);
	}
}

void commandIM() {
	/* マウスカーソルの座標取得 */
	int *x_var = getCaliVariable();
	int *y_var = getCaliVariable();
	SDL_Point p;
	
	sysVar[0] = sys_getMouseInfo(&p, false);
	*x_var = p.x;
	*y_var = p.y;
	TRACE("IM %d,%d:", *x_var, *y_var);
}

void commandIC() {
	/* マウスカーソルの形状変更 */
	static int pre = 1;
	int cursor_num = getCaliValue();
	int *oldcursor  = getCaliVariable();
	*oldcursor = pre;
	pre = cursor_num;
	
	ags_setCursorType(cursor_num);
	TRACE("IC %d,%p:", cursor_num, oldcursor);
}

void commandIZ() {
        /* マウスカーソルの座標を変更する (マウスカーソルはスムーズに移動する) */
	int x = getCaliValue();
	int y = getCaliValue();
	
	ags_setCursorLocation(x, y, true, false);
	TRACE("IZ %d,%d:", x, y);
}

void commandIX() {
	/* 「次の選択肢まで進む」の状態取得 */
	int *var = getCaliVariable();
	
	*var = msgskip_isSkipping() ? 1 : 0;
	TRACE("IX %p:",var);
}

void commandIY() {
	int p1 = getCaliValue();
	
	if (p1 == 0) {
		msgskip_activate(false);
	} else if (p1 == 1) {
		msgskip_activate(true);
	} else if (p1 == 2) {
		msgskip_setFlags(MSGSKIP_STOP_ON_MENU, MSGSKIP_STOP_ON_MENU);
	} else if (p1 == 3) {
		msgskip_setFlags(0, MSGSKIP_STOP_ON_MENU);
	}
	
	TRACE("IY %d:",p1);
}

void commandIG() { /* T2 */
	int *var = getCaliVariable();
	int code = getCaliValue();
	int cnt  = getCaliValue();
	int rsv  = getCaliValue();
	int i;
	
	for (i = 0; i < cnt; i++) {
		*var = RawKeyInfo[code + i] ? 1 : 0;
		var++;
	}
	
	TRACE("IG %p,%d,%d,%d", var, code, cnt, rsv);
}

void commandIE() {
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	
	ags_loadCursor(p1, p2);
	TRACE("IE %d,%d", p1, p2);
}
