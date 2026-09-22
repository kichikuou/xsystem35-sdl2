/*
 * bgm.sdl3mixer.c: BGM (*BA.ALD) playback using SDL3_mixer
 *
 * Copyright (C) 1997-1998 Masaki Chikama (Wren) <chikama@kasumi.ipl.mech.nagoya-u.ac.jp>
 *               1998-                           <masaki-c@is.aist-nara.ac.jp>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdint.h>

#include "ald_manager.h"
#include "audio_meta.h"
#include "bgm.h"
#include "music_private.h"
#include "nact.h"
#include "sdl3_mixer_backend.h"
#include "sdl3_mixer_utils.h"
#include "system.h"

static DRIFILETYPE dri_type;
static int base_no;
static int current_no;
static int current_vol = 100;
static uint32_t start_time;

static float music_gain(int volume)
{
	return sdl3_mixer_gain(volume, prv.volval[BGM_VOLVAL_CH]);
}

static void apply_music_volume(int volume)
{
	current_vol = volume;
	sdl3_mixer_set_music_gain(SDL3_MIXER_MUSIC_BGM,
		music_gain(current_vol));
}

void musbgm_reapply_valance(void)
{
	apply_music_volume(current_vol);
}

static void free_music(void)
{
	current_no = 0;
	sdl3_mixer_stop_music(SDL3_MIXER_MUSIC_BGM, 0);
}

static MIX_Audio *load_music(int no)
{
	free_music();
	int ald_no = no + base_no - 1;
	dridata *dfile = ald_getdata(dri_type, ald_no);
	if (!dfile) {
		WARNING("Failed to open BGM %d", ald_no);
		return NULL;
	}

	MIX_Audio *audio = sdl3_mixer_load_music_memory(
		dfile->data, dfile->size);
	ald_freedata(dfile);
	if (!audio)
		WARNING("Failed to load BGM %d: %s", ald_no, SDL_GetError());
	return audio;
}

bool musbgm_init(DRIFILETYPE type, int base)
{
	dri_type = type;
	base_no = base;
	if (type == DRIFILE_BGM)
		return bgi_read(nact->files.bgi);
	return true;
}

void musbgm_exit(void)
{
	free_music();
}

void musbgm_reset(void)
{
	free_music();
}

bool musbgm_play(int no, int time, int volume, int loop_count)
{
	MIX_Audio *audio = load_music(no);
	if (!audio)
		return false;

	int loops = loop_count == 0 ? -1 : loop_count - 1;
	current_vol = volume;
	if (!sdl3_mixer_play_music(SDL3_MIXER_MUSIC_BGM, audio,
		loops, sdl3_mixer_10ms_to_ms(time), music_gain(current_vol)))
		return false;
	current_no = no;
	start_time = sys_get_ticks();
	return true;
}

void musbgm_stop(int no, int time)
{
	if (no == current_no)
		sdl3_mixer_stop_music(SDL3_MIXER_MUSIC_BGM,
			sdl3_mixer_10ms_to_ms(time));
}

void musbgm_fade(int no, int time, int volume)
{
	(void)time;
	if (no == current_no)
		apply_music_volume(volume);
}

int musbgm_getpos(int no)
{
	if (!musbgm_isplaying(no))
		return 0;
	return sdl3_mixer_elapsed_10ms(start_time, sys_get_ticks());
}

int musbgm_getlen(int no)
{
	bgi_t *bgi = bgi_find(no);
	return bgi ? bgi->len / 441 : 0;
}

bool musbgm_isplaying(int no)
{
	return no == current_no &&
		sdl3_mixer_music_active(SDL3_MIXER_MUSIC_BGM);
}

void musbgm_stopall(int time)
{
	musbgm_stop(current_no, time);
}

void musbgm_wait(int no, int timeout)
{
	while (timeout-- > 0 && musbgm_isplaying(no))
		SDL_Delay(10);
}
