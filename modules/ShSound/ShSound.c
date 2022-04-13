/*
 * ShSound.c 音楽関連 module
 *
 *    大悪司
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
/* $Id: ShSound.c,v 1.13 2003/08/02 13:10:32 chikama Exp $ */
#include "config.h"

#include <stdio.h>
#include <string.h>

#include "portab.h"
#include "xsystem35.h"
#include "modules.h"
#include "nact.h"
#include "dri.h"
#include "ald_manager.h"
#include "music.h"

#ifdef ENABLE_SDLMIXER
#include "pcm.sdlmixer.h"
#include "shpcmlib.c"

static Mix_Chunk *chunk;
#endif

static void Init() {
	/*
	  モジュール初期化
	*/
	int p1 = getCaliValue(); /* ISys3x */
	
	DEBUG_COMMAND("ShSound.Init %d:\n", p1);
}

static void wavLoad() {
	/*
	  指定のスロットにPCMファイルをロード
	  
	  slot: ロードするスロット(チャンネル)番号
	  no  : ロードするファイル番号
	*/
	int slot = getCaliValue();
	int no   = getCaliValue();
	
	DEBUG_COMMAND("ShSound.wavLoad %d,%d:\n", slot, no);
	
	mus_wav_load(slot, no);
}

static void wavUnload() {
	/*
	  指定のスロットのPCMファイルを削除
	  
	  slot: 削除するスロット番号
	*/
	int slot = getCaliValue();
	
	mus_wav_unload(slot);
	
	DEBUG_COMMAND("ShSound.wavUnload %d:\n",slot);
}

static void wavUnloadRange() {
	/*
	  指定のスロット(複数)のPCMファイルを削除
	  
	  slot:  削除する最初のスロット番号
	  range: 削除する個数
	*/
	int slot  = getCaliValue();
	int range = getCaliValue();
	int i;
	
	for (i = slot; i < (slot + range); i++) {
		mus_wav_unload(i);
	}
	
	DEBUG_COMMAND("ShSound.wavUnloadRange %d,%d:\n", slot, range);
}

static void wavUnloadAll() {
	/*
	  すべてのスロットのPCMファイルを削除
	*/
	int i;
	
	for (i = 0; i < 128; i++) {
		mus_wav_unload(i);
	}
	
	DEBUG_COMMAND("ShSound.wavUnloadAll:\n");
}

static void wavLoadMemory() {
	/*
	  指定の番号の WAV ファイルをメモリ上に読み込み
	  
	  no: 読み込むファイル番号
	*/
	int no = getCaliValue();
	
#ifdef ENABLE_SDLMIXER
	chunk = pcm_sdlmixer_load(no);
#endif
	
	DEBUG_COMMAND("ShSound.wavLoadMemory %d:\n", no);
}

static void wavSendMemory() {
	/*
	  wavLoadMemory で読み込んだデータを指定のスロットに投入
	  
	  slot: PCMデータを送るスロット番号
	*/
	int slot = getCaliValue();
	
#ifdef ENABLE_SDLMIXER
	if (chunk) {
		pcm_sdlmixer_load_chunk(slot, chunk);
		chunk = NULL;
	}
#endif
	
	DEBUG_COMMAND("ShSound.wavSendMemory %d:\n", slot);
}

static void wavFadeVolumeMemory() {
	/*
	  wavLoadMemory で読み込んだデータのボリュームのフェード
	  
	  start: フェード開始時間 (10msec単位)
	  range: フェード継続時間 (10msec単位)
	*/
	int start = getCaliValue();
	int range = getCaliValue();
	
#ifdef ENABLE_SDLMIXER
	if (chunk == NULL) return;
	
	pcmlib_fade_volume_memory(chunk, start, range);
#endif
	
	DEBUG_COMMAND("ShSound.wavFadeVolumeMemory %d,%d:\n", start, range);
}

static void wavReversePanMemory() {
	/*
	  wavLoadMemoryで読み込んだデータの左右のチャンネルを反転
	*/
	
#ifdef ENABLE_SDLMIXER
	if (chunk == NULL) return;
	
	pcmlib_reverse_pan_memory(chunk);
#endif
	
	DEBUG_COMMAND("ShSound.wavReversePanMemory:\n");
}

