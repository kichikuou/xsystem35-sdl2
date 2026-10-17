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

#include "sdl_compat.h"

#include "portab.h"
#include "system.h"
#include "midi.h"
#include "music_private.h"
#include "mixer.h"

static void midi_stop(void);

static mixer_music *music;
static int start_time;
static uint32_t fade_tick;
static int current_vol = 100;  // game-requested volume (0-100)

static void apply_volume(void) {
	mixer_music_set_gain(music, current_vol * prv.volval[BGM_VOLVAL_CH] / 10000.0f);
}

static bool midi_initialize(int subdev) {
	return mixer_init_midi();
}

static void midi_exit(void) {
	mixer_quit_midi();
}

static void midi_reset(void) {
	midi_stop();
}

static bool midi_start(int no, int loop, const uint8_t *data, int datalen) {
	midi_stop();

	music = mixer_music_load_midi(data, datalen);
	if (!music) {
		WARNING("Cannot load MIDI: %s", SDL_GetError());
		return false;
	}

	apply_volume();
	if (!mixer_music_play(music, loop ? loop : -1, 0)) {
		WARNING("Cannot play MIDI: %s", SDL_GetError());
		mixer_music_free(music);
		music = NULL;
		return false;
	}

	start_time = SDL_GetTicks();
	return true;
}

static void midi_stop() {
	if (!music)
		return;
	mixer_music_free(music);
	music = NULL;
}

static void midi_pause(void) {
	// FIXME: adjust start_time
	if (music)
		mixer_music_pause(music);
}

static void midi_unpause(void) {
	// FIXME: adjust start_time
	if (music)
		mixer_music_resume(music);
}

static bool midi_get_playing_info(midiplaystate *st) {
	if (!music || !mixer_music_playing(music)) {
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
		current_vol = volume;
		if (music)
			apply_volume();
		if (stop)
			midi_stop();
		return true;
	}

	if (volume == 0) {
		if (music)
			mixer_music_fade_out(music, time);  // FIXME: this always stops the music
		fade_tick = SDL_GetTicks() + time;
		return true;
	}
	WARNING("(time=%d, volume=%d, stop=%d) unsupported", time, volume, stop);
	return false;
}

static bool midi_fading(void) {
	return SDL_GetTicks() < fade_tick;
}

static void midi_reapply_volume(void) {
	if (music)
		apply_volume();
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
	.reapply_volume = midi_reapply_volume,
};
