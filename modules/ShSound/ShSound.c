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
#include <SDL.h>

#include "portab.h"
#include "LittleEndian.h"
#include "xsystem35.h"
#include "modules.h"
#include "nact.h"
#include "dri.h"
#include "ald_manager.h"
#include "music.h"

static struct {
	SDL_AudioSpec spec;  // must be 16-bit, stereo
	uint8_t *buf;
	uint32_t len;
} memwav;

static void free_memory_wav(void) {
	SDL_FreeWAV(memwav.buf);
	memwav.buf = NULL;
}

static void Init() {
	/*
	  モジュール初期化
	*/
	int p1 = getCaliValue(); /* ISys3x */
	
	DEBUG_COMMAND("ShSound.Init %d:", p1);
}

static void wavLoad() {
	/*
	  指定のスロットにPCMファイルをロード
	  
	  slot: ロードするスロット(チャンネル)番号
	  no  : ロードするファイル番号
	*/
	int slot = getCaliValue();
	int no   = getCaliValue();
	
	DEBUG_COMMAND("ShSound.wavLoad %d,%d:", slot, no);
	
	mus_wav_load(slot, no);
}

static void wavUnload() {
	/*
	  指定のスロットのPCMファイルを削除
	  
	  slot: 削除するスロット番号
	*/
	int slot = getCaliValue();
	
	mus_wav_unload(slot);
	
	DEBUG_COMMAND("ShSound.wavUnload %d:",slot);
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
	
	DEBUG_COMMAND("ShSound.wavUnloadRange %d,%d:", slot, range);
}

static void wavUnloadAll() {
	/*
	  すべてのスロットのPCMファイルを削除
	*/
	int i;
	
	for (i = 0; i < 128; i++) {
		mus_wav_unload(i);
	}
	
	DEBUG_COMMAND("ShSound.wavUnloadAll:");
}

static void wavLoadMemory() {
	/*
	  指定の番号の WAV ファイルをメモリ上に読み込み
	  
	  no: 読み込むファイル番号
	*/
	int no = getCaliValue();

	if (memwav.buf)
		free_memory_wav();

	dridata *dfile = ald_getdata(DRIFILE_WAVE, no - 1);
	if (!dfile) {
		WARNING("cannot open WAVE %d", no - 1);
		return;
	}
	if (!SDL_LoadWAV_RW(SDL_RWFromConstMem(dfile->data, dfile->size), 1, &memwav.spec, &memwav.buf, &memwav.len)) {
		WARNING("cannot load WAVE %d", no - 1);
		return;
	}
	if (memwav.spec.channels != 2 || memwav.spec.format != AUDIO_S16LSB) {
		WARNING("unexpected audio format");
		free_memory_wav();
		return;
	}
	
	DEBUG_COMMAND("ShSound.wavLoadMemory %d:", no);
}

static void wavSendMemory() {
	/*
	  wavLoadMemory で読み込んだデータを指定のスロットに投入
	  
	  slot: PCMデータを送るスロット番号
	*/
	int slot = getCaliValue();
	
	if (!memwav.buf) {
		WARNING("wave not loaded");
		return;
	}
	const uint8_t wave_header[] = {
		'R', 'I', 'F', 'F',
		  0,   0,   0,   0,  // filesize - 8 (filled later)
		'W', 'A', 'V', 'E',
		'f', 'm', 't', ' ',
		 16,   0,   0,   0,  // size of fmt chunk
		  1,   0,            // PCM format
		  2,   0,            // stereo
		  0,   0,   0,   0,  // sampling rate (filled later)
		  0,   0,   0,   0,  // bytes / sec (filled later)
		  4,   0,            // block size
		 16,   0,            // bits / sample
		'd', 'a', 't', 'a',
		  0,   0,   0,   0,  // size of data chunk (filled later)
	};

	uint32_t wav_size = sizeof(wave_header) + memwav.len;
	uint8_t *wav_buf = malloc(wav_size);
	memcpy(wav_buf, wave_header, sizeof(wave_header));
	LittleEndian_putDW(wav_size - 8, wav_buf, 4);
	LittleEndian_putDW(memwav.spec.freq, wav_buf, 24);
	LittleEndian_putDW(memwav.spec.freq * 4, wav_buf, 28);
	LittleEndian_putDW(memwav.len, wav_buf, 40);
	memcpy(wav_buf + sizeof(wave_header), memwav.buf, memwav.len);

	mus_wav_load_data(slot, wav_buf, wav_size);
	free_memory_wav();
	free(wav_buf);
	
	DEBUG_COMMAND("ShSound.wavSendMemory %d:", slot);
}

static void wavFadeVolumeMemory() {
	/*
	  wavLoadMemory で読み込んだデータのボリュームのフェード
	  
	  start: フェード開始時間 (10msec単位)
	  range: フェード継続時間 (10msec単位)
	*/
	int start = getCaliValue();
	int range = getCaliValue();
	
	if (!memwav.buf)
		return;
	
	start *= memwav.spec.freq / 100;  // 10ms -> sample
	range *= memwav.spec.freq / 100;  // 10ms -> sample

	if (memwav.len / 4 < start + range)
		return;

	int16_t *buf = (int16_t*)memwav.buf + start * 2;

	// 指定の場所から徐々に音量を下げる
	for (int i = range; i > 0; i--) {
		buf[0] = buf[0] * i / range;
		buf[1] = buf[1] * i / range;
		buf += 2;
	}

	// 残りは無音
	memset(buf, 0, memwav.buf + memwav.len - (uint8_t*)buf);
	
	DEBUG_COMMAND("ShSound.wavFadeVolumeMemory %d,%d:", start, range);
}

static void wavReversePanMemory() {
	/*
	  wavLoadMemoryで読み込んだデータの左右のチャンネルを反転
	*/
	
	if (!memwav.buf)
		return;
	
	int16_t *buf = (int16_t*)memwav.buf;
	int len = memwav.len / 4;
	for (int i = 0; i < len; i++) {
		int16_t tmp = buf[0];
		buf[0] = buf[1];
		buf[1] = tmp;
		buf += 2;
	}
	
	DEBUG_COMMAND("ShSound.wavReversePanMemory:");
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
	
	DEBUG_COMMAND("ShSound.wavPlay %d, %d:", slot, loop);
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
	
	DEBUG_COMMAND("ShSound.wavPlayRing %d,%d,%d:", start, cnt, *cur);
}

static void wavStop() {
	/*
	  指定のスロットの再生を停止
	  
	  slot: 停止するスロット番号
	*/
	int slot = getCaliValue();
	
	DEBUG_COMMAND("ShSound.wavStop %d:", slot);
	
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
	
	DEBUG_COMMAND("ShSound.wavStopAll:");
}

static void wavPause() {
	/*
	  指定のスロットの再生を一時停止
	  
	  slot: 一時停止するスロット番号
	*/
	int slot = getCaliValue();
	
	DEBUG_COMMAND_YET("ShSound.wavPause %d:", slot);
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
	
	DEBUG_COMMAND("ShSound.wavIsPlay %d,%p:", slot, result);
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
	
	DEBUG_COMMAND("ShSound.wavIsPlayRange %d,%d,%p:", slot, range, result);
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
