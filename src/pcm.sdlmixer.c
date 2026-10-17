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

#include "portab.h"
#include "system.h"
#include "ald_manager.h"
#include "dri.h"
#include "music_pcm.h"
#include "music_private.h"
#include "nact.h"
#include "audio_meta.h"
#include "mixer.h"

#define DEFAULT_AUDIO_BUFFER_SIZE 2048

// 0:     S comman 用
// 1-128: wavXXX 用
#define PCM_SLOTS (128 + 1)

static struct {
	mixer_sound *sound;
	uint32_t start_time;
	int volume;  // 0-100
} slots[PCM_SLOTS];

// Set the channel gain from the slot's volume and its volume valancer level.
static void apply_volume(int slot) {
	int ch = prv.vol_pcm_sub[slot];
	if ((unsigned)ch >= 16)  // guard against bad WAI data
		ch = 0;
	mixer_channel_set_gain(slot, prv.volval[ch] * slots[slot].volume / 10000.0f);
}

static dridata *open_asset(DRIFILETYPE type, int no) {
	dridata *dfile = ald_getdata(type, no -1);
	if (dfile == NULL)
		WARNING("DRIFILE_WAVE fail to open %d", no -1);
	return dfile;
}

/**
 * 指定の番号の .WAV|.OGG をロードする。
 * @param no: DRIファイル番号
 */
static mixer_sound *load_asset(DRIFILETYPE type, int no) {
	dridata *dfile = open_asset(type, no);
	if (dfile == NULL)
		return NULL;

	mixer_sound *sound = mixer_sound_load(dfile->data, dfile->size);
	ald_freedata(dfile);
	if (sound == NULL) {
		WARNING("DRIFILE_WAVE %d: not a valid wav file", no-1);
		return NULL;
	}
	return sound;
}

static int16_t *decode_asset(DRIFILETYPE type, int no, size_t *frames) {
	dridata *dfile = open_asset(type, no);
	if (dfile == NULL)
		return NULL;

	int16_t *samples = mixer_decode(dfile->data, dfile->size, frames);
	ald_freedata(dfile);
	if (samples == NULL) {
		WARNING("DRIFILE_WAVE %d: not a valid wav file", no-1);
		return NULL;
	}
	return samples;
}

static bool load_sound(int slot, mixer_sound *sound) {
	if ((unsigned)slot >= PCM_SLOTS)
		return false;

	if (slots[slot].sound)
		muspcm_unload(slot);

	slots[slot].sound = sound;
	apply_volume(slot);  // callers set vol_pcm_sub[slot] before this
	return true;
}

/**
 * noL と noR の .WAV をロードし、左右合成
 *
 * @param noL: 左の WAV ファイルの番号
 * @param noR: 右の WAV ファイルの番号
 * @return   : 合成後の mixer_sound
 */
static mixer_sound *pcm_mixlr(int noL, int noR) {
	size_t lframes, rframes;
	int16_t *lbuf = decode_asset(DRIFILE_WAVE, noL, &lframes);
	int16_t *rbuf = decode_asset(DRIFILE_WAVE, noR, &rframes);
	if (lbuf == NULL || rbuf == NULL) {
		free(lbuf);
		free(rbuf);
		return NULL;
	}

	if (lframes >= rframes) {
		size_t i;
		for (i = 0; i < rframes; i++) {
			lbuf[i * 2 + 1] = rbuf[i * 2 + 1];
		}
		for (; i < lframes; i++) {
			lbuf[i * 2 + 1] = 0;
		}
		free(rbuf);
		return mixer_sound_from_samples(lbuf, lframes);
	} else {
		size_t i;
		for (i = 0; i < lframes; i++) {
			rbuf[i * 2] = lbuf[i * 2];
		}
		for (; i < rframes; i++) {
			rbuf[i * 2] = 0;
		}
		free(lbuf);
		return mixer_sound_from_samples(rbuf, rframes);
	}
}

bool muspcm_init(int audio_buffer_size) {
	if (!audio_buffer_size)
		audio_buffer_size = DEFAULT_AUDIO_BUFFER_SIZE;

	if (!mixer_init(PCM_SLOTS, audio_buffer_size))
		return false;
	for (int i = 0; i < PCM_SLOTS; i++)
		slots[i].volume = 100;
	wai_load(nact->files.wai);
	return true;
}

void muspcm_exit(void) {
	mixer_quit();
}

void muspcm_reset(void) {
	for (int i = 0; i < PCM_SLOTS; i++)
		muspcm_unload(i);
}

