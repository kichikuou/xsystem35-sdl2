/*
 * Copyright (C) 2026 kichikuou <KichikuouChrome@gmail.com>
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

#include <stdlib.h>
#include <string.h>
#include "sdl_compat.h"
#include <SDL_mixer.h>

#include "mixer.h"

// MIX_INIT_FLUIDSYNTH was renamed to MIX_INIT_MID in SDL_mixer 2.0.2
#if (SDL_MIXER_MAJOR_VERSION == 2) && (SDL_MIXER_MINOR_VERSION == 0) && (SDL_MIXER_PATCHLEVEL < 2)
#define MIX_INIT_MID MIX_INIT_FLUIDSYNTH
#endif

#define SAMPLE_RATE 44100
#define BYTES_PER_FRAME 4

struct mixer_sound {
	Mix_Chunk *chunk;
	int16_t *samples;  // owned by this sound if not NULL
};

// SDL_mixer plays one Mix_Music at a time. These functions control the music
// only while it is the one that played last.
struct mixer_music {
	Mix_Music *mus;
	float gain;
};

static int output_rate = SAMPLE_RATE;
static mixer_music *current_music;

static int to_mix_volume(float gain) {
	return (int)(gain * MIX_MAX_VOLUME + 0.5f);
}

bool mixer_init(int num_channels, int buffer_size) {
	Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG);
	if (Mix_OpenAudioDevice(SAMPLE_RATE, AUDIO_S16LSB, 2, buffer_size, NULL, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE) < 0)
		return false;
	Mix_QuerySpec(&output_rate, NULL, NULL);
	Mix_AllocateChannels(num_channels);
	return true;
}

void mixer_quit(void) {
	Mix_CloseAudio();
	Mix_Quit();
}

bool mixer_init_midi(void) {
	return Mix_Init(MIX_INIT_MID) == MIX_INIT_MID;
}

void mixer_quit_midi(void) {
	Mix_Quit();
}

static mixer_sound *sound_new(Mix_Chunk *chunk, int16_t *samples) {
	mixer_sound *sound = malloc(sizeof(mixer_sound));
	sound->chunk = chunk;
	sound->samples = samples;
	return sound;
}

mixer_sound *mixer_sound_load(const void *data, size_t size) {
	Mix_Chunk *chunk = Mix_LoadWAV_RW(SDL_IOFromConstMem(data, size), 1);
	if (!chunk)
		return NULL;
	return sound_new(chunk, NULL);
}

mixer_sound *mixer_sound_from_samples(int16_t *samples, size_t frames) {
	Mix_Chunk *chunk = Mix_QuickLoad_RAW((Uint8 *)samples, frames * BYTES_PER_FRAME);
	if (!chunk) {
		free(samples);
		return NULL;
	}
	return sound_new(chunk, samples);
}

void mixer_sound_free(mixer_sound *sound) {
	Mix_FreeChunk(sound->chunk);
	free(sound->samples);
	free(sound);
}

int mixer_sound_duration_ms(mixer_sound *sound) {
	return sound->chunk->alen * 1000LL / (output_rate * BYTES_PER_FRAME);
}

int16_t *mixer_decode(const void *data, size_t size, size_t *frames) {
	Mix_Chunk *chunk = Mix_LoadWAV_RW(SDL_IOFromConstMem(data, size), 1);
	if (!chunk)
		return NULL;
	int16_t *samples = malloc(chunk->alen);
	if (samples) {
		memcpy(samples, chunk->abuf, chunk->alen);
		*frames = chunk->alen / BYTES_PER_FRAME;
	}
	Mix_FreeChunk(chunk);
	return samples;
}

bool mixer_channel_play(int channel, mixer_sound *sound, int loops) {
	return Mix_PlayChannel(channel, sound->chunk, loops) >= 0;
}

void mixer_channel_halt(int channel) {
	Mix_HaltChannel(channel);
}

void mixer_channel_fade_out(int channel, int ms) {
	Mix_FadeOutChannel(channel, ms);
}

void mixer_channel_pause(int channel) {
	Mix_Pause(channel);
}

void mixer_channel_resume(int channel) {
	Mix_Resume(channel);
}

bool mixer_channel_playing(int channel) {
	return Mix_Playing(channel);
}

void mixer_channel_set_gain(int channel, float gain) {
	Mix_Volume(channel, to_mix_volume(gain));
}

static mixer_music *music_new(Mix_Music *mus) {
	if (!mus)
		return NULL;
	mixer_music *music = malloc(sizeof(mixer_music));
	music->mus = mus;
	music->gain = 1.0f;
	return music;
}

mixer_music *mixer_music_load(const void *data, size_t size) {
	return music_new(Mix_LoadMUS_RW(SDL_IOFromConstMem(data, size), SDL_TRUE));
}

mixer_music *mixer_music_load_midi(const void *data, size_t size) {
	return music_new(Mix_LoadMUSType_RW(SDL_IOFromConstMem(data, size), MUS_MID, SDL_TRUE));
}

mixer_music *mixer_music_load_file(const char *path) {
	return music_new(Mix_LoadMUS(path));
}

void mixer_music_free(mixer_music *music) {
	if (music == current_music)
		current_music = NULL;
	Mix_FreeMusic(music->mus);
	free(music);
}

bool mixer_music_play(mixer_music *music, int play_count, int fade_in_ms) {
	if (Mix_FadeInMusic(music->mus, play_count, fade_in_ms) != 0)
		return false;
	current_music = music;
	Mix_VolumeMusic(to_mix_volume(music->gain));
	return true;
}

void mixer_music_fade_out(mixer_music *music, int ms) {
	if (music == current_music)
		Mix_FadeOutMusic(ms);
}

void mixer_music_pause(mixer_music *music) {
	if (music == current_music)
		Mix_PauseMusic();
}

void mixer_music_resume(mixer_music *music) {
	if (music == current_music)
		Mix_ResumeMusic();
}

bool mixer_music_playing(mixer_music *music) {
	return music == current_music && Mix_PlayingMusic();
}

void mixer_music_set_gain(mixer_music *music, float gain) {
	music->gain = gain;
	if (music == current_music)
		Mix_VolumeMusic(to_mix_volume(gain));
}
