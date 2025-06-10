/*
 * cmdz.c  SYSTEM35 Z command
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
/* $Id: cmdz.c,v 1.35 2003/01/12 10:48:50 chikama Exp $ */

#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "portab.h"
#include "xsystem35.h"
#include "gfx.h"
#include "sdl_core.h"
#include "scheduler.h"
#include "ags.h"
#include "scenario.h"
#include "randMT.h"
#include "selection.h"
#include "message.h"
#include "input.h"
#include "nact.h"
#include "font.h"

static struct {
	uint32_t base;
	int divisor;
} counters[257];  // [0] for ZT 1-5, [1..256] for ZT 10-11

static void reset_counter(int num, int divisor, int offset) {
	counters[num].base = sdl_getTicks() - offset;
	counters[num].divisor = divisor;
}

static uint32_t get_counter(int num) {
	scheduler_on_event(SCHEDULER_EVENT_TIMER_CHECK);
	return sdl_getTicks() - counters[num].base;
}

void commandZC() {
	/* システムの使用環境を変更する */
	int m = getCaliValue();
	int n = getCaliValue();
	
	switch(m) {
	case 0: cg_vspPB               = n; break;
	case 1: nact->msg.MsgFontColor       = n; break;
	case 2: nact->sel.MsgFontColor       = n; break;
	case 3: nact->sel.WinFrameColor      = n; break;
	case 4: nact->sel.WinBackgroundColor = n; break;
	case 5: nact->msg.WinFrameColor      = n; break;
	case 6: nact->msg.WinBackgroundColor = n; break;
	case 7: nact->msg.HitAnyKeyMsgColor  = n; break;
	case 10: nact->sel.EncloseType       = n; break;
	case 11: nact->sel.SelectedElementColor     = n; break;
	case 13: nact->msg.WinBackgroundTransparent = n; break;
	case 14: nact->sel.WinBackgroundTransparent = n; break;
	case 15: sel_setDefaultElement(n); break;
	default:
		WARNING("commandZC(): Unknown Command (%d)", m); break;
	}
	
	TRACE("ZC %d,%d:",m,n);
}

void commandZM() {
	/* シナリオメッセージのフォントサイズを指定する。*/
	int size = getCaliValue();

	if (size > 100) {
		WARNING("msg font size force to 100 from %d", size);
		size = 100;
	}
	
	nact->msg.MsgFontSize = size;

	TRACE("ZM %d:",size);
}

void commandZS() {
	/* 選択肢のフォントサイズを指定する */
	int size = getCaliValue();
	
	nact->sel.MsgFontSize = size;
	
	TRACE("ZS %d:",size);
}

void commandZB() {
	/* メッセージ文字を太さを設定 */
	int weight = getCaliValue();

	if (nact->ags.enable_zb)
		nact->ags.font_weight = weight;

	TRACE("ZB %d:", weight);
}

void commandZH() {
	/* 全角半角切替え */
	int sw = getCaliValue();
	
	sys_setHankakuMode(sw);
	
	TRACE("ZH %d:",sw);
}

void commandZW() {
	/* CAPS 状態の内部的制御を変更する。 */
	int sw = getCaliValue();
	
	if (sw < 256) {
		// System 3.5 has a bug in which ZW 1 does not disable message waiting,
		// and Rance 2 depends on it.
		if (game_id == GAME_RANCE2)
			sw = 2;
		nact->messagewait_enable = ((sw & 0xff) > 1);
		nact->messagewait_cancelled = false;
	} else {
		nact->messagewait_time = sw & 0xff;
		nact->messagewait_cancel = !!(sw & 0x200);
	}
	
	TRACE("ZW %d:", sw);
}

void commandZL() {
	/* メッセージ領域の文字の縦方向行間ドット数を指定する。*/
	int line = getCaliValue();
	
	nact->msg.LineIncrement = line;
	
	TRACE("ZL %d:",line);
}

void commandZE() {
	/* 選択肢を選んだらメッセージ領域を初期化するかどうかを指定する */
	int sw = getCaliValue();
	
	nact->sel.ClearMsgWindow = sw != 0;
	
	TRACE("ZE %d:", sw);
}

