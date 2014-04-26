/*
 * cmdu.c  SYSTEM35 U command
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
/* $Id: cmdu.c,v 1.7 2000/09/10 10:39:49 chikama Exp $ */

#include <stdio.h>
#include "portab.h"
#include "xsystem35.h"
#include "scenario.h"

void commandUC() { /* 王道勇者 */
	/* ラベル・シナリオコールのスタックフレームを削除する。*/
	int mode = sys_getc();
	int num  = getCaliValue();
	
	switch(mode) {
	case 0:
		 sl_stackClear_allCall(); break;
	case 1:
		 sl_stackClear_labelCall(num); break;
	case 2:
		 sl_stackClear_pageCall(num); break;
	default:
		break;
	}
	DEBUG_COMMAND("UC %d,%d:\n",mode,num);
}

void commandUD() {
	int mode = getCaliValue();
	
	switch(mode) {
	case 0:
		sl_reinit(); break;
	case 1:
		sl_retFar2(); break;
	default:
		WARNING("UnKnown UD command %d\n", mode);
	}
	
	DEBUG_COMMAND("UD %d:\n",mode);
}

void commandUR() {
	/* 最後に積まれたスタックの属性をリード */
	int *var = getCaliVariable();
	
	DEBUG_COMMAND_YET("UR %p:\n",var);
}

void commandUS() {
	/* ローカル変数指定(変数 PUSH) */
	int *var = getCaliVariable();
	int cnt = getCaliValue();
	
	sl_pushVar(var, cnt);
	DEBUG_COMMAND("US %p,%d:\n",var,cnt);
}

void commandUG() {
	/* ローカル変数指定(変数 POP) */
	int *var = getCaliVariable();
	int cnt = getCaliValue();
	
	sl_popVar(var, cnt);
	DEBUG_COMMAND("UG %p,%d:\n",var,cnt);
}

void commandUP0() {
	/* 子プロセスを起動する */
	int no = getCaliValue();
	int mode = getCaliValue();
	
	DEBUG_COMMAND_YET("UP0 %d,%d:\n",no,mode);
}

void commandUP1() {
	/* 子プロセスを起動する */
	char *str = sys_getString(':');
	int mode = getCaliValue();
	
	DEBUG_COMMAND_YET("UP1 %s,%d:\n",str,mode);
}

void commandUP3() {
	/* 外部プログラム起動後SYSTEM3.6終了*/
	char *str1 = sys_getString(':');
	char *str2 = sys_getString(':');
	
	DEBUG_COMMAND_YET("UP3 %s,%s:\n",str1,str2);
}
