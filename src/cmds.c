/*
 * cmds.c  SYSTEM35 S command
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
/* $Id: cmds.c,v 1.33 2002/12/31 04:11:19 chikama Exp $ */

#include <stdio.h>
#include "portab.h"
#include "xsystem35.h"
#include "scenario.h"
#include "music.h"
#include "hacks.h"

/* ぱにょ〜ん 異常シナリオ対策 */
static bool dummy_pcm_in_play = false;
/* 次の cdrom の loop 回数 */ 
static int next_cdrom_loopcnt = 0;

void commandSS() {
	/* 音楽演奏を開始する（ＣＤのみ）*/
	int num = getCaliValue();
	
	TRACE("SS %d:",num);
	
	if (num == 0) {
		muscd_stop();
	} else {
		muscd_start(num + 1, next_cdrom_loopcnt);
	}
	
	next_cdrom_loopcnt = 0;
}

void commandSC() {
	/* ＣＤのプレイ中のタイムを取得する */
	int *var = getCaliVariable();
	int t, m, s, f;
	
	if (muscd_getpos(&t, &m, &s, &f)) {
		*var++ = t - 1;
		*var++ = m;
		*var++ = s;
		*var++ = f;
	} else {
		*var++ = 999;
		*var++ = 999;
		*var++ = 999;
		*var++ = 999;
	}
	
	TRACE("SC %p:",var);
}

void commandSD() {
	/* 音楽演奏を開始する（デフォルトはＣＤ−ＤＡ）*/
	int num1 = getCaliValue();
	int num2 = getCaliValue();
	
	TRACE_UNIMPLEMENTED("SD %d,%d:",num1,num2);
}

void commandSR() {
	/* 音楽演奏情報を返す (CD,MIDI)
	   num   = デバイスの指定 (0=CD , 1=MIDI)
	   var   = 演奏曲番号 (0の時は演奏停止中)
	   var+1 = ループ回数
	   var+2 = 演奏位置
	   var+3 = フェードイン／アウト中
	*/
	/* Hushaby から SR,c,P:に変更か？ */
	int num, *var;
	int c = sl_getcAt(sl_getIndex());
	if (c < 0x40) {
		num = sl_getc();
	} else {
		num  = getCaliValue();
	}
	var = getCaliVariable();
	
	if (num == 0) {
		int t, m, s, f;
		if (muscd_getpos(&t, &m, &s, &f)) {
			// System3.5 returns the music number (track_no - 1),
			// while System3.6 and later return the track number.
			if (!memcmp(sl_sco, "S350", 4))
				*var = t - 1;
			else
				*var = t;
		} else {
			*var = 0;
		}
	} else {
		midiplaystate st;
		mus_midi_get_playposition(&st);
		*var = st.play_no;
	}
	
	TRACE("SR %d,%p:",num, var);
}

void commandSL() {
	/* 次の音楽のループ回数を指定する */
	int num = getCaliValue();
	
	next_cdrom_loopcnt = num;
	
	TRACE("SL %d:",num);
}

void commandSI() {
	/* 指定した音源の接続状態を var に取得 */
	int type = sl_getc();
	int *var = getCaliVariable();
	
	if (type == 0) {        /* MIDI */
		*var = mus_midi_get_state() ? 1 : 0;
	} else if (type == 1) { /* PCM */
		*var = mus_pcm_get_state() ? 1 : 0;
	} else if (type == 2) { /* CD */
		*var = muscd_is_available() ? 1 : 0;
	}
	
	TRACE("SI %d,%d:",type,*var);
}

void commandSG() {
	/* MIDI演奏 */
	static int loopcnt = 0;
	int sw  = sl_getc();
	int num, fnum, *var;
	midiplaystate st;
	
	switch(sw) {
	case 0:
		/* 演奏中のＭＩＤＩを停止する */
		num = getCaliValue();
		mus_midi_stop();
		TRACE("SG0 %d:", num);
		break;
	case 1:
		/* ＭＩＤＩを演奏する */
		num = getCaliValue();
		if (num == 0) {
			mus_midi_stop();
		} else {
			mus_midi_start(num, loopcnt);
		}
		TRACE("SG1 %d:", num);
		break;
	case 2:
		/* ＭＩＤＩ演奏位置を1/100秒単位で取得する */
		var = getCaliVariable();
		mus_midi_get_playposition(&st);
		*var = st.loc_ms / 10;
		TRACE("SG2 %p:", var);
		break;
	case 3:
		num = getCaliValue();
		if (num == 0) {
			/* 演奏中のＭＩＤＩを一時停止する */
			mus_midi_pause();
		} else {
			/* 一時停止中のＭＩＤＩの一時停止を解除する */
			mus_midi_unpause();
		}
		TRACE("SG3 %d:", num);
		break;
	case 4:
		num = getCaliValue();
		/* 次のSG1コマンドでのMIDI演奏の繰り返し回数指定 */
		loopcnt = num;
		TRACE("SG4 %d:", num);
		break;
	case 5:
		fnum = getCaliValue() & 0x7f;
		num  = getCaliValue();
		mus_midi_set_flag(0, fnum, num);
		
		TRACE("SG5 %d,%d:", fnum, num);
		break;
	case 6:
		fnum = getCaliValue() & 0x7f;
		num  = getCaliValue();
		mus_midi_set_flag(1, fnum, num);
		
		TRACE("SG6 %d,%d:", fnum, num);
		break;
	case 7:
		fnum = getCaliValue() & 0x7f;
		var  = getCaliVariable();
		*var = mus_midi_get_flag(0, fnum);
		
		TRACE("SG7 %d,%d:", fnum, *var);
		break;
	case 8: {
		fnum = getCaliValue() & 0x7f;
		var  = getCaliVariable();
		*var = mus_midi_get_flag(1, fnum);
		TRACE("SG8 %d,%p:", fnum, var);
		break;
	}
	default:
		SYSERROR("Unknown SG command %d", sw);
		break;
	}
}