void commandZF() {
	/* 選択肢枠サイズを可変にするか固定にするかを指定 */
	int sw = getCaliValue();
	
	switch(sw) {
	case 0:
		nact->sel.WinResizeHeight = true;  break;
	case 1:
		nact->sel.WinResizeHeight = false; break;
	case 2:
		nact->sel.WinResizeWidth  = false; break;
	case 3:
		nact->sel.WinResizeWidth =  true;  break;
	}
	
	TRACE("ZF %d:", sw);
}

void commandZD() {
	int c0 = sl_getc();
	int sw = 0, *var;
	char *msg;
	
	switch(c0) {
	case 0:
	case 1:
		sw = getCaliValue();
		break;
	case 2:
		sw = getCaliValue();
		msg = toUTF8(svar_get(sw));
		sys_message(2, "[DEBUG] %s\n", msg);
		free(msg);
		break;
	case 3:
		sw = getCaliValue();
		sys_message(2, "[DEBUG] %d\n", sw);
		break;
	case 4:
		var = getCaliVariable(); *var = 0; break;
	}
	
	TRACE("ZD %d,%d:", c0, sw);
}

void commandZT0() {
	/* 現在の日時を var0〜var6 の変数列に返す。*/
	time_t    time_now = time(NULL);
	struct tm *lc;
	int       sv = sl_getIndex();
	int       c1, c2;
	int       *var;

	/* ZT 0,1対策 for DALK */
	c1 = sl_getc();
	c2 = sl_getc();
	if (c1 == 0x41 && c2 == 0x7f) {
		reset_counter(0, 1, 0);
		return;
	} else {
		sl_jmpNear(sv);
		var = getCaliVariable();
	}

	lc = localtime(&time_now);
	*var       = 1900 + lc->tm_year;
	*(var + 1) = 1    + lc->tm_mon;
	*(var + 2) =        lc->tm_mday;
	*(var + 3) =        lc->tm_hour;
	*(var + 4) =        lc->tm_min;
	*(var + 5) =        lc->tm_sec;
	*(var + 6) = 1    + lc->tm_wday;
	
	TRACE("ZT0 %p:", var);
}

void commandZT1() {
	/* タイマーを n の数値でクリアする */
	int n = getCaliValue();
	
	reset_counter(0, 1, n);
	
	TRACE("ZT1 %d:", n);
}

void commandZT2() {
	/* タイマーを var に取得する 1/10 sec */
	int *var = getCaliVariable();
	int val = get_counter(0) / 100;
	
	*var = val & 0xffff;
	
	TRACE("ZT2 %p:", var);
}

void commandZT3() {
	/* タイマーを var に取得する 1/30 sec */
	int *var = getCaliVariable();
	int val = get_counter(0) * 3 / 100;
	
	*var = val & 0xffff;
	
	TRACE("ZT3 %p:", var);
}

void commandZT4() {
	/* タイマーを var に取得する 1/60 sec */
	int *var = getCaliVariable();
	int val = get_counter(0) * 3 / 50;
	
	*var = val & 0xffff;
	
	TRACE("ZT4 %p:", var);
}

void commandZT5() {
	/* タイマーを var に取得する */
	int *var = getCaliVariable();
	int val = get_counter(0) / 10;
	
	*var = val & 0xffff;
	
	TRACE("ZT5 %d:", val);
}

void commandZT10() {
	/* 高精度タイマー 設定 */
	int num   = getCaliValue();
	int base  = getCaliValue();
	int count = getCaliValue();
	
	if (num > 256) {
		WARNING("invalid timer id %d", num);
	} else if (num == 0) {
		for (int i = 1; i <= 256; i++)
			reset_counter(i, base, count);
	} else {
		reset_counter(num, base, count);
	}
	
	TRACE("ZT10 %d,%d,%d:", num, base, count);
}

void commandZT11() {
	/* 高精度タイマー取得 */
	int num  = getCaliValue();
	int *var = getCaliVariable();

	int divisor = counters[num].divisor ? counters[num].divisor : 1;
	if (num > 256)
		WARNING("invalid timer id %d", num);
	else
		*var = (get_counter(num) / divisor) & 0xffff;

	TRACE("ZT11 %d,%d:", num, *var);
}

