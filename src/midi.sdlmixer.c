/*
 * midi.sdlmixer.c  midi play with SDL_mixer
 *
 * Copyright (C) 2019 <KichikuouChrome@gmail.com>
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

#include <SDL_mixer.h>

#include "portab.h"
#include "counter.h"
#include "midi.h"

static int midi_initilize(char *pname, int subdev);
static int midi_exit();
static int midi_start(int no, int loop, char *data, int datalen);
static int midi_stop();
static int midi_pause(void);
static int midi_unpause(void);
static int midi_get_playing_info(midiplaystate *st);
static int midi_getflag(int mode, int index);
static int midi_setflag(int mode, int index, int val);
static int midi_setvol(int vol);
static int midi_getvol();

#define midi midi_sdlmixer
mididevice_t midi = {
	midi_initilize,
	midi_exit,
	midi_start,
	midi_stop,
	midi_pause,
	midi_unpause,
	midi_get_playing_info,
	midi_getflag,
	midi_setflag,
	midi_setvol,
	midi_getvol
};

static Mix_Music *mix_music;
static int midino;
static int counter;

static int midi_initilize(char *pname, int subdev) {
	if (Mix_Init(MIX_INIT_MID) != MIX_INIT_MID)
		return NG;
	reset_counter_high(SYSTEMCOUNTER_MIDI, 10, 0);
	return OK;
}

static int midi_exit() {
	Mix_Quit();
	return OK;
}

static int midi_start(int no, int loop, char *data, int datalen) {
	midi_stop();

	SDL_RWops *rwops = SDL_RWFromConstMem(data, datalen);
	mix_music = Mix_LoadMUSType_RW(rwops, MUS_MID, SDL_TRUE /* freesrc */);
	if (!mix_music)
		return NG;

	if (Mix_PlayMusic(mix_music, loop ? loop : -1) != 0) {
		Mix_FreeMusic(mix_music);
		mix_music = NULL;
		return NG;
	}

	midino = no;
	counter = get_high_counter(SYSTEMCOUNTER_MIDI);
	return OK;
}

static int midi_stop() {
	if (!mix_music)
		return OK;
	Mix_FreeMusic(mix_music);
	mix_music = NULL;
	midino = 0;
	return OK;
}

static int midi_pause(void) {
	// FIXME: adjust counter
	Mix_PauseMusic();
	return OK;
}

static int midi_unpause(void) {
	// FIXME: adjust counter
	Mix_ResumeMusic();
	return OK;
}

static int midi_get_playing_info(midiplaystate *st) {
	if (midino == 0) {
		st->in_play = FALSE;
		st->play_no = 0;
		st->loc_ms  = 0;
		return OK;
	}

	int cnt = get_high_counter(SYSTEMCOUNTER_MIDI) - counter;

	st->in_play = TRUE;
	st->play_no = midino;
	st->loc_ms = cnt * 10;
	return OK;
}

static int midi_getflag(int mode, int index) {
	return 0;
}

static int midi_setflag(int mode, int index, int val) {
	return NG;
}

static int midi_setvol(int vol) {
	Mix_VolumeMusic(vol * MIX_MAX_VOLUME / 100);
	return OK;
}

static int midi_getvol() {
	return Mix_VolumeMusic(-1) * 100 / MIX_MAX_VOLUME;
}