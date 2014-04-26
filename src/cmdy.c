/*
 * cmdy.c  SYSTEM35 Y command
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
/* $Id: cmdy.c,v 1.26 2001/04/29 23:30:25 chikama Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "portab.h"
#include "xsystem35.h"
#include "randMT.h"
#include "message.h"
#include "imput.h"

boolean Y3waitCancel = TRUE;

void commandY() {
	int i;
	unsigned int p1 = getCaliValue();
	unsigned int p2 = getCaliValue();
	
	if (p1 == 1) {
		if (p2 == 0) {
			/* メッセージ領域の初期化と、文字の表示位置を左上端にセットする */
			msg_nextPage(TRUE);
		} else if (p2 == 1) {
			/* メッセージ領域の文字の表示位置を左上端にセットする */
			msg_nextPage(FALSE);
		}
	} else if (p1 == 2) {
		/*システム変数 D01〜D20 までを初期化する */ 
		for (i = 0; i < 20; i++) {
			sysVar[i + 1] = 0;
		}
	} else if (p1 == 3) {
		if (p2 == 10000) {
			/* 以降のウェイトではキー入力を受けつけなくなる */
			Y3waitCancel = FALSE;
		} else if (p2 == 10001) {
			/* 以降のウェイトではキー入力を受けつける（初期設定）*/
			Y3waitCancel = TRUE;
		} else if (p2 == 0) {
			sysVar[0] = sys_getInputInfo(); /* thanx TOTOさん */
		} else {
			/* ＷＡＩＴ (1/60秒) × n だけウェイトをかける。*/
			if (get_skipMode()) return;
			sysVar[0] = sys_keywait(16 * p2, Y3waitCancel);
		}
	} else if (p1 == 4) {
		/* 1 〜 n までの乱数を RND に返す。*/
		if (p2 == 0 || p2 == 1) {
			sysVar[0] = p2;
		} else {
			sysVar[0] = (int)(genrand() * p2) +1;
		}
	} else {
		WARNING("Y undefined command %d\n", p1);
	}
	DEBUG_COMMAND("Y %d,%d:\n",p1,p2);
}
