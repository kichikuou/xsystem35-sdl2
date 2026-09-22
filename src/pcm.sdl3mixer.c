/*
 * pcm.sdl3mixer.c  PCM audio using SDL3_mixer
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

#include "sdl_mixer_compat.h"

#include "ald_manager.h"
#include "audio_meta.h"
#include "dri.h"
#include "music_pcm.h"
#include "music_private.h"
#include "nact.h"
#include "sdl3_mixer_backend.h"
#include "sdl3_mixer_utils.h"
#include "system.h"

#define SAMPLE_RATE 44100
#define BYTES_PER_SAMPLE_FRAME 4
#define PCM_SLOTS (128 + 1)
#define DECODE_BLOCK_SIZE 16384

static struct {
	MIX_Audio *audio;
	MIX_Track *track;
	uint32_t start_time;
	int volume;
} slots[PCM_SLOTS];

struct decoded_audio {
	uint8_t *data;
	size_t length;
};

static void apply_volume(int slot)
{
	if (!slots[slot].track)
		return;
	int channel = prv.vol_pcm_sub[slot];
	if ((unsigned)channel >= 16)
		channel = 0;
	MIX_SetTrackGain(slots[slot].track,
		sdl3_mixer_gain(slots[slot].volume, prv.volval[channel]));
}

static MIX_Audio *load_audio(const void *data, size_t size)
{
	SDL_IOStream *io = SDL_IOFromConstMem(data, size);
	if (!io)
		return NULL;
	return MIX_LoadAudio_IO(sdl3_mixer_backend_get(), io, true, true);
}

static MIX_Audio *load_asset(DRIFILETYPE type, int no)
{
	dridata *dfile = ald_getdata(type, no - 1);
	if (!dfile) {
		WARNING("Audio asset fail to open %d", no - 1);
		return NULL;
	}

	MIX_Audio *audio = load_audio(dfile->data, dfile->size);
	ald_freedata(dfile);
	if (!audio)
		WARNING("Audio asset %d is not valid: %s", no - 1, SDL_GetError());
	return audio;
}

static bool decode_asset(DRIFILETYPE type, int no, struct decoded_audio *out)
{
	dridata *dfile = ald_getdata(type, no - 1);
	if (!dfile)
		return false;

	SDL_IOStream *io = SDL_IOFromConstMem(dfile->data, dfile->size);
	MIX_AudioDecoder *decoder = io
		? MIX_CreateAudioDecoder_IO(io, true, 0) : NULL;
	if (!decoder) {
		ald_freedata(dfile);
		return false;
	}

	SDL_AudioSpec spec = {
		.format = SDL_AUDIO_S16LE,
		.channels = 2,
		.freq = SAMPLE_RATE,
	};
	uint8_t *data = NULL;
	size_t length = 0;
	bool success = true;
	for (;;) {
		if (length > SIZE_MAX - DECODE_BLOCK_SIZE) {
			success = false;
			break;
		}
		uint8_t *new_data = SDL_realloc(data, length + DECODE_BLOCK_SIZE);
		if (!new_data) {
			success = false;
			break;
		}
		data = new_data;
		int decoded = MIX_DecodeAudio(decoder, data + length,
			DECODE_BLOCK_SIZE, &spec);
		if (decoded < 0 || decoded % BYTES_PER_SAMPLE_FRAME != 0) {
			success = false;
			break;
		}
		if (decoded == 0)
			break;
		length += decoded;
	}

	MIX_DestroyAudioDecoder(decoder);
	ald_freedata(dfile);
	if (!success || length == 0) {
		SDL_free(data);
		return false;
	}
	out->data = data;
	out->length = length;
	return true;
}

static MIX_Audio *mix_lr(int no_left, int no_right)
{
	struct decoded_audio left = {0};
	struct decoded_audio right = {0};
	if (!decode_asset(DRIFILE_WAVE, no_left, &left) ||
	    !decode_asset(DRIFILE_WAVE, no_right, &right)) {
		SDL_free(left.data);
		SDL_free(right.data);
		return NULL;
	}

	size_t length = max(left.length, right.length);
	int16_t *mixed = SDL_calloc(1, length);
	if (!mixed) {
		SDL_free(left.data);
		SDL_free(right.data);
		return NULL;
	}

	int16_t *left_samples = (int16_t *)left.data;
	int16_t *right_samples = (int16_t *)right.data;
	for (size_t i = 0; i < left.length / BYTES_PER_SAMPLE_FRAME; i++)
		mixed[i * 2] = left_samples[i * 2];
	for (size_t i = 0; i < right.length / BYTES_PER_SAMPLE_FRAME; i++)
		mixed[i * 2 + 1] = right_samples[i * 2 + 1];

	SDL_AudioSpec spec = {
		.format = SDL_AUDIO_S16LE,
		.channels = 2,
		.freq = SAMPLE_RATE,
	};
	MIX_Audio *audio = MIX_LoadRawAudio(
		sdl3_mixer_backend_get(), mixed, length, &spec);
	SDL_free(mixed);
	SDL_free(left.data);
	SDL_free(right.data);
	return audio;
}

static bool load_slot(int slot, MIX_Audio *audio)
{
	if (!audio)
		return false;
	muspcm_unload(slot);
	if (!MIX_SetTrackAudio(slots[slot].track, audio)) {
		MIX_DestroyAudio(audio);
		return false;
	}
	slots[slot].audio = audio;
	apply_volume(slot);
	return true;
}

bool muspcm_init(int audio_buffer_size)
{
	(void)audio_buffer_size;
	MIX_Mixer *mixer = sdl3_mixer_backend_get();
	if (!mixer)
		return false;

	for (int i = 0; i < PCM_SLOTS; i++) {
		slots[i].track = MIX_CreateTrack(mixer);
		slots[i].volume = 100;
		if (!slots[i].track) {
			muspcm_exit();
			return false;
		}
	}
	wai_load(nact->files.wai);
	return true;
}

void muspcm_exit(void)
{
	for (int i = 0; i < PCM_SLOTS; i++) {
		muspcm_unload(i);
		MIX_DestroyTrack(slots[i].track);
		slots[i].track = NULL;
	}
}

void muspcm_reset(void)
{
	for (int i = 0; i < PCM_SLOTS; i++)
		muspcm_unload(i);
}

bool muspcm_load_no(int slot, int no)
{
	if ((unsigned)slot >= PCM_SLOTS)
		return false;
	MIX_Audio *audio = load_asset(DRIFILE_WAVE, no);
	if (!audio)
		return false;
	if (wai_loaded()) {
		int channel = wai_mixch(no);
		prv.vol_pcm_sub[slot] = channel < 0 ? 0 : channel;
	} else {
		prv.vol_pcm_sub[slot] = SE_VOLVAL_CH;
	}
	return load_slot(slot, audio);
}

bool muspcm_load_bgm(int slot, int no)
{
	if ((unsigned)slot >= PCM_SLOTS)
		return false;
	MIX_Audio *audio = load_asset(DRIFILE_BGM, no);
	if (!audio)
		return false;
	prv.vol_pcm_sub[slot] = 0;
	return load_slot(slot, audio);
}

bool muspcm_load_mixlr(int slot, int no_left, int no_right)
{
	if ((unsigned)slot >= PCM_SLOTS)
		return false;
	MIX_Audio *audio = mix_lr(no_left, no_right);
	if (!audio)
		return false;
	prv.vol_pcm_sub[slot] = SE_VOLVAL_CH;
	return load_slot(slot, audio);
}

bool muspcm_load_data(int slot, uint8_t *data, uint32_t length)
{
	if ((unsigned)slot >= PCM_SLOTS)
		return false;
	MIX_Audio *audio = load_audio(data, length);
	if (!audio)
		return false;
	prv.vol_pcm_sub[slot] = SE_VOLVAL_CH;
	return load_slot(slot, audio);
}

void muspcm_unload(int slot)
{
	if ((unsigned)slot >= PCM_SLOTS || !slots[slot].audio)
		return;
	MIX_StopTrack(slots[slot].track, 0);
	MIX_SetTrackAudio(slots[slot].track, NULL);
	MIX_DestroyAudio(slots[slot].audio);
	slots[slot].audio = NULL;
}

bool muspcm_start(int slot, int loop)
{
	if ((unsigned)slot >= PCM_SLOTS || !slots[slot].audio)
		return false;
	SDL_PropertiesID props = SDL_CreateProperties();
	if (!props)
		return false;
	bool success = SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER,
		loop <= 0 ? -1 : loop - 1) &&
		MIX_PlayTrack(slots[slot].track, props);
	SDL_DestroyProperties(props);
	if (success)
		slots[slot].start_time = sys_get_ticks();
	return success;
}

void muspcm_stop(int slot)
{
	if ((unsigned)slot < PCM_SLOTS)
		MIX_StopTrack(slots[slot].track, 0);
}

void muspcm_fadeout(int slot, int msec)
{
	if ((unsigned)slot >= PCM_SLOTS || !slots[slot].audio)
		return;
	Sint64 frames = msec > 0
		? MIX_TrackMSToFrames(slots[slot].track, msec) : 0;
	if (frames < 0)
		frames = 0;
	MIX_StopTrack(slots[slot].track, frames);
}

void muspcm_pause(int slot)
{
	if ((unsigned)slot < PCM_SLOTS)
		MIX_PauseTrack(slots[slot].track);
}

void muspcm_unpause(int slot)
{
	if ((unsigned)slot < PCM_SLOTS)
		MIX_ResumeTrack(slots[slot].track);
}

static bool slot_active(int slot)
{
	return MIX_TrackPlaying(slots[slot].track) ||
		MIX_TrackPaused(slots[slot].track);
}

int muspcm_getpos(int slot)
{
	if ((unsigned)slot >= PCM_SLOTS || !slot_active(slot))
		return 0;
	int position = sys_get_ticks() - slots[slot].start_time;
	return position ? position : 1;
}

void muspcm_setvol(int slot, int volume)
{
	if ((unsigned)slot >= PCM_SLOTS)
		return;
	slots[slot].volume = volume;
	apply_volume(slot);
}

int muspcm_getwavelen(int slot)
{
	if ((unsigned)slot >= PCM_SLOTS || !slots[slot].audio)
		return 0;
	Sint64 frames = MIX_GetAudioDuration(slots[slot].audio);
	if (frames < 0)
		return 0;
	Sint64 length = MIX_AudioFramesToMS(slots[slot].audio, frames);
	if (length < 0)
		return 0;
	return length > 65535 ? 65535 : (int)length;
}

bool muspcm_isplaying(int slot)
{
	return (unsigned)slot < PCM_SLOTS && slot_active(slot);
}

void muspcm_waitend(int slot)
{
	(void)slot;
	WARNING("not implemented");
}

void muspcm_reapply_valance(void)
{
	for (int slot = 0; slot < PCM_SLOTS; slot++)
		apply_volume(slot);
}
