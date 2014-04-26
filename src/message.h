/*
 * message.h  文字列表示関係
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
/* $Id: message.h,v 1.22 2002/05/07 21:43:53 chikama Exp $ */

#ifndef __MESSAGE__
#define __MESSAGE__

#include "portab.h"
#include "windowframe.h"
#include "graphics.h"

extern void msg_init();
extern void msg_setFontSize(int size);
extern void msg_setStringDecorationColor(int col);
extern void msg_setStringDecorationType(int type);
extern void msg_putMessage(char *msg);
extern void msg_nextLine();
extern void msg_nextPage(boolean clear);
extern void msg_hitAnyKey();
extern void msg_openWindow(int W, int C1, int C2, int N, int M);
extern void msg_setMessageLocation(int x, int y);
extern void msg_getMessageLocation(MyPoint *loc);
extern void msg_mg6_command(int cmd);

struct __message {
	/* メッセージフォントの大きさ */
	int MsgFontSize;
	int MsgFontBoldSize;
	int MsgFont;
	
	/* 各種色 */
	int MsgFontColor;
	int WinFrameColor;
	int WinBackgroundColor;
	int HitAnyKeyMsgColor;
	int WinBackgroundTransparentColor;
	
	boolean AutoPageChange;
	int     LineIncrement;
	int     WinBackgroundTransparent;
	
	/* MG command関連 */
	boolean mg_getString;
	boolean mg_dspMsg;
	int     mg_startStrVarNo;
	int     mg_curStrVarNo;
	int     mg_policyR;
	int     mg_policyA;
	
	/* メッセージ window */
	int              winno;  
	Bcom_WindowInfo *win;  // 現在使用中の window
	Bcom_WindowInfo  wininfo[MSGWINMAX];
};
typedef struct __message msg_t;

#endif /* __MESSAGE__ */