void commandZT20() {
	/* ??? wait? */
	int p1  = getCaliValue();
	
	sysVar[0] = sys_keywait(p1, KEYWAIT_NONCANCELABLE);
	TRACE("ZT20 %d:",p1);
}

void commandZT21() {
	/* ??? wait? */
	int p1  = getCaliValue();
	
	sysVar[0] = sys_keywait(p1, KEYWAIT_CANCELABLE);
	
	TRACE("ZT21 %d:",p1);
}

void commandZZ0() {
	/* ＳＹＳＴＥＭ３．５を終了する */
	int sw = getCaliValue();
	
	TRACE("ZZ0 %d:",sw);
	
	if (sw == 0) {
#ifdef __EMSCRIPTEN__
		nact_quit(true);
#else
		sys_exit(sysVar[0]);
#endif
	} else if (sw == 1) {
		while (!nact->is_quit)
			sys_keywait(1000, 0);
	}
}

void commandZZ1() {
	/* 現在の動作機種コードを var に返す */
	int *var = getCaliVariable();
#if 1
	*var = 1;
#else
#if   defined(linux)
	*var = 5;
#elif defined(__FreeBSD__)
	*var = 6;
#endif
#endif
	TRACE("ZZ1 %p:",var);
}

void commandZZ2() {
	/* 機種文字列を文字列領域 num に返す（ＭＡＸ１２文字）*/
	int num = getCaliValue();
#if defined(linux)
	static uint8_t str[] = {0x82, 0x6B, 0x82, 0x89, 0x82, 0x8E, 0x82, 0x95, 0x82, 0x98, 0};
#elif defined(__FreeBSD__) 
	static uint8_t str[] = {0x82, 0x65, 0x82, 0x92, 0x82, 0x85, 0x82, 0x85, 0x82, 0x61, 0x82, 0x72, 0x82, 0x63, 0};
#elif defined(__OpenBSD__)
	static uint8_t str[] = {0x82, 0x6E, 0x82, 0x90, 0x82, 0x85, 0x82, 0x8E, 0x82, 0x61, 0x82, 0x72, 0x82, 0x63, 0};
#elif defined(__NetBSD__)
	static uint8_t str[] = {0x82, 0x6D, 0x82, 0x85, 0x82, 0x94, 0x82, 0x61, 0x82, 0x72, 0x82, 0x63, 0};
#elif defined(sun)
	static uint8_t str[] = {0x82, 0x72, 0x82, 0x95, 0x82, 0x8E, 0x82, 0x6E, 0x82, 0x72, 0};
#elif defined(__osf__)
	static uint8_t str[] = {0x82, 0x63, 0x82, 0x89, 0x82, 0x87, 0x82, 0x89, 0x82, 0x94, 0x82, 0x81, 0x82, 0x8C, 0x82, 0x74, 0x82, 0x6D, 0x82, 0x68, 0x82, 0x77, 0};
#elif defined(sgi)
	static uint8_t str[] = {0x82, 0x68, 0x82, 0x71, 0x82, 0x68, 0x82, 0x77, 0};
#else
	static uint8_t str[] = {0x82, 0x74, 0x82, 0x8e, 0x82, 0x8b, 0x82, 0x8e, 0x82, 0x8f, 0x82, 0x97, 0x82, 0x8e, 0};
#endif
	svar_set(num, str);

	TRACE("ZZ2 %d:",num);
}

void commandZZ3() {
	/* WINDOWSの全画面サイズや表示色数を変数列に返す */
	int *var = getCaliVariable();
	DispInfo info;
	
	ags_getWindowInfo(&info);
	*var = info.width;
	*(var + 1) = info.height;
	*(var + 2) = info.depth;
	
	TRACE("ZZ3 %p:",var);
}

void commandZZ4() {
	/* ＤＩＢ の全画面 サイズや色数を変数列に返す */
	int *var = getCaliVariable();
	DispInfo info;
	
	ags_getDIBInfo(&info);
	*var = info.width;
	*(var + 1) = info.height;
	*(var + 2) = info.depth;
	
	TRACE("ZZ4 %p:",var);
}

