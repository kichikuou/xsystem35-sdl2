/*
 * music_pcm.c  music server PCM part
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
/* $Id: music_pcm.c,v 1.16 2003/11/09 15:06:13 chikama Exp $ */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <SDL_mixer.h>

#include "portab.h"
#include "system.h"
#include "ald_manager.h"
#include "dri.h"
#include "music_pcm.h"
#include "music_private.h"
#include "sdl_core.h"
#include "nact.h"
#include "LittleEndian.h"
#include "mmap.h"

#define SAMPLE_RATE 44100
#define BYTES_PER_SAMPLE 4
#define DEFAULT_AUDIO_BUFFER_SIZE 2048

// 0:     S comman 用
// 1-128: wavXXX 用
#define PCM_SLOTS (128 + 1)

static struct {
	Mix_Chunk* chunk;
	uint32_t start_time;
} slots[PCM_SLOTS];

static int load_wai();
static mmap_t *wai_map;

// Volume mixer channel
#define WAIMIXCH(no) LittleEndian_getDW(wai_map->addr, 36 + (no -1) * 4 * 3 + 8)

/**
 * 指定の番号の .WAV|.OGG をロードする。
 * @param no: DRIファイル番号
 */
static Mix_Chunk *load_asset(DRIFILETYPE type, int no) {
	dridata *dfile = ald_getdata(type, no -1);
	if (dfile == NULL) {
		WARNING("DRIFILE_WAVE fail to open %d", no -1);
		return NULL;
	}

	Mix_Chunk *chunk = Mix_LoadWAV_RW(SDL_RWFromConstMem(dfile->data, dfile->size), 1);
	ald_freedata(dfile);
	if (chunk == NULL) {
		WARNING("DRIFILE_WAVE %d: not a valid wav file", no-1);
		return NULL;
	}
	assert(chunk->allocated);

	return chunk;
}

static bool load_chunk(int slot, Mix_Chunk *chunk) {
	if ((unsigned)slot >= PCM_SLOTS)
		return false;

	if (slots[slot].chunk)
		muspcm_unload(slot);

	slots[slot].chunk = chunk;
	return true;
}

/**
 * noL と noR の .WAV をロードし、左右合成
 *
 * @param noL: 左の WAV ファイルの番号
 * @param noR: 右の WAV ファイルの番号
 * @return   : 合成後の Mix_Chunk
 */
static Mix_Chunk *pcm_mixlr(int noL, int noR) {
	Mix_Chunk *chunkL = load_asset(DRIFILE_WAVE, noL);
	Mix_Chunk *chunkR = load_asset(DRIFILE_WAVE, noR);
	if (chunkL == NULL || chunkR == NULL) {
		if (chunkL)
			Mix_FreeChunk(chunkL);
		if (chunkR)
			Mix_FreeChunk(chunkR);
		return NULL;
	}

	short *lbuf = (short*)chunkL->abuf;
	short *rbuf = (short*)chunkR->abuf;
	if (chunkL->alen >= chunkR->alen) {
		int i;
		for (i = 0; i < chunkR->alen / 4; i++) {
			lbuf[i * 2 + 1] = rbuf[i * 2 + 1];
		}
		for (; i < chunkL->alen / 4; i++) {
			lbuf[i * 2 + 1] = 0;
		}
		Mix_FreeChunk(chunkR);
		return chunkL;
	} else {
		int i;
		for (i = 0; i < chunkL->alen / 4; i++) {
			rbuf[i * 2] = lbuf[i * 2];
		}
		for (; i < chunkR->alen / 4; i++) {
			rbuf[i * 2] = 0;
		}
		Mix_FreeChunk(chunkL);
		return chunkR;
	}
}

bool muspcm_init(int audio_buffer_size) {
	if (!audio_buffer_size)
		audio_buffer_size = DEFAULT_AUDIO_BUFFER_SIZE;

	Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG);
	if (Mix_OpenAudio(SAMPLE_RATE, AUDIO_S16LSB, 2, audio_buffer_size) < 0)
		return false;
	Mix_AllocateChannels(PCM_SLOTS);
	load_wai();
	return true;
}

void muspcm_exit(void) {
	Mix_CloseAudio();
	Mix_Quit();
}

void muspcm_reset(void) {
	for (int i = 0; i < PCM_SLOTS; i++)
		muspcm_unload(i);
}

// 番号指定のPCMファイル読み込み
bool muspcm_load_no(int slot, int no) {
	if ((unsigned)slot >= PCM_SLOTS)
		return false;

	Mix_Chunk *chunk = load_asset(DRIFILE_WAVE, no);
	if (chunk == NULL) {
		return false;
	}
	
	// if has .wai file
	if (wai_map) {
		int ch = WAIMIXCH(no);
		prv.vol_pcm_sub[slot] = ch < 0 ? 0: ch;
	} else {
		prv.vol_pcm_sub[slot] = 0;
	}

	return load_chunk(slot, chunk);
}

