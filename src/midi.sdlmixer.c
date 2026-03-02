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

#include <SDL.h>
#include <SDL_mixer.h>

#include "portab.h"
#include "system.h"
#include "midi.h"

// MIX_INIT_FLUIDSYNTH was renamed to MIX_INIT_MID in SDL_mixer 2.0.2
#if (SDL_MIXER_MAJOR_VERSION == 2) && (SDL_MIXER_MINOR_VERSION == 0) && (SDL_MIXER_PATCHLEVEL < 2)
#define MIX_INIT_MID MIX_INIT_FLUIDSYNTH
#endif

static void midi_stop(void);

static Mix_Music *mix_music;
static int start_time;
static uint32_t fade_tick;

static bool midi_initialize(int subdev) {
	if (Mix_Init(MIX_INIT_MID) != MIX_INIT_MID)
		return false;
	return true;
}

static void midi_exit(void) {
	Mix_Quit();
}

static void midi_reset(void) {
	midi_stop();
}

static bool midi_start(int no, int loop, const uint8_t *data, int datalen) {
	midi_stop();

	SDL_RWops *rwops = SDL_RWFromConstMem(data, datalen);
	mix_music = Mix_LoadMUSType_RW(rwops, MUS_MID, SDL_TRUE /* freesrc */);
	if (!mix_music) {
		WARNING("Cannot load MIDI: %s", SDL_GetError());
		return false;
	}

	if (Mix_PlayMusic(mix_music, loop ? loop : -1) != 0) {
		WARNING("Cannot play MIDI: %s", SDL_GetError());
		Mix_FreeMusic(mix_music);
		mix_music = NULL;
		return false;
	}

	start_time = SDL_GetTicks();
	return true;
}

static void midi_stop() {
	if (!mix_music)
		return;
	Mix_FreeMusic(mix_music);
	mix_music = NULL;
}

static void midi_pause(void) {
	// FIXME: adjust start_time
	Mix_PauseMusic();
}

static void midi_unpause(void) {
	// FIXME: adjust start_time
	Mix_ResumeMusic();
}

static bool midi_get_playing_info(midiplaystate *st) {
	if (!mix_music || !Mix_PlayingMusic()) {
		st->in_play = false;
		st->loc_ms  = 0;
		return true;
	}

	st->in_play = true;
	st->loc_ms = SDL_GetTicks() - start_time;
	return true;
}

static int midi_getflag(int mode, int index) {
	return 0;
}

static bool midi_setflag(int mode, int index, int val) {
	return false;
}

static bool midi_fadestart(int time, int volume, int stop) {
	if (time == 0) {
		Mix_VolumeMusic(volume * MIX_MAX_VOLUME / 100);
		if (stop)
			midi_stop();
		return true;
	}

	if (volume == 0) {
		Mix_FadeOutMusic(time);  // FIXME: this always stops the music
		fade_tick = SDL_GetTicks() + time;
		return true;
	}
	WARNING("(time=%d, volume=%d, stop=%d) unsupported", time, volume, stop);
	return false;
}

static bool midi_fading(void) {
	return SDL_GetTicks() < fade_tick;
}

mididevice_t midi_sdlmixer = {
	.init = midi_initialize,
	.exit = midi_exit,
	.reset = midi_reset,
	.start = midi_start,
	.stop = midi_stop,
	.pause = midi_pause,
	.unpause = midi_unpause,
	.getpos = midi_get_playing_info,
	.getflag = midi_getflag,
	.setflag = midi_setflag,
	.fadestart = midi_fadestart,
	.fading = midi_fading,
};
