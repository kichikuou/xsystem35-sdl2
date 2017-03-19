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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <SDL_mixer.h>

#include "portab.h"
#include "system.h"
#include "music_pcm.h"
#include "music_private.h"
#include "counter.h"
#include "pcmlib.h"

#define IS_LOADED(slot) (prv.pcm[(slot)])

static int load_chunk(int slot, Mix_Chunk *chunk);


#include "music_pcm_wai.c"

int muspcm_init() {
	Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG);
	if (Mix_OpenAudio(44100, AUDIO_S16LSB, 2, 4096) < 0) {
		prv.pcm_valid = FALSE;
		return NG;
	}
	
	muspcm_load_wai();
	
	prv.pcm_valid = TRUE;
	return OK;
}

int muspcm_exit() {
	Mix_CloseAudio();
	Mix_Quit();
	return OK;
}

// 番号指定のPCMファイル読み込み
int muspcm_load_no(int slot, int no) {
	if (IS_LOADED(slot)) muspcm_unload(slot);
	
	Mix_Chunk *chunk = pcmlib_load(no);
	if (chunk == NULL) {
		return NG;
	}
	
	// if has .wai file
	if (wai_mapadr) {
		int ch = WAIMIXCH(no);
		prv.vol_pcm_sub[slot] = ch < 0 ? 0: ch;
	} else {
		prv.vol_pcm_sub[slot] = 0;
	}

	return load_chunk(slot, chunk);
}

int muspcm_load_mixlr(int slot, int noL, int noR) {
	/* mix 2 wave files */
	Mix_Chunk* chunk = pcmlib_mixlr(noL, noR);
	if (chunk == NULL) {
		puts("mixlr fail");
		return NG;
	}

	return load_chunk(slot, chunk);
}

static int load_chunk(int slot, Mix_Chunk *chunk) {
	if (IS_LOADED(slot)) muspcm_unload(slot);

	pcmobj_t *obj = calloc(1, sizeof(pcmobj_t));
	
	obj->chunk    = chunk;
	obj->slot     = slot;
	obj->playing  = FALSE;
	
	prv.pcm[slot] = obj;
	
	return OK;
}

// PCMデータを再生
int muspcm_start(int slot, int loop) {
	pcmobj_t *obj;
	// printf("pcm start slot = %d, loop = %d\n", slot, loop);
	
	obj = prv.pcm[slot];
	if (obj == NULL) return NG;
	
	obj->channel = Mix_PlayChannel(-1, obj->chunk, loop - 1);
	if (obj->channel < 0)
		return NG;
	
	obj->playing = TRUE;
	return OK;
}

// PCMデータの再生停止
int muspcm_stop(int slot) {
	pcmobj_t *obj;
	
	obj = prv.pcm[slot];
	if (obj == NULL) return NG;
	
	if (obj->playing) {
		Mix_HaltChannel(obj->channel);
		obj->playing = FALSE;
	}
	
	return OK;
}

// PCMデータのメモリ上からのアンロード
int muspcm_unload(int slot) {
	pcmobj_t *obj;
	
	obj = prv.pcm[slot];
	if (obj == NULL) return NG;
	
	if (obj->playing) muspcm_stop(slot);
	
	Mix_FreeChunk(obj->chunk);
	free(obj);
	
	prv.pcm[slot] = NULL;
	
	return OK;
}

// PCMデータ再生一時停止
int muspcm_pause(int slot) {
	if (prv.pcm[slot] != NULL) {
		Mix_Pause(prv.pcm[slot]->channel);
	}
	return OK;
}

// PCMデータ再生一時停止解除
int muspcm_unpause(int slot) {
	if (prv.pcm[slot] != NULL) {
		Mix_Resume(prv.pcm[slot]->channel);
	}
	return OK;
}

// 現在の再生位置を返す
int muspcm_getpos(int slot) {
	pcmobj_t *obj;
	uint64_t len;
	
	obj = prv.pcm[slot];
	if (obj == NULL) return 0;
	
	if (!obj->playing) return 0;
	
	printf("%s not implemented\n", __func__);
	return 0;
}

// PCMオブジェクトに対してボリュームをセット
int muspcm_setvol(int dev, int slot, int lv) {
	pcmobj_t *obj;
	
	obj = prv.pcm[slot];
	if (obj == NULL) return NG;
	
	Mix_VolumeChunk(obj->chunk, lv * MIX_MAX_VOLUME / 100);
	
	return OK;
}

// PCMデータの長さを取得
int muspcm_getwavelen(int slot) {
	pcmobj_t *obj;
	
	obj = prv.pcm[slot];
	if (obj == NULL) return 0;
	
	long len = obj->chunk->alen * 1000 / (44100 * 4);

	return len > 65535 ? 65535 : len;
}

// 指定のスロットが現在演奏中かどうかを取得
boolean muspcm_isplaying(int slot) {
	pcmobj_t *obj;
	
	obj = prv.pcm[slot];
	if (obj == NULL)   return FALSE;
	
	return obj->playing;
}
