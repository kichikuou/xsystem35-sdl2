/*
 * midi.sdl3mixer.c  MIDI playback using SDL3_mixer
 *
 * Copyright (C) 2019 <KichikuouChrome@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <stdlib.h>
#include <string.h>

#include "midi.h"
#include "music_private.h"
#include "sdl3_mixer_backend.h"
#include "system.h"

#define FLUIDSYNTH_SOUNDFONT_PATH \
	"SDL_mixer.decoder.fluidsynth.soundfont_path"

static uint32_t start_time;
static uint32_t fade_tick;
static int current_vol = 100;

static bool has_decoder(const char *name)
{
	for (int i = 0; i < MIX_GetNumAudioDecoders(); i++) {
		if (!strcmp(MIX_GetAudioDecoder(i), name))
			return true;
	}
	return false;
}

static int clamp_volume(int volume)
{
	if (volume < 0)
		return 0;
	if (volume > 100)
		return 100;
	return volume;
}

static float music_gain(void)
{
	float gain = clamp_volume(current_vol) / 100.0f;
	return gain * clamp_volume(prv.volval[BGM_VOLVAL_CH]) / 100.0f;
}

static void apply_volume(void)
{
	sdl3_mixer_set_music_gain(SDL3_MIXER_MUSIC_MIDI, music_gain());
}

static char *last_soundfont_path(const char *paths)
{
	const char *last = NULL;
	size_t last_length = 0;
	const char *segment = paths;
	for (const char *p = paths;; p++) {
		if (*p != ';' && *p)
			continue;
		if (p != segment) {
			last = segment;
			last_length = p - segment;
		}
		if (!*p)
			break;
		segment = p + 1;
	}
	return last ? SDL_strndup(last, last_length) : NULL;
}

static MIX_Audio *load_midi(const uint8_t *data, int length)
{
	if (length <= 0)
		return NULL;
	SDL_IOStream *io = SDL_IOFromConstMem(data, length);
	SDL_PropertiesID props = SDL_CreateProperties();
	if (!io || !props) {
		if (io)
			SDL_CloseIO(io);
		if (props)
			SDL_DestroyProperties(props);
		return NULL;
	}

	bool success =
		SDL_SetPointerProperty(props,
			MIX_PROP_AUDIO_LOAD_IOSTREAM_POINTER, io) &&
		SDL_SetBooleanProperty(props,
			MIX_PROP_AUDIO_LOAD_CLOSEIO_BOOLEAN, true) &&
		SDL_SetPointerProperty(props,
			MIX_PROP_AUDIO_LOAD_PREFERRED_MIXER_POINTER,
			sdl3_mixer_backend_get());

	const char *soundfonts = getenv("SDL_SOUNDFONTS");
	const char *timidity_cfg = getenv("TIMIDITY_CFG");
	char *soundfont_path = NULL;
	if (success && soundfonts && *soundfonts && has_decoder("FLUIDSYNTH")) {
		soundfont_path = last_soundfont_path(soundfonts);
		success = soundfont_path &&
			SDL_SetStringProperty(props, MIX_PROP_AUDIO_DECODER_STRING,
				"FLUIDSYNTH") &&
			SDL_SetStringProperty(props, FLUIDSYNTH_SOUNDFONT_PATH,
				soundfont_path);
	} else if (success && timidity_cfg && *timidity_cfg &&
		   has_decoder("TIMIDITY")) {
		success = SDL_SetStringProperty(props, MIX_PROP_AUDIO_DECODER_STRING,
			"TIMIDITY");
	}

	MIX_Audio *audio = success ? MIX_LoadAudioWithProperties(props) : NULL;
	if (!success)
		SDL_CloseIO(io);
	SDL_free(soundfont_path);
	SDL_DestroyProperties(props);
	return audio;
}

static bool midi_initialize(int subdevice)
{
	(void)subdevice;
	return sdl3_mixer_backend_get() &&
		(has_decoder("FLUIDSYNTH") || has_decoder("TIMIDITY"));
}

static void midi_stop(void)
{
	sdl3_mixer_stop_music(SDL3_MIXER_MUSIC_MIDI, 0);
}

static void midi_exit(void)
{
	midi_stop();
}

static void midi_reset(void)
{
	midi_stop();
}

static bool midi_start(int no, int loop, const uint8_t *data, int length)
{
	(void)no;
	midi_stop();
	MIX_Audio *audio = load_midi(data, length);
	if (!audio) {
		WARNING("Cannot load MIDI: %s", SDL_GetError());
		return false;
	}

	/* MIDI passes loop directly; zero means infinite playback. */
	int loops = loop == 0 ? -1 : loop;
	if (!sdl3_mixer_play_music(SDL3_MIXER_MUSIC_MIDI, audio,
		loops, 0, music_gain())) {
		WARNING("Cannot play MIDI: %s", SDL_GetError());
		return false;
	}
	start_time = sys_get_ticks();
	return true;
}

static void midi_pause(void)
{
	sdl3_mixer_pause_music(SDL3_MIXER_MUSIC_MIDI);
}

static void midi_unpause(void)
{
	sdl3_mixer_resume_music(SDL3_MIXER_MUSIC_MIDI);
}

static bool midi_get_playing_info(midiplaystate *state)
{
	if (!sdl3_mixer_music_active(SDL3_MIXER_MUSIC_MIDI)) {
		state->in_play = false;
		state->loc_ms = 0;
		return true;
	}
	state->in_play = true;
	state->loc_ms = sys_get_ticks() - start_time;
	return true;
}

static int midi_getflag(int mode, int index)
{
	(void)mode;
	(void)index;
	return 0;
}

static bool midi_setflag(int mode, int index, int value)
{
	(void)mode;
	(void)index;
	(void)value;
	return false;
}

static bool midi_fadestart(int time, int volume, int stop)
{
	if (time == 0) {
		current_vol = volume;
		apply_volume();
		if (stop)
			midi_stop();
		return true;
	}

	if (volume == 0) {
		/* Match SDL2_mixer: a fade to silence always stops the music. */
		sdl3_mixer_stop_music(SDL3_MIXER_MUSIC_MIDI, time);
		fade_tick = sys_get_ticks() + time;
		return true;
	}
	WARNING("(time=%d, volume=%d, stop=%d) unsupported",
		time, volume, stop);
	return false;
}

static bool midi_fading(void)
{
	return sys_get_ticks() < fade_tick;
}

static void midi_reapply_volume(void)
{
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