bool muspcm_load_bgm(int slot, int no) {
	if ((unsigned)slot >= PCM_SLOTS)
		return false;

	Mix_Chunk *chunk = load_asset(DRIFILE_BGM, no);
	if (chunk == NULL) {
		return false;
	}
	prv.vol_pcm_sub[slot] = 0;

	return load_chunk(slot, chunk);
}

bool muspcm_load_mixlr(int slot, int noL, int noR) {
	if ((unsigned)slot >= PCM_SLOTS)
		return false;

	/* mix 2 wave files */
	Mix_Chunk* chunk = pcm_mixlr(noL, noR);
	if (chunk == NULL) {
		return false;
	}

	return load_chunk(slot, chunk);
}

bool muspcm_load_data(int slot, uint8_t *buf, uint32_t len) {
	if ((unsigned)slot >= PCM_SLOTS)
		return false;

	Mix_Chunk *chunk = Mix_LoadWAV_RW(SDL_RWFromConstMem(buf, len), 1);
	if (!chunk)
		return false;

	return load_chunk(slot, chunk);
}

// PCMデータのメモリ上からのアンロード
void muspcm_unload(int slot) {
	if ((unsigned)slot >= PCM_SLOTS)
		return;
	if (!slots[slot].chunk)
		return;

	Mix_HaltChannel(slot);
	Mix_FreeChunk(slots[slot].chunk);
	slots[slot].chunk = NULL;
}

// PCMデータを再生
bool muspcm_start(int slot, int loop) {
	if ((unsigned)slot >= PCM_SLOTS)
		return false;

	if (!slots[slot].chunk)
		return false;

	if (Mix_PlayChannel(slot, slots[slot].chunk, loop - 1) < 0)
		return false;

	slots[slot].start_time = sdl_getTicks();
	return true;
}

// PCMデータの再生停止
void muspcm_stop(int slot) {
	if ((unsigned)slot >= PCM_SLOTS)
		return;
	Mix_HaltChannel(slot);
}

// 指定時間のフェードアウトの後に再生停止
void muspcm_fadeout(int slot, int msec) {
	if ((unsigned)slot >= PCM_SLOTS)
		return;
	Mix_FadeOutChannel(slot, msec);
}

// PCMデータ再生一時停止
void muspcm_pause(int slot) {
	if ((unsigned)slot >= PCM_SLOTS)
		return;
	Mix_Pause(slot);
}

// PCMデータ再生一時停止解除
void muspcm_unpause(int slot) {
	if ((unsigned)slot >= PCM_SLOTS)
		return;
	Mix_Resume(slot);
}

// 現在の再生位置を返す
int muspcm_getpos(int slot) {
	if ((unsigned)slot >= PCM_SLOTS)
		return 0;

	if (!Mix_Playing(slot))
		return 0;

	int pos = sdl_getTicks() - slots[slot].start_time;
	if (pos == 0)
		pos = 1;  // because 0 means "not playing"

	return pos;
}

// Set the volume for a channel.
void muspcm_setvol(int slot, int lv) {
	if ((unsigned)slot >= PCM_SLOTS)
		return;
	Mix_Volume(slot, lv * MIX_MAX_VOLUME / 100);
}

// PCMデータの長さを取得
int muspcm_getwavelen(int slot) {
	if ((unsigned)slot >= PCM_SLOTS)
		return 0;

	if (!slots[slot].chunk)
		return 0;

	long len = slots[slot].chunk->alen * 1000LL / (SAMPLE_RATE * BYTES_PER_SAMPLE);
	return len > 65535 ? 65535 : len;
}

// 指定のスロットが現在演奏中かどうかを取得
bool muspcm_isplaying(int slot) {
	if ((unsigned)slot >= PCM_SLOTS)
		return false;

	return Mix_Playing(slot);
}

// 指定のチャンネルの再生が終了するまで待つ
void muspcm_waitend(int slot) {
	WARNING("not implemented");
}

static int load_wai() {
	if (wai_map) {
		unmap_file(wai_map);
		wai_map = NULL;
	}
	if (nact->files.wai == NULL)
		return NG;
	wai_map = map_file(nact->files.wai);
	if (!wai_map)
		return NG;

	char *adr = wai_map->addr;
	if (*adr != 'X' || *(adr+1) != 'I' || *(adr+2) != '2') {
		WARNING("not WAI file");
		unmap_file(wai_map);
		wai_map = NULL;
		return NG;
	}
	return OK;
}
