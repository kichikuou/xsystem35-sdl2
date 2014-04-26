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
#include "ags.h"
#include "scenario.h"
#include "counter.h"
#include "randMT.h"
#include "selection.h"
#include "message.h"
#include "imput.h"
#include "nact.h"

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
	
	DEBUG_COMMAND("ZC %d,%d:\n",m,n);
}

void commandZM() {
	/* シナリオメッセージのフォントサイズを指定する。*/
	int size = getCaliValue();

	if (size > 100) {
		WARNING("msg font size force to 100 from %d\n", size);
		size = 100;
	}
	
	nact->msg.MsgFontSize = size;

	DEBUG_COMMAND("ZM %d:\n",size);
}

void commandZS() {
	/* 選択肢のフォントサイズを指定する */
	int size = getCaliValue();
	
	nact->sel.MsgFontSize = size;
	
	DEBUG_COMMAND("ZS %d:\n",size);
}

void commandZB() {
	/* メッセージ文字を太さを設定 */
	int size = getCaliValue();
	
	nact->msg.MsgFontBoldSize = size;
	
	DEBUG_COMMAND("ZB %d:\n",size);
}

void commandZH() {
	/* 全角半角切替え */
	int sw = getCaliValue();
	
	sys_setHankakuMode(sw);
	
	DEBUG_COMMAND("ZH %d:\n",sw);
}

void commandZW() {
	/* CAPS 状態の内部的制御を変更する。 */
	int sw = getCaliValue();
	
	if (sw < 256) {
		nact->messagewait_enable_save = 
		nact->messagewait_enable      = ((sw & 0xff) <= 1) ? FALSE : TRUE;
	} else {
		nact->messagewait_time   = sw & 0xff;
		nact->messagewait_cancel = (sw & 0x200) ? TRUE : FALSE;
	}
	
	DEBUG_COMMAND("ZW %d:\n", sw);
}

void commandZL() {
	/* メッセージ領域の文字の縦方向行間ドット数を指定する。*/
	int line = getCaliValue();
	
	nact->msg.LineIncrement = line;
	
	DEBUG_COMMAND("ZL %d:\n",line);
}

void commandZE() {
	/* 選択肢を選んだらメッセージ領域を初期化するかどうかを指定する */
	int sw = getCaliValue();
	
	nact->sel.ClearMsgWindow = sw == 0 ? FALSE : TRUE;
	
	DEBUG_COMMAND("ZE %d:\n", sw);
}

void commandZF() {
	/* 選択肢枠サイズを可変にするか固定にするかを指定 */
	int sw = getCaliValue();
	
	switch(sw) {
	case 0:
		nact->sel.WinResizeHeight = TRUE;  break;
	case 1:
		nact->sel.WinResizeHeight = FALSE; break;
	case 2:
		nact->sel.WinResizeWidth  = FALSE; break;
	case 3:
		nact->sel.WinResizeWidth =  TRUE;  break;
	}
	
	DEBUG_COMMAND("ZF %d:\n", sw);
}

void commandZD() {
	/* デバッグモード時のデバッグメッセージの出力 ON/OFF/PAUSE */
	int c0 = sys_getc();
	int sw = 0, *var;
	
	switch(c0) {
	case 0:
	case 1:
		sw = getCaliValue();
		break;
	case 2:
		sw = getCaliValue();
		DEBUG_MESSAGE("(ZD2)%s\n", v_str(sw -1));
		break;
	case 3:
		sw = getCaliValue(); break;
	case 4:
		var = getCaliVariable(); *var = 0; break;
	}
	
	DEBUG_COMMAND("ZD %d,%d:\n", c0, sw);
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
		reset_counter(0);
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
	
	DEBUG_COMMAND("ZT0 %p:\n", var);
}

void commandZT1() {
	/* タイマーを n の数値でクリアする */
	int n = getCaliValue();
	
	reset_counter(n);
	
	DEBUG_COMMAND("ZT1 %d:\n", n);
}

void commandZT2() {
	/* タイマーを var に取得する 1/10 sec */
	int *var = getCaliVariable();
	int val = get_counter(100);
	
	if (val > 65535) val = 65535;
	*var = val;
	
	DEBUG_COMMAND("ZT2 %p:\n", var);
}

