/*
 * cmdb.c  SYSTEM35 B command
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
/* $Id: cmdb.c,v 1.20 2002/05/07 21:43:53 chikama Exp $ */

#include <stdio.h>
#include "portab.h"
#include "xsystem35.h"
#include "windowframe.h"
#include "selection.h"
#include "message.h"
#include "selection.h"

/* 選択肢Window情報 */
Bcom_WindowInfo selWinInfo[SELWINMAX] = {{ 464, 80, 160, 160, TRUE}, 0};
/* 現在の選択肢Window番号 */
static int selWinNo;
/* MessageWindow情報 */
Bcom_WindowInfo msgWinInfo[MSGWINMAX] = {{ 8, 311, 616, 80, TRUE}, 0};
/* 現在のMessageWindow番号 */
static int msgWinNo;

void commandB0() {
	/* メッセージウィンドウをクリアする。*/
	int num = getCaliValue();
	
	switch(num) {
	case 0:
		msg_nextPage(TRUE); break;
	case 1:
		msg_nextPage(FALSE); break;
	default:
		break;
	}

	DEBUG_COMMAND("B0 %d:\n", num);
}

void commandB1() {
	int num = getCaliValue() - 1;
	int X1  = getCaliValue();
	int Y1  = getCaliValue();
	int X2  = getCaliValue();
	int Y2  = getCaliValue();
	int V   = getCaliValue();
	
	if (num < 0 || num >= SELWINMAX) {
		WARNING("commandB1(): Window number is out of range %d", num);
		return;
	}	
	selWinInfo[num].x = X1;
	selWinInfo[num].y = Y1;
	selWinInfo[num].width = X2;
	selWinInfo[num].height = Y2;
	selWinInfo[num].save = (V == 0) ? false : true;
	
	DEBUG_COMMAND("B1 %d,%d,%d,%d,%d,%d:\n", num + 1, X1, Y1, X2, Y2, V);
}

void commandB2() {
	int num = getCaliValue() - 1 ;
	int W   = getCaliValue();
	int C1  = getCaliValue();
	int C2  = getCaliValue();
	int C3  = getCaliValue();
	int dot = getCaliValue();
	
	if (num < 0 || num >= SELWINMAX) {
		WARNING("commandB2(): Window number is out of range %d", num);
		return;
	}
	nact->sel.win = &selWinInfo[num];
	
	nact->sel.WindowFrameType = W;
	nact->sel.FrameCgNoTop = C1;
	nact->sel.FrameCgNoMid = C2;
	nact->sel.FrameCgNoBot = C3;
	nact->sel.Framedot = W == 0 ? 0 : W == 1 ? 8 : dot ;
	
	selWinNo = num + 1;
	
	DEBUG_COMMAND("B2 %d,%d,%d,%d,%d,%d:\n", num + 1, W, C1, C2, C3, dot);
}

void commandB3() {
	int num = getCaliValue() - 1;
	int X1  = getCaliValue();
	int Y1  = getCaliValue();
	int X2  = getCaliValue();
	int Y2  = getCaliValue();
	int V   = getCaliValue();
	
	if (num < 0 || num >= MSGWINMAX) {
		WARNING("commandB3(): Window number is out of range %d", num);
		return;
	}
	msgWinInfo[num].x = X1;
	msgWinInfo[num].y = Y1;
	msgWinInfo[num].width = X2;
	msgWinInfo[num].height = Y2;
	msgWinInfo[num].save = (V == 0) ? FALSE : TRUE;
	
	DEBUG_COMMAND("B3 %d,%d,%d,%d,%d,%d:\n", num + 1, X1, Y1, X2, Y2, V);
}

void commandB4() {
	int num = getCaliValue() - 1;
	int W   = getCaliValue();
	int C1  = getCaliValue();
	int C2  = getCaliValue();
	int N   = getCaliValue();
	int M   = getCaliValue();
	
	if (num < 0 || num >= MSGWINMAX) {
		WARNING("commandB4(): Window number is out of range %d", num);
		num = 0;
	}
	
	msgWinNo = num + 1;
	nact->msg.win = &msgWinInfo[num];
	msg_openWindow(W, C1, C2, N, M);
	
	DEBUG_COMMAND("B4 %d,%d,%d,%d,%d,%d:\n", num + 1, W, C1, C2, N, M);
}

