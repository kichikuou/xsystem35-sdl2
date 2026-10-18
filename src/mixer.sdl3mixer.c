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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sdl_compat.h"
#include <SDL3_mixer/SDL_mixer.h>

#include "mixer.h"

#define BYTES_PER_FRAME 4

static const SDL_AudioSpec sample_spec = { SDL_AUDIO_S16LE, 2, 44100 };

struct mixer_sound {
	MIX_Audio *audio;
};

struct mixer_music {
	MIX_Audio *audio;
	MIX_Track *track;
};

static MIX_Mixer *mixer;
static MIX_Track **channels;
static int num_channels;

bool mixer_init(int n, int buffer_size) {
	if (!MIX_Init())
		return false;
	char buf[16];
	snprintf(buf, sizeof(buf), "%d", buffer_size);
	SDL_SetHint(SDL_HINT_AUDIO_DEVICE_SAMPLE_FRAMES, buf);
	mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &sample_spec);
	if (!mixer) {
		MIX_Quit();
		return false;
	}
	channels = calloc(n, sizeof(MIX_Track *));
	num_channels = n;
	for (int i = 0; i < n; i++) {
		channels[i] = MIX_CreateTrack(mixer);
		if (!channels[i]) {
			mixer_quit();
			return false;
		}
	}
	return true;
}

void mixer_quit(void) {
	for (int i = 0; i < num_channels; i++)
		MIX_DestroyTrack(channels[i]);
	free(channels);
	channels = NULL;
	num_channels = 0;
	MIX_DestroyMixer(mixer);
	mixer = NULL;
	MIX_Quit();
}

bool mixer_init_midi(void) {
	if (!MIX_Init())
		return false;
	for (int i = 0; i < MIX_GetNumAudioDecoders(); i++) {
		const char *name = MIX_GetAudioDecoder(i);
		if (!strcmp(name, "FLUIDSYNTH") || !strcmp(name, "TIMIDITY"))
			return true;
	}
	SDL_SetError("No MIDI decoder is available");
	MIX_Quit();
	return false;
}

void mixer_quit_midi(void) {
	MIX_Quit();
}

static mixer_sound *sound_new(MIX_Audio *audio) {
	if (!audio)
		return NULL;
	mixer_sound *sound = malloc(sizeof(mixer_sound));
	sound->audio = audio;
	return sound;
}

mixer_sound *mixer_sound_load(const void *data, size_t size) {
	return sound_new(MIX_LoadAudio_IO(mixer, SDL_IOFromConstMem(data, size), true, true));
}

mixer_sound *mixer_sound_from_samples(int16_t *samples, size_t frames) {
	MIX_Audio *audio = MIX_LoadRawAudio(mixer, samples, frames * BYTES_PER_FRAME, &sample_spec);
	free(samples);
	return sound_new(audio);
}

void mixer_sound_free(mixer_sound *sound) {
	MIX_DestroyAudio(sound->audio);
	free(sound);
}

int mixer_sound_duration_ms(mixer_sound *sound) {
	Sint64 frames = MIX_GetAudioDuration(sound->audio);
	if (frames < 0)
		return 0;
	return MIX_AudioFramesToMS(sound->audio, frames);
}

int16_t *mixer_decode(const void *data, size_t size, size_t *frames) {
	MIX_AudioDecoder *decoder = MIX_CreateAudioDecoder_IO(SDL_IOFromConstMem(data, size), true, 0);
	if (!decoder)
		return NULL;

	size_t capacity = 65536;
	size_t len = 0;
	uint8_t *buf = malloc(capacity);
	while (buf) {
		if (capacity - len < 4096) {
			uint8_t *p = realloc(buf, capacity * 2);
			if (!p) {
				free(buf);
				buf = NULL;
				break;
			}
			buf = p;
			capacity *= 2;
		}
		int n = MIX_DecodeAudio(decoder, buf + len, capacity - len, &sample_spec);
		if (n <= 0) {
			if (n < 0) {
				free(buf);
				buf = NULL;
			}
			break;
		}
		len += n;
	}
	MIX_DestroyAudioDecoder(decoder);
	if (!buf)
		return NULL;
	*frames = len / BYTES_PER_FRAME;
	return (int16_t *)buf;
}

static bool play_track(MIX_Track *track, int loops, int fade_in_ms) {
	SDL_PropertiesID props = SDL_CreateProperties();
	SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, loops);
	if (fade_in_ms > 0)
		SDL_SetNumberProperty(props, MIX_PROP_PLAY_FADE_IN_MILLISECONDS_NUMBER, fade_in_ms);
	bool ok = MIX_PlayTrack(track, props);
	SDL_DestroyProperties(props);
	return ok;
}