void commandZZ5() {
	/* ＳＹＳＴＥＭ３．５用表示画面 の サイズや色数を変数列に返す */
	int *var = getCaliVariable();
	DispInfo info;

	ags_getViewAreaInfo(&info);
	*var = info.width;
	*(var + 1) = info.height;
	*(var + 2) = info.depth;
	
	TRACE("ZZ5 %p:",var);
}

void commandZZ7() {
	// セーブドライブの残りディスク容量を得る
	int *var = getCaliVariable();
	
	*var = 65535;
	
	TRACE("ZZ7 %p:",var);
}

void commandZZ8() {
	// メモリオンバッファの残り容量を得る
	int *var = getCaliVariable();
	
	TRACE_UNIMPLEMENTED("ZZ8 %p:",var);
}

void commandZZ9() {
	/* 起動時のスクリーンサイズを取得する */
	int *var = getCaliVariable();
	DispInfo info;
	
	ags_getWindowInfo(&info);
	*var = info.width;
	*(var + 1) = info.height;
	*(var + 2) = info.depth;
	
	TRACE("ZZ9 %p:",var);
}

void commandZZ10() {
	/* スクリーンモードを取得する */
	int *var = getCaliVariable();

	*var = gfx_isFullscreen() ? 1 : 0;
	
	TRACE("ZZ10 %d:",*var);
}

void commandZZ13() {
	/* 表示フォントを設定する */
	int num = getCaliValue();
#ifdef __EMSCRIPTEN__
	if (num == FONT_MINCHO) {
		if (!load_mincho_font())
			num = FONT_GOTHIC;
	}
#endif
	if (num < FONTTYPEMAX)
		nact->ags.font_type = num;
	
	TRACE("ZZ13 %d:",num);
}

void commandZZ14() {
	int no = getCaliValue();
	char *s = "xsys35_user";

#ifdef HAVE_GETLOGIN
	char *lname=getlogin();
	if (lname)
		s = lname;
#endif
	
	if (no <= 0) return;
	svar_set(no, s);
	
	TRACE("ZZ14 %d:", no);
}

void commandZG() {
	/* ＣＧのロードした回数をリンク番号毎に配列に書き込む配列を設定 */
	int *var = getCaliVariable();
	
	cg_loadCountVar = var;
	
	TRACE("ZG %p:",var);
}

void commandZI() { /* T2 */
	/* Ａコマンドのキー入力待ち時の各キーの動作の指定 */
	int key  = getCaliValue();
	int mode = getCaliValue();
	
	set_hak_keymode(key, mode);
	
	TRACE("ZI %d,%d:", key, mode);
}

void commandZA() { /* T2 */
	/* 文字飾りの種類を指定する */
	int p1 = sl_getc();
	int p2 = getCaliValue();
	
	switch(p1) {
	case 0:
		ags_setTextDecorationType(p2); break;
	case 1:
		ags_setTextDecorationColor(p2); break;
	case 2:
	case 3:
		nact->msg.AutoPageChange = p2 != 0; break;
	default:
		WARNING("Unknown ZA comannd %d, %d", p1, p2);
	}
	
	TRACE("ZA %d,%d:", p1, p2);
}

void commandZK() {
	// ディスクの入れ替えを促す
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	const char *str = sl_getString(':');
	
	TRACE("ZK %d,%d,%s:", p1, p2, str);
}

void commandZR() {
	/* 1 〜 num までの乱数を発生して変数に返す。 */
	int num  = getCaliValue();
	int *var = getCaliVariable();
	
	if (num == 0 || num == 1) {
		*var = num;
	} else {
		*var = (int)(genrand() * num) +1;
	}
	
	TRACE("ZR %d,%d:", num, *var);
}

void commandZU() {
	/* xsystem35 extension: sets unicode mode */
	int sw = getCaliValue();

	if (sw <= CHARACTER_ENCODING_MAX)
		sys_setCharacterEncoding(sw);

	TRACE("ZU %d:",sw);
}

void cmdz_reset(void) {
	memset(counters, 0, sizeof(counters));
}
