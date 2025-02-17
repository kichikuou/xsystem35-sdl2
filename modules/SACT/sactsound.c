/*
 * sactsound.c: SACTの効果音関連
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
/* $Id: sactsound.c,v 1.4 2003/08/02 13:10:32 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <string.h>

#include "portab.h"
#include "system.h"
#include "nact.h"
#include "input.h"
#include "sactsound.h"
#include "music.h"
#include "sact.h"

// slot番号の 1から 20を SACT用に使用
#define CACHEMAX 20
#define SLOTOFFSET 1
static int cache[CACHEMAX];
static int cachei;

// 再生する効果音用の空いているスロットを探す
static int slt_getnext(int no) {
	int indx;
	
	indx = cachei % CACHEMAX;
	cache[indx] = no;
	cachei++;
	if (cachei == CACHEMAX) cachei = 0;
	return indx + SLOTOFFSET;
}

// 指定の番号の効果音が入っているスロットを探す
static int slt_find(int no) {
	int i;
	
	for (i = 0; i < CACHEMAX; i++) {
		if (cache[i] == no) return i + SLOTOFFSET;
	}
	return -1;
}

/*
 SACT 効果音 subsystem 初期化
*/
void ssnd_init(void) {
	// ch 1-8 をキャッシュとして使おう
	cachei = 0;
	memset(cache, 0, sizeof(cache));
}

// 指定の番号の効果音をメモリに読み込み
void ssnd_prepare(int no) {
	int slot = slt_find(no);
	
	if (slot == -1) {
		slot = slt_getnext(no);
		mus_wav_load(slot, no);
	}
}

// 指定の番号の効果音を再生
void ssnd_play(int no) {
	int slot = slt_find(no);
	
	if (slot == -1) {
		slot = slt_getnext(no);
		mus_wav_load(slot, no);
	}
	mus_wav_play(slot, 1);
}

// 指定の番号の効果音を再生停止
void ssnd_stop(int no, int fadetime) {
	int slot = slt_find(no);

	if (slot != -1) {
		mus_wav_fadeout_start(slot, fadetime, 0, true);
		cache[slot - SLOTOFFSET] = 0;
	}
}

// 指定の番号の効果音が再生終了するのを待つ
void ssnd_wait(int no) {
	int slot = slt_find(no);

	if (slot != -1) {
		mus_wav_waitend(slot);
		cache[slot - SLOTOFFSET] = 0;
	}
}

// 指定の効果音が終了するか、キーが押されるまで待つ
void ssnd_waitkey(int no, int *res) {
	int slot = slt_find(no);
	
	if (slot == -1) {
		*res = 0;
		return;
	}

	if (sact.waitskiplv > 1) {
		*res = SYS35KEY_RET;
		return;
	}

	sact.waittype = KEYWAIT_SIMPLE;
	sact.waitkey = -1;
	
	while (sact.waitkey == -1 && mus_wav_get_playposition(slot) && !nact->is_quit) {
		sys_keywait(25, KEYWAIT_CANCELABLE);
	}
	
	if (sact.waitkey == -1) {
		cache[slot - SLOTOFFSET] = 0;
		*res = 0;
	} else {
		*res = sact.waitkey;
	}
	sact.waittype = KEYWAIT_NONE;
}

// 左右チャンネルの反転した効果音をメモリに読み込む
void ssnd_prepareLRrev(int no) {
	mus_wav_load_lrsw(30, no); // slot は 30固定でいい？
}

// 左右チャンネルの反転した効果音を再生
void ssnd_playLRrev(int no) {
	mus_wav_load_lrsw(30, no);
	mus_wav_play(30, 1);
}

// 指定の番号の効果音が存在するかどうかをチェック
int ssnd_getlinknum(int no) {
	WARNING("NOT IMPLEMENTED");
	return 0;
}

// すべての再生中の効果音が再生を終了するのを待つ
void ssnd_stopall(int time) {
	int i;
	
	for (i = 0; i < CACHEMAX; i++) {
		if (cache[i] > 0) {
			mus_wav_fadeout_start(i + SLOTOFFSET, time, 0, true);
			cache[i] = 0;
		}
	}
}
