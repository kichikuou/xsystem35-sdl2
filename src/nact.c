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
#include "gfx.h"
#include "scheduler.h"
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
		TRACE_MESSAGE("%s", msg);
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
	while (!nact->is_quit) {
		nact->current_page = sl_getPage();
		nact->current_addr = sl_getIndex();
		if (dbg_trapped()) {
			gfx_updateScreen();
			dbg_main(0);
		}

		exec_command();
		scheduler_on_command();

		if (is_yield_requested() || nact->popupmenu_opened || dbg_trapped()) {
			nact->callback();  // Async in emscripten
			sys_getInputInfo();
			scheduler_yield();
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
	if (nact->is_quit && !nact->restart) {
		sys_exit(0);
	}
}

void nact_init() {
	nact->is_quit = false;
	nact->restart = false;
	nact->callback = nact_callback;
	nact->is_va_animation = false;
	nact->is_cursor_animation = false;
	nact->encoding = SHIFT_JIS;

	free(nact->game_title_utf8);
	nact->game_title_utf8 = NULL;
	nact->scenario_version = 0;

	nact->datatbl_addr = NULL;
	nact->fnc_return_value = 0;

	nact->messagewait_enable = false;
	nact->messagewait_cancelled = false;
	nact->messagewait_time = 0;
	nact->messagewait_cancel = false;

	msg_init();
	nact->is_msg_out = true;
	nact->msgout = NULL;

	sel_init();

	nact->patch_ec = 1;  // TODO: revisit
	nact->patch_emen = 0;
	nact->patch_g0 = 0;

	msg_msgHankakuMode = 0;
}

void nact_reset(void) {
	nact_init();
	sl_reinit();
	v_reset();
	va_reset();
	cmdz_reset();
	cmd2F_reset();
}

void nact_quit(bool restart) {
	nact->is_quit = true;
	nact->restart = restart;
	request_yield();
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE int nact_current_page(void) { return nact->current_page; }
EMSCRIPTEN_KEEPALIVE int nact_current_addr(void) { return nact->current_addr; }
#endif
