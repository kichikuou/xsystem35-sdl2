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
#include "scenario.h"
#include "windowframe.h"
#include "selection.h"
#include "message.h"
#include "selection.h"

void commandB0() {
	/* メッセージウィンドウをクリアする。*/
	int num = getCaliValue();
	
	switch(num) {
	case 0:
		msg_nextPage(true); break;
	case 1:
		msg_nextPage(false); break;
	default:
		break;
	}

	TRACE("B0 %d:", num);
}

void commandB1() {
	int num = getCaliValue();
	int X1  = getCaliValue();
	int Y1  = getCaliValue();
	int X2  = getCaliValue();
	int Y2  = getCaliValue();
	int V   = getCaliValue();
	
	if (num < 0 || num >= SELWINMAX) {
		WARNING("commandB1(): Window number is out of range %d", num);
		return;
	}	
	nact->sel.wininfo[num].x = X1;
	nact->sel.wininfo[num].y = Y1;
	nact->sel.wininfo[num].width = X2;
	nact->sel.wininfo[num].height = Y2;
	nact->sel.wininfo[num].save = (V == 0) ? false : true;
	
	TRACE("B1 %d,%d,%d,%d,%d,%d:", num, X1, Y1, X2, Y2, V);
}

void commandB2() {
	int num = getCaliValue();
	int W   = getCaliValue();
	int C1  = getCaliValue();
	int C2  = getCaliValue();
	int C3  = getCaliValue();
	int dot = getCaliValue();
	
	if (num < 0 || num >= SELWINMAX) {
		WARNING("commandB2(): Window number is out of range %d", num);
		return;
	}

	nact->sel.winno = num;
	nact->sel.win = &nact->sel.wininfo[num];
	
	nact->sel.WindowFrameType = W;
	nact->sel.FrameCgNoTop = C1;
	nact->sel.FrameCgNoMid = C2;
	nact->sel.FrameCgNoBot = C3;
	nact->sel.Framedot = W == 0 ? 0 : W == 1 ? 8 : dot ;
	
	TRACE("B2 %d,%d,%d,%d,%d,%d:", num, W, C1, C2, C3, dot);
}

void commandB3() {
	int num = getCaliValue();
	int X1  = getCaliValue();
	int Y1  = getCaliValue();
	int X2  = getCaliValue();
	int Y2  = getCaliValue();
	int V   = getCaliValue();
	
	if (num < 0 || num >= MSGWINMAX) {
		WARNING("commandB3(): Window number is out of range %d", num);
		return;
	}
	nact->msg.wininfo[num].x = X1;
	nact->msg.wininfo[num].y = Y1;
	nact->msg.wininfo[num].width = X2;
	nact->msg.wininfo[num].height = Y2;
	nact->msg.wininfo[num].save = V != 0;
	
	TRACE("B3 %d,%d,%d,%d,%d,%d:", num, X1, Y1, X2, Y2, V);
}

void commandB4() {
	int num = getCaliValue();
	int W   = getCaliValue();
	int C1  = getCaliValue();
	int C2  = getCaliValue();
	int N   = getCaliValue();
	int M   = getCaliValue();
	
	if (num < 0 || num >= MSGWINMAX) {
		WARNING("commandB4(): Window number is out of range %d", num);
		return;
	}
	
	nact->msg.winno = num;
	nact->msg.win = &nact->msg.wininfo[num];
	msg_openWindow(W, C1, C2, N, M);
	
	TRACE("B4 %d,%d,%d,%d,%d,%d:", num, W, C1, C2, N, M);
}

void commandB10() {
	MyPoint p;
	int *x_var = getCaliVariable();
	int *y_var = getCaliVariable();
	
	msg_getMessageLocation(&p);
	*x_var = p.x;
	*y_var = p.y;
	TRACE("B10 %d,%d:", *x_var, *y_var);
}

void commandB11() {
	int *sel_no_var = getCaliVariable();
	int *msg_no_var = getCaliVariable();
	
	*sel_no_var = nact->sel.winno;
	*msg_no_var = nact->msg.winno;
	
	TRACE("B11 %d,%d:", *sel_no_var, *msg_no_var);
}

void commandB12() {
	int *var = getCaliVariable();
	
	*var = sel_getRegistoredElementNumber();
	TRACE("B12 %d:", *var);
}

void commandB13() {
	int *var = getCaliVariable();
	
	*var = sel_getRegistoredElementWidth(); 
	TRACE("B13 %d:", *var);
}

void commandB14() {
	int *var = getCaliVariable();
	
	*var = sel_getRegistoredElement_strlen(); 
	TRACE("B14 %d:", *var);
}

void commandB21() {
	int no = getCaliValue();
	int *x_var = getCaliVariable();
	int *y_var = getCaliVariable();
	
	*x_var = nact->sel.win->x;
	*y_var = nact->sel.win->y;
	
	TRACE("B21 %d,%d,%d:", no, *x_var, *y_var);
}

void commandB22() {
	int no = getCaliValue();
	int *x_var = getCaliVariable();
	int *y_var = getCaliVariable();
	
	*x_var = nact->sel.win->width;
	*y_var = nact->sel.win->height;
	
	TRACE("B22 %d,%d,%d:", no, *x_var, *y_var);
}

void commandB23() {
	int no = getCaliValue();
	int *x_var = getCaliVariable();
	int *y_var = getCaliVariable();
	
	*x_var = nact->msg.win->x;
	*y_var = nact->msg.win->y;
	
	TRACE("B23 %d,%d,%d:", no, *x_var, *y_var);
}

void commandB24() {
	int no = getCaliValue();
	int *x_var_size = getCaliVariable();
	int *y_var_size = getCaliVariable();
	
	*x_var_size = nact->msg.win->width;
	*y_var_size = nact->msg.win->height;
	
	TRACE("B24 %d,%d,%d:", no, *x_var_size, *y_var_size);
}

void commandB31() {
	/* 設定されてある選択肢ウィンドウの左上座標を取得する */
	int no = getCaliValue();
	int *x_var = getCaliVariable();
	int *y_var = getCaliVariable();
	
	*x_var = nact->sel.wininfo[no].x;
	*y_var = nact->sel.wininfo[no].y;
	
	TRACE("B31 %d,%d,%d:", no, *x_var, *y_var);
}

void commandB32() {
	int no = getCaliValue();
	int *x_var_size = getCaliVariable();
	int *y_var_size = getCaliVariable();
	
	*x_var_size = nact->sel.wininfo[no].width;
	*y_var_size = nact->sel.wininfo[no].height;
	
	TRACE("B32 %d,%d,%d:", no, *x_var_size, *y_var_size);
}

void commandB33() {
	int no = getCaliValue();
	int *x_var = getCaliVariable();
	int *y_var = getCaliVariable();

	*x_var = nact->msg.wininfo[no].x;
	*y_var = nact->msg.wininfo[no].y;
	TRACE("B33 %d,%d,%d:", no, *x_var, *y_var);
}

void commandB34() {
	int no = getCaliValue();
	int *x_var_size = getCaliVariable();
	int *y_var_size = getCaliVariable();

	*x_var_size = nact->msg.wininfo[no].width;
	*y_var_size = nact->msg.wininfo[no].height;
	TRACE("B34 %d,%d,%d:", no, *x_var_size, *y_var_size);
}

