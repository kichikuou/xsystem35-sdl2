/*
 * bgm.sdlmixer.c: BGM (*BA.ALD) playback
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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <SDL.h>
#include <SDL_mixer.h>

#include "portab.h"
#include "system.h"
#include "nact.h"
#include "bgm.h"
#include "music_private.h"
#include "ald_manager.h"

#define BGI_MAX 100

typedef struct {
	int no;      // シナリオ上での曲番号
	int loopno;  // ループ回数 (０の場合は無限)
	int looptop; // ループ時先頭戻り位置 (サンプル数)
	int len;     // 曲長さ (サンプル数)
} bgi_t;

static int bgi_nfile;
static bgi_t bgi_data[BGI_MAX];

static int current_no;
static Mix_Music *mix_music;
static dridata* dfile;
static Uint32 start_time;

static char *bgi_gets(char *buf, int n, FILE *fp) {
	char *s = buf;
	int c;
	while (--n > 0 && (c = fgetc(fp)) != EOF) {
		c = c >> 4 | (c & 0xf) << 4;  // decrypt
		*s++ = c;
		if (c == '\n')
			break;
	}
	if (s == buf && c == EOF)
		return NULL;
	*s = '\0';
	return buf;
}

static void bgi_read() {
	if (nact->files.bgi == NULL)
		return;

	FILE *fp = fopen(nact->files.bgi, "rb");
	if (!fp) {
		WARNING("Could not open %s\n", nact->files.bgi);
		return;
	}

	char buf[100];
	while (bgi_nfile < BGI_MAX && bgi_gets(buf, sizeof(buf), fp)) {
		int terminator;
		if (sscanf(buf, " %d, %d, %d, %d, %d",
				   &bgi_data[bgi_nfile].no,
				   &bgi_data[bgi_nfile].loopno,
				   &bgi_data[bgi_nfile].looptop,
				   &bgi_data[bgi_nfile].len,
				   &terminator) != 5
			|| terminator != -1) {
			continue;
		}
		bgi_nfile++;
	}
	return;
}

static bgi_t *bgi_find(int no) {
	for (int i = 0; i < bgi_nfile; i++) {
		if (bgi_data[i].no == no)
			return &bgi_data[i];
	}
	return NULL;
}

static void free_music() {
	current_no = 0;
	if (mix_music) {
		Mix_FreeMusic(mix_music);
		mix_music = NULL;
	}
	if (dfile) {
		ald_freedata(dfile);
		dfile = NULL;
	}
}

static Mix_Music *bgm_load(int no) {
	free_music();

	dfile = ald_getdata(DRIFILE_BGM, no -1);
	if (dfile == NULL) {
		WARNING("DRIFILE_BGM fail to open %d\n", no -1);
		return NULL;
	}

	SDL_RWops *rwops = SDL_RWFromConstMem(dfile->data, dfile->size);

	mix_music = Mix_LoadMUSType_RW(rwops, MUS_WAV, SDL_TRUE /* freesrc */);
	if (mix_music == NULL) {
		WARNING("not .wav file\n");
		free_music();
		return NULL;
	}

	current_no = no;
	return mix_music;
}

int musbgm_init() {
	bgi_read();
	if (!bgi_nfile) return NG;
	return OK;
}

int musbgm_exit() {
	free_music();
	return OK;
}

int musbgm_play(int no, int time, int vol) {
	if (current_no)
		musbgm_stop(current_no, 0);

	if (!bgm_load(no))
		return NG;

	// We don't use the loop information in the BGI file, but SDL_mixer
	// understands loop info in the WAVE's "smpl" chunk.

	Mix_VolumeMusic(vol * MIX_MAX_VOLUME / 100);
	if (Mix_FadeInMusic(mix_music, -1, time * 10) != 0) {
		free_music();
		return NG;
	}
	start_time = SDL_GetTicks();

	return OK;
}

int musbgm_stop(int no, int time) {
	if (no == current_no)
		Mix_FadeOutMusic(time * 10);
	return OK;
}

int musbgm_fade(int no, int time, int vol) {
	if (no != current_no)
		return NG;

	// SDL_mixer doesn't provide arbitrary fading, so just set the volume immediately.
	Mix_VolumeMusic(vol * MIX_MAX_VOLUME / 100);
	return OK;
}

int musbgm_getpos(int no) {
	if (!musbgm_isplaying(no))
		return 0;

	// FIXME: This is not correct after loop
	return (SDL_GetTicks() - start_time) / 10;
}

int musbgm_getlen(int no) {
	bgi_t *bgi = bgi_find(no);
	if (!bgi) return 0;

	// FIXME: This assumes 44.1kHz
	return bgi->len / 441;
}

int musbgm_isplaying(int no) {
	return no == current_no && Mix_PlayingMusic();
}

int musbgm_stopall(int time) {
	return musbgm_stop(current_no, time);
}

int musbgm_wait(int no, int timeout) {
	while (timeout-- > 0 && musbgm_isplaying(no))
		SDL_Delay(10);
	return OK;
}