void commandZT3() {
	/* タイマーを var に取得する 1/30 sec */
	int *var = getCaliVariable();
	int val = get_counter(33);
	
	if (val > 65535) val = 65535;
	*var = val;
	
	DEBUG_COMMAND("ZT3 %p:\n", var);
}

void commandZT4() {
	/* タイマーを var に取得する 1/60 sec */
	int *var = getCaliVariable();
	int val = get_counter(17);
	
	if (val > 65535) val = 65535;
	*var = val;
	
	DEBUG_COMMAND("ZT4 %p:\n", var);
}

void commandZT5() {
	/* タイマーを var に取得する */
	int *var = getCaliVariable();
	int val = get_counter(10);
	
	if (val > 65535) val = 65535;
	*var = val;
	
	DEBUG_COMMAND("ZT5 %d:\n", val);
}

void commandZT10() {
	/* 高精度タイマー 設定 */
	int num   = getCaliValue();
	int base  = getCaliValue();
	int count = getCaliValue();
	
	reset_counter_high(num, base ? base : 1, count);
	
	DEBUG_COMMAND("ZT10 %d,%d,%d:\n", num, base, count);
}

void commandZT11() {
	/* 高精度タイマー取得 */
	int num  = getCaliValue();
	int *var = getCaliVariable();
	int val  = get_high_counter(num);
	
	*var = val;
	
	DEBUG_COMMAND("ZT11 %d,%d,%d:\n", num, val, *var);
}

void commandZT20() {
	/* ??? wait? */
	int p1  = getCaliValue();
	
	sysVar[0] = sys_keywait(p1, FALSE);
	DEBUG_COMMAND("ZT20 %d:\n",p1);
}

void commandZT21() {
	/* ??? wait? */
	int p1  = getCaliValue();
	
	sysVar[0] =  sys_keywait(p1, TRUE);
	
	DEBUG_COMMAND("ZT21 %d:\n",p1);
}

