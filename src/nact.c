/*
 * nact.c シナリオデータの解析
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
/* $Id: nact.c,v 1.24 2003/11/09 15:06:13 chikama Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glib.h>

#include "portab.h"
#include "scenario.h"
#include "xsystem35.h"
#include "ags.h"
#include "counter.h"
#include "nact.h"
#include "selection.h"
#include "message.h"
#include "imput.h"
#include "menu.h"

/*

SCOでのコード   ZH 0          ZH 1
半角カナ       全角平仮名   半角カナ
全角平仮名     全角平仮名   全角平仮名
全角片仮名     全角片仮名   半角カナ

MS コマンド: 表示時の ZH に依存, MS cali,str(:) のstrは常に全角文字が格納.
MG コマンド: 表示時の ZH に依存

*/

/* defined in hankaku.c */
extern BYTE *zen2han(BYTE *src);
extern BYTE *han2zen(BYTE *src);
/* defined by hankan2sjis.c */
extern char *hankana2sjis(int index);
/* defined by cmd_check.c */
extern void check_command(int c0);
/* defined by cmdv.c */
extern void va_animation();
/* コマンド解析時の展開バッファ */
static char msgbuf[512];
/* 半角モード */
static int msg_msgHankakuMode = 0; /* 0:全角 1:半角, 2: 無変換 */

/* ゲームシステム情報 */
static NACTINFO nactprv;
NACTINFO *nact = &nactprv;

/* メッセージの半角化 */
void sys_setHankakuMode(int mode) {
	msg_msgHankakuMode = mode;
}

/* 文字列の取り出し */
char *sys_getString(char term) {
	int c0;
	char *index = msgbuf;
	
	while ((c0 = sl_getc()) != (int)term) {
		*index++ = c0;
	}
	*index = 0;
	return msgbuf;
}

/* 特殊 const string */
char *sys_getConstString() {
	int c0;
	char *index = msgbuf;

	c0 = sl_getc();
	
	while ((c0 = sl_getc()) != 0) {
		*index++ = ((c0 & 0xf0) >> 4) + ((c0 & 0x0f) << 4);
	}
	
	*index = 0;
	return msgbuf;
}

/* 加工済み(半カナ->全カナ) 文字列抽出 */
char* sys_getConvString(char term) {
	int c0;
	char *index = msgbuf;
	char *kindex;
	
	while ((c0 = sl_getc()) != (int)term) {
		if (c0 == 0x20) {
			*index++ = 0x81; *index++ = 0x40;
		} else if (c0 >= 0xe0) {
			*index++ = (char)c0; *index++ = (char)sl_getc();
		} else if (c0 >= 0xa0) {
			kindex = hankana2sjis(c0);
			*index++ = *kindex; *index++ = *(kindex+1);
		} else {
			*index++ = (char)c0; *index++ = (char)sl_getc();
		}
	}
	*index = 0;
	return msgbuf;
}

/* 選択肢・通常メッセージ振り分け */
void sys_addMsg(char *str) {
	char *msg = NULL;

	switch(msg_msgHankakuMode) {
	case 0:
		msg = han2zen(str); break;
	case 1:
		msg = zen2han(str); break;
	case 2:
		msg = str; break;
	default:
		return;
	}
	
	if (nact->sel.in_setting) {
		// 選択肢のバッファへ
		sel_addElement(msg);
	} else {
		// 通常のメッセージ
		DEBUG_MESSAGE("%s", msg);
		if (nact->is_msg_out) {
			msg_putMessage(msg);
		}
		if (nact->msgout) {
			nact->msgout(msg);
		}
	}
	
	if (msg && msg_msgHankakuMode < 2) {
		free(msg);
	}
}

/* 文字列抽出 */
static int checkMessage() {
	char *index = msgbuf;
	int c0 = sl_getc();
	
	while (c0 == 0x20 || c0 > 0x80) {
		if (c0 == 0x20) {
			*index++ = (char)c0;
		} else if (c0 >= 0xe0) {
			*index++ = (char)c0; *index++ = (char)sl_getc();
		} else if (c0 >= 0xa0) {
			*index++ = (char)c0;
		} else {
			*index++ = (char)c0; *index++ = (char)sl_getc();
		}
		c0 = sl_getc();
	}
	if (index != msgbuf) {
		*index = 0;
		sys_addMsg(msgbuf);
	}
	return c0;
}

#define MAINLOOP_EVENTCHECK_INTERVAL 16 /* 16 msec */
void nact_main() {
	int c0;
	static int cnt = 0;
	
	reset_counter_high(SYSTEMCOUNTER_MSEC, 1, 0);
	reset_counter_high(SYSTEMCOUNTER_MAINLOOP, MAINLOOP_EVENTCHECK_INTERVAL, 0);
	
	while (!nact->is_quit) {
		DEBUG_MESSAGE("%d:%x\n", sl_getPage(), sl_getIndex());
		if (!nact->popupmenu_opened) {
			c0 = checkMessage();
			check_command(c0);
		}
		if (!nact->is_message_locked) {
			if (get_high_counter(SYSTEMCOUNTER_MAINLOOP)) {
				sys_getInputInfo();
				reset_counter_high(SYSTEMCOUNTER_MAINLOOP, MAINLOOP_EVENTCHECK_INTERVAL, 0);
			}
		}
		if (cnt == 10000) {
			usleep(10); /* XXXX */
			cnt = 0;
		}
		cnt++;
		nact->callback();
	}
}

static void nact_callback() {
	if (nact->is_va_animation) {
		va_animation();
	}
	if (nact->is_cursor_animation) {
		/* cursor animation */
	}
	if (nact->popupmenu_opened) {
		menu_gtkmainiteration();
		if (nact->is_quit) sys_exit(0);
	}
}

void nact_init() {
	nact->sys_mouse_movesw = 2;
	nact->is_quit = FALSE;
	nact->is_va_animation = FALSE;
	nact->is_cursor_animation = FALSE;
	nact->is_message_locked = FALSE;
	nact->is_msg_out = TRUE;
	nact->callback = nact_callback;
}