static void stop_track(MIX_Track *track, int fade_out_ms) {
	MIX_StopTrack(track, fade_out_ms > 0 ? MIX_TrackMSToFrames(track, fade_out_ms) : 0);
}

static bool track_playing(MIX_Track *track) {
	return MIX_TrackPlaying(track) || MIX_TrackPaused(track);
}

bool mixer_channel_play(int channel, mixer_sound *sound, int loops) {
	MIX_Track *track = channels[channel];
	return MIX_SetTrackAudio(track, sound->audio) && play_track(track, loops, 0);
}

void mixer_channel_halt(int channel) {
	MIX_StopTrack(channels[channel], 0);
}

void mixer_channel_fade_out(int channel, int ms) {
	stop_track(channels[channel], ms);
}

void mixer_channel_pause(int channel) {
	MIX_PauseTrack(channels[channel]);
}

void mixer_channel_resume(int channel) {
	MIX_ResumeTrack(channels[channel]);
}

bool mixer_channel_playing(int channel) {
	return track_playing(channels[channel]);
}

void mixer_channel_set_gain(int channel, float gain) {
	MIX_SetTrackGain(channels[channel], gain);
}

static mixer_music *music_new(MIX_Audio *audio) {
	if (!audio)
		return NULL;
	MIX_Track *track = MIX_CreateTrack(mixer);
	if (!track || !MIX_SetTrackAudio(track, audio)) {
		MIX_DestroyTrack(track);
		MIX_DestroyAudio(audio);
		return NULL;
	}
	mixer_music *music = malloc(sizeof(mixer_music));
	music->audio = audio;
	music->track = track;
	return music;
}

mixer_music *mixer_music_load(const void *data, size_t size) {
	if (!mixer) {
		SDL_SetError("Audio device is not open");
		return NULL;
	}
	return music_new(MIX_LoadAudioNoCopy(mixer, data, size, false));
}

// Returns the first path in the SDL_SOUNDFONTS environment variable, which
// SDL2_mixer also reads.
static char *soundfont_path(void) {
	const char *env = SDL_getenv("SDL_SOUNDFONTS");
	if (!env || !*env)
		return NULL;
#ifdef _WIN32
	const char *sep = strchr(env, ';');
#else
	const char *sep = strchr(env, ':');
#endif
	return sep ? SDL_strndup(env, sep - env) : SDL_strdup(env);
}

mixer_music *mixer_music_load_midi(const void *data, size_t size) {
	if (!mixer) {
		SDL_SetError("Audio device is not open");
		return NULL;
	}
	SDL_PropertiesID props = SDL_CreateProperties();
	SDL_SetPointerProperty(props, MIX_PROP_AUDIO_LOAD_IOSTREAM_POINTER, SDL_IOFromConstMem(data, size));
	SDL_SetBooleanProperty(props, MIX_PROP_AUDIO_LOAD_CLOSEIO_BOOLEAN, true);
	SDL_SetPointerProperty(props, MIX_PROP_AUDIO_LOAD_PREFERRED_MIXER_POINTER, mixer);
	char *sf = soundfont_path();
	if (sf) {
		// SDL3_mixer documents this name in README-migration.md, but does not
		// define a macro for it.
		SDL_SetStringProperty(props, "SDL_mixer.decoder.fluidsynth.soundfont_path", sf);
		SDL_free(sf);
	}
	MIX_Audio *audio = MIX_LoadAudioWithProperties(props);
	SDL_DestroyProperties(props);
	return music_new(audio);
}

mixer_music *mixer_music_load_file(const char *path) {
	if (!mixer) {
		SDL_SetError("Audio device is not open");
		return NULL;
	}
	return music_new(MIX_LoadAudio(mixer, path, false));
}

void mixer_music_free(mixer_music *music) {
	MIX_DestroyTrack(music->track);
	MIX_DestroyAudio(music->audio);
	free(music);
}

bool mixer_music_play(mixer_music *music, int play_count, int fade_in_ms) {
	int loops = play_count < 0 ? -1 : play_count > 1 ? play_count - 1 : 0;
	return play_track(music->track, loops, fade_in_ms);
}

void mixer_music_fade_out(mixer_music *music, int ms) {
	stop_track(music->track, ms);
}

void mixer_music_pause(mixer_music *music) {
	MIX_PauseTrack(music->track);
}

void mixer_music_resume(mixer_music *music) {
	MIX_ResumeTrack(music->track);
}

bool mixer_music_playing(mixer_music *music) {
	return track_playing(music->track);
}

void mixer_music_set_gain(mixer_music *music, float gain) {
	MIX_SetTrackGain(music->track, gain);
}