void commandSP() {
	/* ＰＣＭデータを演奏する */
	int no = getCaliValue();
	int loop = getCaliValue();

	TRACE("SP %d,%d:",no,loop);
	
	if (!mus_pcm_get_state()) {
		dummy_pcm_in_play = true;
	}
	
	/* ???? */
	if (no == 0) {
		mus_pcm_stop(0);
	} else {
		mus_pcm_start(no, loop);
	}
}

void commandST() {
	/* ＰＣＭデータの演奏を停止する。 */
	int time = getCaliValue();
	
	TRACE("ST %d:",time);
	
	if (!mus_pcm_get_state()) { 
		dummy_pcm_in_play = false;
	}
	mus_pcm_stop(time);
}

void commandSU() {
	/* ＰＣＭの演奏状態を変数 var1 , var2 に返す */
	int *var1 = getCaliVariable();
	int *var2 = getCaliVariable();

	if (!mus_pcm_get_state()) {
		*var1 = dummy_pcm_in_play ? 1 : 0;
		*var2 = 0;
		if (dummy_pcm_in_play) dummy_pcm_in_play = false;
	} else {
		*var2 = 0;
		mus_pcm_get_playposition(var2);
		*var1 = *var2 ? 1 : 0;
		/* XXX for panyon_new */
		if (*var2 == 0){
			*var1 = dummy_pcm_in_play;
			dummy_pcm_in_play = !dummy_pcm_in_play;
		}
	}
	/* 闘神都市II 異常シナリオ対策 */
	if (game_id == GAME_TT2) {
		*var1 = *var2 = 0;
	}
	
	TRACE("SU %d,%d:",*var1, *var2);
}

void commandSV() {
	// Set volume (Rance4 v2)
	int device = getCaliValue();
	int volume = getCaliValue();
	mus_mixer_set_level(device, volume);

	TRACE("SV %d,%d", device, volume);
}

void commandSQ() {
	/* 左右別々のＰＣＭデータを合成して演奏する */
	int noL  = getCaliValue();
	int noR  = getCaliValue();
	int loop = getCaliValue();
	
	TRACE("SQ %d,%d,%d:", noL, noR, loop);
		     
	if (!mus_pcm_get_state()) {
		dummy_pcm_in_play = true;
	}
	
	if( noL<1 || noR<1 ) {
		mus_pcm_stop(0);
		return;
	}
	mus_pcm_mix(noL, noR, loop);
	return;
}

void commandSO() {
	// ＰＣＭデバイスのサポート情報を取得
	int *var = getCaliVariable();
	
	TRACE_UNIMPLEMENTED("SO %p:",var);
}

void commandSW() {
	/* 指定データ形式が演奏出来るかチェックする．*/
	int *var    = getCaliVariable();
	int channel = getCaliValue();
	int Srate   = getCaliValue();
	int bit     = getCaliValue();
	
	*var = mus_pcm_get_state() ? 2 : 0;
	
	TRACE("SW %p,%d,%d,%d:",var, channel, Srate, bit);
}

void commandSM() {
	/* ＰＣＭデータをメモリ上に乗せる。*/
	int no = getCaliValue();
	TRACE("SM %d:",no);
	
	mus_pcm_load(no);
}

void commandSX() {
	int device = sl_getc();
	int sw     = sl_getc();

	switch(sw) {
	case 1: {
		/* フェード */
		int time   = getCaliValue();
		int volume = getCaliValue();
		int stop   = getCaliValue();
		mus_mixer_fadeout_start(device, time, volume, stop);
		TRACE("SX %d,%d,%d,%d,%d:", device, sw, time, volume, stop);
		break;
	}
	case 2: {
		/* フェード終了確認 */
		int *var   = getCaliVariable();
		bool st;
		st = mus_mixer_fadeout_get_state(device);
		*var = (st ? 0 : 1);
		TRACE("SX %d,%d:", device, sw);
		break;
	}
	case 3: {
		/*  フェード強制終了 */
		mus_mixer_fadeout_stop(device);
		TRACE("SX %d,%d:", device, sw);
		break;
	}
	case 4: {
		/* ボリューム取得 */
		int *var   = getCaliVariable();
		*var = mus_mixer_get_level(device);
		TRACE("SX %d,%d:", device, sw);
		break;
	}
	default:
		SYSERROR("Unknown SX command");
	}
}