void commandZZ0() {
	/* ＳＹＳＴＥＭ３．５を終了する */
	int sw = getCaliValue();
	
	DEBUG_COMMAND("ZZ0 %d:\n",sw);
	
	if (sw == 0) {
		sys_exit(sysVar[0]);
	} else if (sw == 1) {
		while (TRUE) {
			usleep(1000*1000);
			sys_getInputInfo();
		}
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
	DEBUG_COMMAND("ZZ1 %p:",var);
}

void commandZZ2() {
	/* 機種文字列を文字列領域 num に返す（ＭＡＸ１２文字）*/
	int num = getCaliValue();
#if defined(linux)
	static BYTE str[] = {0x82, 0x6B, 0x82, 0x89, 0x82, 0x8E, 0x82, 0x95, 0x82, 0x98, 0};
#elif defined(__FreeBSD__) 
	static BYTE str[] = {0x82, 0x65, 0x82, 0x92, 0x82, 0x85, 0x82, 0x85, 0x82, 0x61, 0x82, 0x72, 0x82, 0x63, 0};
#elif defined(__OpenBSD__)
	static BYTE str[] = {0x82, 0x6E, 0x82, 0x90, 0x82, 0x85, 0x82, 0x8E, 0x82, 0x61, 0x82, 0x72, 0x82, 0x63, 0};
#elif defined(__NetBSD__)
	static BYTE str[] = {0x82, 0x6D, 0x82, 0x85, 0x82, 0x94, 0x82, 0x61, 0x82, 0x72, 0x82, 0x63, 0};
#elif defined(sun)
	static BYTE str[] = {0x82, 0x72, 0x82, 0x95, 0x82, 0x8E, 0x82, 0x6E, 0x82, 0x72, 0};
#elif defined(__osf__)
	static BYTE str[] = {0x82, 0x63, 0x82, 0x89, 0x82, 0x87, 0x82, 0x89, 0x82, 0x94, 0x82, 0x81, 0x82, 0x8C, 0x82, 0x74, 0x82, 0x6D, 0x82, 0x68, 0x82, 0x77, 0};
#elif defined(sgi)
	static BYTE str[] = {0x82, 0x68, 0x82, 0x71, 0x82, 0x68, 0x82, 0x77, 0};
#else
	static BYTE str[] = {0x82, 0x74, 0x82, 0x8e, 0x82, 0x8b, 0x82, 0x8e, 0x82, 0x8f, 0x82, 0x97, 0x82, 0x8e, 0};
#endif
	v_strcpy(num -1, str);

	DEBUG_COMMAND("ZZ2 %d:\n",num);
}

void commandZZ3() {
	/* WINDOWSの全画面サイズや表示色数を変数列に返す */
	int *var = getCaliVariable();
	DispInfo info;
	
	ags_getWindowInfo(&info);
	*var = info.width;
	*(var + 1) = info.height;
	*(var + 2) = info.depth;
	
	DEBUG_COMMAND("ZZ3 %p:\n",var);
}

void commandZZ4() {
	/* ＤＩＢ の全画面 サイズや色数を変数列に返す */
	int *var = getCaliVariable();
	DispInfo info;
	
	ags_getDIBInfo(&info);
	*var = info.width;
	*(var + 1) = info.height;
	*(var + 2) = info.depth;
	
	DEBUG_COMMAND("ZZ4 %p:\n",var);
}

void commandZZ5() {
	/* ＳＹＳＴＥＭ３．５用表示画面 の サイズや色数を変数列に返す */
	int *var = getCaliVariable();
	DispInfo info;

	ags_getViewAreaInfo(&info);
	*var = info.width;
	*(var + 1) = info.height;
	*(var + 2) = info.depth;
	
	DEBUG_COMMAND("ZZ5 %p:\n",var);
}

void commandZZ7() {
	// セーブドライブの残りディスク容量を得る
	int *var = getCaliVariable();
	
	*var = 65535;
	
	DEBUG_COMMAND("ZZ7 %p:\n",var);
}

void commandZZ8() {
	// メモリオンバッファの残り容量を得る
	int *var = getCaliVariable();
	
	DEBUG_COMMAND_YET("ZZ8 %p:\n",var);
}

void commandZZ9() {
	/* 起動時のスクリーンサイズを取得する */
	int *var = getCaliVariable();
	DispInfo info;
	
	ags_getWindowInfo(&info);
	*var = info.width;
	*(var + 1) = info.height;
	*(var + 2) = info.depth;
	
	DEBUG_COMMAND("ZZ9 %p:\n",var);
}

void commandZZ10() {
	/* スクリーンモードを取得する */
	int *var = getCaliVariable();

	*var = nact->sys_fullscreen_on ? 1 : 0;
	
	DEBUG_COMMAND("ZZ10 %d:\n",*var);
}

void commandZZ13() {
	/* 表示フォントを設定する */
	int num = getCaliValue();
	
	nact->msg.MsgFont = num;
	
	DEBUG_COMMAND("ZZ13 %d:\n",num);
}

void commandZZ14() {
	int no = getCaliValue();
	char *lname=getlogin();
	
	if (no <= 0) return;
	if (lname) {
		v_strcpy(no -1, lname);
	} else {
		char s[256];
		sprintf(s, "%d", getuid());
		v_strcpy(no -1, s);
	}
	
	DEBUG_COMMAND("ZZ14 %d:\n", no);
}

void commandZG() {
	/* ＣＧのロードした回数をリンク番号毎に配列に書き込む配列を設定 */
	int *var = getCaliVariable();
	
	cg_loadCountVar = var;
	
	DEBUG_COMMAND("ZG %p:\n",var);
}

void commandZI() { /* T2 */
	/* Ａコマンドのキー入力待ち時の各キーの動作の指定 */
	int key  = getCaliValue();
	int mode = getCaliValue();
	
	set_hak_keymode(key, mode);
	
	DEBUG_COMMAND("ZI %d,%d:\n", key, mode);
}

void commandZA() { /* T2 */
	/* 文字飾りの種類を指定する */
	int p1 = sys_getc();
	int p2 = getCaliValue();
	
	switch(p1) {
	case 0:
		msg_setStringDecorationType(p2); break;
	case 1:
		msg_setStringDecorationColor(p2); break;
	case 2:
	case 3:
		nact->msg.AutoPageChange = (p2 == 0 ? FALSE : TRUE); break;
	default:
		WARNING("Unknown ZA comannd %d, %d\n", p1, p2);
	}
	
	DEBUG_COMMAND("ZA %d,%d:\n", p1, p2);
}

void commandZK() {
	// ディスクの入れ替えを促す
	int p1 = getCaliValue();
	int p2 = getCaliValue();
	char *str = sys_getString(':');
	
	DEBUG_COMMAND("ZK %d,%d,%s:\n", p1, p2, str);
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
	
	DEBUG_COMMAND("ZR %d,%d:\n", num, *var);
}