// 番号指定のPCMファイル読み込み
bool muspcm_load_no(int slot, int no) {
	if ((unsigned)slot >= PCM_SLOTS)
		return false;

	mixer_sound *sound = load_asset(DRIFILE_WAVE, no);
	if (sound == NULL) {
		return false;
	}
	
	// if has .wai file
	if (wai_loaded()) {
		int ch = wai_mixch(no);
		prv.vol_pcm_sub[slot] = ch < 0 ? 0: ch;
	} else {
		prv.vol_pcm_sub[slot] = SE_VOLVAL_CH;
	}

	return load_sound(slot, sound);
}

bool muspcm_load_bgm(int slot, int no) {
	if ((unsigned)slot >= PCM_SLOTS)
		return false;

	mixer_sound *sound = load_asset(DRIFILE_BGM, no);
	if (sound == NULL) {
		return false;
	}
	prv.vol_pcm_sub[slot] = 0;

	return load_sound(slot, sound);
}

bool muspcm_load_mixlr(int slot, int noL, int noR) {
	if ((unsigned)slot >= PCM_SLOTS)
		return false;

	/* mix 2 wave files */
	mixer_sound *sound = pcm_mixlr(noL, noR);
	if (sound == NULL) {
		return false;
	}
	prv.vol_pcm_sub[slot] = SE_VOLVAL_CH;

	return load_sound(slot, sound);
}

bool muspcm_load_data(int slot, uint8_t *buf, uint32_t len) {
	if ((unsigned)slot >= PCM_SLOTS)
		return false;

	mixer_sound *sound = mixer_sound_load(buf, len);
	if (!sound)
		return false;
	prv.vol_pcm_sub[slot] = SE_VOLVAL_CH;

	return load_sound(slot, sound);
}

// PCMデータのメモリ上からのアンロード
void muspcm_unload(int slot) {
	if ((unsigned)slot >= PCM_SLOTS)
		return;
	if (!slots[slot].sound)
		return;

	mixer_channel_halt(slot);
	mixer_sound_free(slots[slot].sound);
	slots[slot].sound = NULL;
}

// PCMデータを再生
bool muspcm_start(int slot, int loop) {
	if ((unsigned)slot >= PCM_SLOTS)
		return false;

	if (!slots[slot].sound)
		return false;

	if (!mixer_channel_play(slot, slots[slot].sound, loop - 1))
		return false;

	slots[slot].start_time = sys_get_ticks();
	return true;
}

// PCMデータの再生停止
void muspcm_stop(int slot) {
	if ((unsigned)slot >= PCM_SLOTS)
		return;
	mixer_channel_halt(slot);
}

// 指定時間のフェードアウトの後に再生停止
void muspcm_fadeout(int slot, int msec) {
	if ((unsigned)slot >= PCM_SLOTS)
		return;
	mixer_channel_fade_out(slot, msec);
}

// PCMデータ再生一時停止
void muspcm_pause(int slot) {
	if ((unsigned)slot >= PCM_SLOTS)
		return;
	mixer_channel_pause(slot);
}

// PCMデータ再生一時停止解除
void muspcm_unpause(int slot) {
	if ((unsigned)slot >= PCM_SLOTS)
		return;
	mixer_channel_resume(slot);
}

// 現在の再生位置を返す
int muspcm_getpos(int slot) {
	if ((unsigned)slot >= PCM_SLOTS)
		return 0;

	if (!mixer_channel_playing(slot))
		return 0;

	int pos = sys_get_ticks() - slots[slot].start_time;
	if (pos == 0)
		pos = 1;  // because 0 means "not playing"

	return pos;
}

// Set the volume for a channel.
void muspcm_setvol(int slot, int lv) {
	if ((unsigned)slot >= PCM_SLOTS)
		return;
	slots[slot].volume = lv;
	apply_volume(slot);
}

// PCMデータの長さを取得
int muspcm_getwavelen(int slot) {
	if ((unsigned)slot >= PCM_SLOTS)
		return 0;

	if (!slots[slot].sound)
		return 0;

	int len = mixer_sound_duration_ms(slots[slot].sound);
	return len > 65535 ? 65535 : len;
}

// 指定のスロットが現在演奏中かどうかを取得
bool muspcm_isplaying(int slot) {
	if ((unsigned)slot >= PCM_SLOTS)
		return false;

	return mixer_channel_playing(slot);
}

// 指定のチャンネルの再生が終了するまで待つ
void muspcm_waitend(int slot) {
	WARNING("not implemented");
}

// Re-apply the valancer levels to all channels.
void muspcm_reapply_valance(void) {
	for (int slot = 0; slot < PCM_SLOTS; slot++)
		apply_volume(slot);
}