static void wavPlay() {
	/*
	  指定のスロットのPCMを再生
	  
	  slot: 再生するスロット番号
	  loop: 0なら１回だけ再生、!0なら無限に繰り返し
	*/
	int slot = getCaliValue();
	int loop = getCaliValue();
	
	mus_wav_play(slot, loop == 0 ? 1 : -1);
	
	DEBUG_COMMAND("ShSound.wavPlay %d, %d:\n", slot, loop);
}

static void wavPlayRing() {
	/*
	  指定の範囲のスロットのPCMを呼ばれる毎に繰り返し
	  
	  start: 最初のスロット番号
	  cnt:   繰り返すスロットの個数
	  *cur:  現在再生しているスロットのインデックス
	*/
	int start = getCaliValue();
	int cnt   = getCaliValue();
	int *cur  = getCaliVariable();
	
	mus_wav_play(start + (*cur % cnt), 1);
	*cur = (*cur + 1) % cnt;
	
	DEBUG_COMMAND("ShSound.wavPlayRing %d,%d,%d:\n", start, cnt, *cur);
}

static void wavStop() {
	/*
	  指定のスロットの再生を停止
	  
	  slot: 停止するスロット番号
	*/
	int slot = getCaliValue();
	
	DEBUG_COMMAND("ShSound.wavStop %d:\n", slot);
	
	mus_wav_stop(slot);
}

static void wavStopAll() {
	/*
	  全てのスロットの再生を停止
	*/
	int i;
	
	for (i = 0; i < 128; i++) {
		mus_wav_stop(i);
	}
	
	DEBUG_COMMAND("ShSound.wavStopAll:\n");
}

static void wavPause() {
	/*
	  指定のスロットの再生を一時停止
	  
	  slot: 一時停止するスロット番号
	*/
	int slot = getCaliValue();
	
	DEBUG_COMMAND_YET("ShSound.wavPause %d:\n", slot);
}

static void wavIsPlay() {
	/*
	  指定のスロットが再生中かどうかを調べる
	  
	  slot:    調べるスロット番号
	  *result: 0なら停止中、!0なら再生中
	*/ 
	int slot = getCaliValue();
	int *result = getCaliVariable();
	
	*result = mus_wav_get_playposition(slot);
	
	DEBUG_COMMAND("ShSound.wavIsPlay %d,%p:\n", slot, result);
}

static void wavIsPlayRange() {
	/*
	  指定の範囲のスロットが再生中かどうかを調べる
	  
	  slot: 調べる最初のスロット
	  range: 調べるスロットの個数
	  *result: 0:すべてのスロットが停止中, 1: どれかのスロットが再生中
	*/
	int slot  = getCaliValue();
	int range = getCaliValue();
	int *result = getCaliVariable();
	int i, ret = 0;
	
	for (i = slot; i < (slot + range); i++) {
		ret += mus_wav_get_playposition(i);
	}
	
	*result = ret;
	
	DEBUG_COMMAND("ShSound.wavIsPlayRange %d,%d,%p:\n", slot, range, result);
}

static const ModuleFunc functions[] = {
	{"Init", Init},
	{"wavFadeVolumeMemory", wavFadeVolumeMemory},
	{"wavIsPlay", wavIsPlay},
	{"wavIsPlayRange", wavIsPlayRange},
	{"wavLoad", wavLoad},
	{"wavLoadMemory", wavLoadMemory},
	{"wavPause", wavPause},
	{"wavPlay", wavPlay},
	{"wavPlayRing", wavPlayRing},
	{"wavReversePanMemory", wavReversePanMemory},
	{"wavSendMemory", wavSendMemory},
	{"wavStop", wavStop},
	{"wavStopAll", wavStopAll},
	{"wavUnload", wavUnload},
	{"wavUnloadAll", wavUnloadAll},
	{"wavUnloadRange", wavUnloadRange},
};

const Module module_ShSound = {"ShSound", functions, sizeof(functions) / sizeof(ModuleFunc)};