void commandB10() {
	MyPoint p;
	int *x_var = getCaliVariable();
	int *y_var = getCaliVariable();
	
	msg_getMessageLocation(&p);
	*x_var = p.x;
	*y_var = p.y;
	DEBUG_COMMAND("B10 %d,%d:\n", *x_var, *y_var);
}

void commandB11() {
	int *sel_no_var = getCaliVariable();
	int *msg_no_var = getCaliVariable();
	
	*sel_no_var = selWinNo;
	*msg_no_var = msgWinNo;
	
	DEBUG_COMMAND("B11 %d,%d:\n", *sel_no_var, *msg_no_var);
}

void commandB12() {
	int *var = getCaliVariable();
	
	*var = sel_getRegistoredElementNumber();
	DEBUG_COMMAND("B12 %d:\n", *var);
}

void commandB13() {
	int *var = getCaliVariable();
	
	*var = sel_getRegistoredElementWidth(); 
	DEBUG_COMMAND("B13 %d:\n", *var);
}

void commandB14() {
	int *var = getCaliVariable();
	
	*var = sel_getRegistoredElement_strlen(); 
	DEBUG_COMMAND("B14 %d:\n", *var);
}

void commandB21() {
	int no = getCaliValue();
	int *x_var = getCaliVariable();
	int *y_var = getCaliVariable();
	
	*x_var = selWinInfo[selWinNo - 1].x;
	*y_var = selWinInfo[selWinNo - 1].y;
	
	DEBUG_COMMAND("B21 %d,%d,%d:\n", no, *x_var, *y_var);
}

void commandB22() {
	int no = getCaliValue();
	int *x_var = getCaliVariable();
	int *y_var = getCaliVariable();
	
	*x_var = selWinInfo[selWinNo - 1].width;
	*y_var = selWinInfo[selWinNo - 1].height;
	
	DEBUG_COMMAND("B22 %d,%d,%d:\n", no, *x_var, *y_var);
}

void commandB23() {
	int no = getCaliValue();
	int *x_var = getCaliVariable();
	int *y_var = getCaliVariable();
	
	*x_var = msgWinInfo[msgWinNo - 1].x;
	*y_var = msgWinInfo[msgWinNo - 1].y;
	
	DEBUG_COMMAND("B23 %d,%d,%d:\n", no, *x_var, *y_var);
}

void commandB24() {
	int no = getCaliValue();
	int *x_var_size = getCaliVariable();
	int *y_var_size = getCaliVariable();
	
	*x_var_size = msgWinInfo[msgWinNo - 1].width;
	*y_var_size = msgWinInfo[msgWinNo - 1].height;
	
	DEBUG_COMMAND("B24 %d,%d,%d:\n", no, *x_var_size, *y_var_size);
}

void commandB31() {
	/* 設定されてある選択肢ウィンドウの左上座標を取得する */
	int no = getCaliValue();
	int *x_var = getCaliVariable();
	int *y_var = getCaliVariable();
	
	*x_var = selWinInfo[no - 1].x;
	*y_var = selWinInfo[no - 1].y;
	
	DEBUG_COMMAND("B31 %d,%d,%d:\n", no, *x_var, *y_var);
}

void commandB32() {
	int no = getCaliValue();
	int *x_var_size = getCaliVariable();
	int *y_var_size = getCaliVariable();
	
	*x_var_size = selWinInfo[no - 1].width;
	*y_var_size = selWinInfo[no - 1].height;
	
	DEBUG_COMMAND("B32 %d,%d,%d:\n", no, *x_var_size, *y_var_size);
}

void commandB33() {
	int no = getCaliValue();
	int *x_var = getCaliVariable();
	int *y_var = getCaliVariable();

	*x_var = msgWinInfo[no - 1].x;
	*y_var = msgWinInfo[no - 1].y;
	DEBUG_COMMAND("B33 %d,%d,%d:\n", no, *x_var, *y_var);
}

void commandB34() {
	int no = getCaliValue();
	int *x_var_size = getCaliVariable();
	int *y_var_size = getCaliVariable();

	*x_var_size = msgWinInfo[no - 1].width;
	*y_var_size = msgWinInfo[no - 1].height;
	DEBUG_COMMAND("B34 %d,%d,%d:\n", no, *x_var_size, *y_var_size);
}

