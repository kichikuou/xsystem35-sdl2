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

// Audio playback through SDL_mixer. A sound is decoded into memory and plays
// on one of the numbered channels. A music is decoded while it plays.
//
// The functions report errors through SDL_GetError().

#ifndef __MIXER_H__
#define __MIXER_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Opens the audio device and allocates channels 0 to num_channels - 1.
// buffer_size is the device buffer size in sample frames.
bool mixer_init(int num_channels, int buffer_size);
void mixer_quit(void);

// Enables the MIDI decoder.
bool mixer_init_midi(void);
void mixer_quit_midi(void);

typedef struct mixer_sound mixer_sound;

// Loads a WAV, Ogg Vorbis or MP3 file.
mixer_sound *mixer_sound_load(const void *data, size_t size);
// Creates a sound from samples in the format that mixer_decode() returns.
// Takes ownership of samples, which must be allocated with malloc(), also when
// it fails.
mixer_sound *mixer_sound_from_samples(int16_t *samples, size_t frames);
void mixer_sound_free(mixer_sound *sound);
int mixer_sound_duration_ms(mixer_sound *sound);

// Decodes a WAV, Ogg Vorbis or MP3 file to interleaved 16-bit stereo samples.
// All results have the same sample rate. Free the result with free().
int16_t *mixer_decode(const void *data, size_t size, size_t *frames);

// Plays the sound 1 + loops times. If loops is -1, plays it forever.
bool mixer_channel_play(int channel, mixer_sound *sound, int loops);
void mixer_channel_halt(int channel);
void mixer_channel_fade_out(int channel, int ms);
void mixer_channel_pause(int channel);
void mixer_channel_resume(int channel);
// Returns true while a sound plays on the channel, also when it is paused.
bool mixer_channel_playing(int channel);
// gain is from 0.0 to 1.0.
void mixer_channel_set_gain(int channel, float gain);

typedef struct mixer_music mixer_music;

// Keep data valid until the music is freed.
mixer_music *mixer_music_load(const void *data, size_t size);
mixer_music *mixer_music_load_midi(const void *data, size_t size);
mixer_music *mixer_music_load_file(const char *path);
void mixer_music_free(mixer_music *music);

// Plays the music play_count times. If play_count is -1, plays it forever.
// A play_count of 0 is the same as 1. With SDL2_mixer, this stops the music
// that is playing.
bool mixer_music_play(mixer_music *music, int play_count, int fade_in_ms);
void mixer_music_fade_out(mixer_music *music, int ms);
void mixer_music_pause(mixer_music *music);
void mixer_music_resume(mixer_music *music);
// Returns true while the music plays, also when it is paused.
bool mixer_music_playing(mixer_music *music);
// gain is from 0.0 to 1.0.
void mixer_music_set_gain(mixer_music *music, float gain);

#endif /* __MIXER_H__ */
