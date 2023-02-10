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
	int mode = sl_getc();
	int num  = getCaliValue();
	
	switch(mode) {
	case 0:
		sl_clearStack(true);
		break;
	case 1:
		sl_dropLabelCalls(num);
		break;
	case 2:
		sl_dropPageCalls(num);
		break;
	case 3:
		sl_clearStack(false);
		break;
	}
	DEBUG_COMMAND("UC %d,%d:",mode,num);
}

void commandUD() {
	int mode = getCaliValue();
	
	switch(mode) {
	case 0:
		sl_reinit(); break;
	case 1:
		sl_retFar(); break;
	default:
		WARNING("UnKnown UD command %d", mode);
	}
	
	DEBUG_COMMAND("UD %d:",mode);
}

void commandUR() {
	/* 最後に積まれたスタックの属性をリード */
	int *var = getCaliVariable();

	struct stack_info info;
	sl_getStackInfo(&info);
	var[0] = info.top_attr;
	var[1] = info.page_calls;
	var[2] = info.label_calls;
	var[3] = info.var_pushes;
	var[4] = info.label_calls_after_page_call;
	var[5] = info.var_pushes_after_call;
	
	DEBUG_COMMAND("UR %p:",var);
}

void commandUS() {
	/* ローカル変数指定(変数 PUSH) */
	struct VarRef vref;
	getCaliArray(&vref);
	int cnt = getCaliValue();
	
	sl_pushVar(&vref, cnt);
	DEBUG_COMMAND("US %d,%d:", vref.var, cnt);
}

void commandUG() {
	/* ローカル変数指定(変数 POP) */
	struct VarRef vref;
	getCaliArray(&vref);
	int cnt = getCaliValue();
	
	sl_popVar(&vref, cnt);
	DEBUG_COMMAND("UG %p,%d:", vref.var, cnt);
}

void commandUP0() {
	/* 子プロセスを起動する */
	int no = getCaliValue();
	int mode = getCaliValue();
	
	DEBUG_COMMAND_YET("UP0 %d,%d:",no,mode);
}

void commandUP1() {
	/* 子プロセスを起動する */
	const char *str = sl_getString(':');
	int mode = getCaliValue();
	
	DEBUG_COMMAND_YET("UP1 %s,%d:",str,mode);
}

void commandUP3() {
	/* 外部プログラム起動後SYSTEM3.6終了*/
	const char *str1 = sl_getString(':');
	const char *str2 = sl_getString(':');
	
	DEBUG_COMMAND_YET("UP3 %s,%s:",str1,str2);
}
