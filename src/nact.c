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
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "portab.h"
#include "scenario.h"
#include "xsystem35.h"
#include "sdl_core.h"
#include "ags.h"
#include "nact.h"
#include "debugger.h"
#include "selection.h"
#include "message.h"
#include "msgskip.h"
#include "input.h"
#include "menu.h"
#include "hankaku.h"

/*

SCOでのコード   ZH 0          ZH 1
半角カナ       全角平仮名   半角カナ
全角平仮名     全角平仮名   全角平仮名
全角片仮名     全角片仮名   半角カナ

MS コマンド: 表示時の ZH に依存, MS cali,str(:) のstrは常に全角文字が格納.
MG コマンド: 表示時の ZH に依存

*/

/* defined by cmdv.c */
extern void va_animation();
/* 半角モード */
static int msg_msgHankakuMode = 0; /* 0:全角 1:半角, 2: 無変換 */

/* ゲームシステム情報 */
static NACTINFO nactprv;
NACTINFO *nact = &nactprv;

/* メッセージの半角化 */
void sys_setHankakuMode(int mode) {
	msg_msgHankakuMode = mode;
}

void sys_setCharacterEncoding(CharacterEncoding encoding) {
	nact->encoding = encoding;
}

/* 選択肢・通常メッセージ振り分け */
void sys_addMsg(const char *str) {
	const char *msg = NULL;

	switch(msg_msgHankakuMode) {
	case 0:
		msg = han2zen(str, nact->encoding); break;
	case 1:
		msg = zen2han(str, nact->encoding); break;
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
		msgskip_onMessage();
	}
	
	if (msg != str) {
		free((char *)msg);
	}
}

EMSCRIPTEN_KEEPALIVE
void nact_main() {
	nact->frame_count = 0;
	nact->cmd_count = 0;
	nact->wait_vsync = FALSE;

	int cnt = 0;
	while (!nact->is_quit) {
		nact->current_page = sl_getPage();
		nact->current_addr = sl_getIndex();
		if (dbg_trapped())
			dbg_main(0);

		exec_command();
		nact->cmd_count++;

		if (++cnt >= 10000 || nact->wait_vsync || nact->popupmenu_opened || dbg_trapped()) {
			nact->callback();  // Async in emscripten

			if (!nact->is_message_locked)
				sys_getInputInfo();

			sdl_wait_vsync();
			nact->frame_count++;
			nact->wait_vsync = FALSE;
			cnt = 0;
		}
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
	}
	if (nact->is_quit) {
		sys_exit(0);
	}
}

void nact_init() {
	nact->ags.mouse_movesw = 2;
	nact->is_quit = FALSE;
	nact->is_va_animation = FALSE;
	nact->is_cursor_animation = FALSE;
	nact->is_message_locked = FALSE;
	nact->is_msg_out = TRUE;
	nact->callback = nact_callback;
	nact->patch_ec = 1;  // TODO: revisit
}
