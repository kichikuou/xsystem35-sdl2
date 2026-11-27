/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "sdl_compat.h"
#include <SDL_mixer.h>
#include "system.h"
#include "midi.h"
#include "music_private.h"

#define TSF_IMPLEMENTATION
#define TML_IMPLEMENTATION
#include "../vendor/tinysoundfont/tsf.h"
#include "../vendor/tinysoundfont/tml.h"

static SDL_mutex *mutex;
static tsf *synth;
static tml_message *song, *next_event;
static int rate, repeats, track;
static uint64_t frame, duration, fade_left, fade_total;
static float volume = 100, balance = 1, fade_from, fade_target;
static bool playing, paused, backgrounded, fade_stop;
static bool releasing;
static uint64_t release_left;

static float clamp_volume(int value) {
	return value < 0 ? 0 : value > 100 ? 100 : value;
}

static void rewind_song(void) {
	tsf_reset(synth);
	for (int ch = 0; ch < 16; ch++)
		tsf_channel_set_presetnumber(synth, ch, 0, ch == 9);
	next_event = song;
	frame = 0;
	releasing = false;
}

static void stop_locked(void) {
	playing = false;
	fade_left = 0;
	frame = 0;
	if (synth)
		tsf_reset(synth);
	tml_free(song);
	song = next_event = NULL;
}

static void dispatch(tml_message *event) {
	switch (event->type) {
	case TML_PROGRAM_CHANGE:
		tsf_channel_set_presetnumber(synth, event->channel, event->program, event->channel == 9);
		break;
	case TML_NOTE_ON:
		tsf_channel_note_on(synth, event->channel, event->key, event->velocity / 127.0f);
		break;
	case TML_NOTE_OFF:
		tsf_channel_note_off(synth, event->channel, event->key);
		break;
	case TML_PITCH_BEND:
		tsf_channel_set_pitchwheel(synth, event->channel, event->pitch_bend);
		break;
	case TML_CONTROL_CHANGE:
		tsf_channel_midi_control(synth, event->channel, event->control, event->control_value);
		break;
	}
}

/* Called with mutex held; adds stereo floats without touching recorded audio. */
static void render(float *out, int frames) {
	memset(out, 0, frames * 2 * sizeof(float));
	if (!playing || paused || backgrounded || !rate)
		return;
	for (int i = 0; i < frames && playing;) {
		while (next_event && (uint64_t)next_event->time * rate <= frame * 1000) {
			dispatch(next_event);
			next_event = next_event->next;
		}
		if (!releasing && frame >= duration) {
			if (repeats > 0 && --repeats == 0) {
				releasing = true;
				release_left = rate * 2;
				tsf_note_off_all(synth);
			} else {
				rewind_song();
				continue;
			}
		}
		if (releasing && (!release_left || !tsf_active_voice_count(synth))) {
			playing = false;
			fade_left = 0;
			tsf_reset(synth);
			break;
		}
		uint64_t end = frame + frames - i;
		if (releasing && end - frame > release_left)
			end = frame + release_left;
		if (!releasing && end > duration)
			end = duration;
		if (!releasing && next_event) {
			uint64_t event_frame = ((uint64_t)next_event->time * rate + 999) / 1000;
			if (end > event_frame)
				end = event_frame;
		}
		if (fade_left && end - frame > fade_left)
			end = frame + fade_left;
		int count = end - frame;
		if (releasing)
			release_left -= count;
		tsf_render_float(synth, out + i * 2, count, 0);
		for (int j = 0; j < count; j++, i++) {
			if (fade_left) {
				fade_left--;
				volume = fade_target + (fade_from - fade_target) * ((double)fade_left / fade_total);
			}
			float gain = volume * balance / 100.0f;
			out[i * 2] *= gain;
			out[i * 2 + 1] *= gain;
		}
		frame = end;
		if (!fade_left && fade_stop) {
			playing = false;
			fade_stop = false;
		}
	}
}

static void postmix(void *unused, Uint8 *stream, int len) {
	(void)unused;
	SDL_LockMutex(mutex);
	while (len >= 4) {
		float samples[128 * 2];
		int count = len / 4 < 128 ? len / 4 : 128;
		render(samples, count);
		for (int i = 0; i < count * 2; i++) {
			Sint16 old;
			memcpy(&old, stream + i * 2, 2);
			old = SDL_SwapLE16(old);
			float mixed = old + samples[i] * 32767.0f;
			Sint16 value = mixed > 32767 ? 32767 : mixed < -32768 ? -32768 : (Sint16)mixed;
			value = SDL_SwapLE16(value);
			memcpy(stream + i * 2, &value, 2);
		}
		stream += count * 4;
		len -= count * 4;
	}
	SDL_UnlockMutex(mutex);
}

void midi_tsf_audio_ready(void) {
	if (!synth)
		return;
	int frequency, channels;
	Uint16 format;
	if (!Mix_QuerySpec(&frequency, &format, &channels))
		return;
	if (format != AUDIO_S16LSB || channels != 2 || frequency <= 0) {
		WARNING("Unsupported MIDI output format: %d Hz, 0x%x, %d channels", frequency, format, channels);
		return;
	}
	SDL_LockMutex(mutex);
	rate = frequency;
	tsf_set_output(synth, TSF_STEREO_INTERLEAVED, rate, 0);
	SDL_UnlockMutex(mutex);
	Mix_SetPostMix(postmix, NULL);
}

static bool initialize(int subdev) {
	(void)subdev;
	const char *path = getenv("XSYSTEM35_SOUNDFONT");
	if (!path || !*path || !(synth = tsf_load_filename(path))) {
		WARNING("Cannot load MIDI soundfont: %s", path ? path : "XSYSTEM35_SOUNDFONT is unset");
		return false;
	}
	mutex = SDL_CreateMutex();
	if (!mutex) {
		tsf_close(synth);
		synth = NULL;
		return false;
	}
	tsf_set_max_voices(synth, 256);
	volume = 100;
	fade_left = fade_total = 0;
	fade_stop = false;
	paused = backgrounded = playing = releasing = false;
	balance = prv.volval[BGM_VOLVAL_CH] / 100.0f;
	midi_tsf_audio_ready();
	return true;
}

static void stop(void) {
	if (!mutex)
		return;
	SDL_LockMutex(mutex);
	stop_locked();
	SDL_UnlockMutex(mutex);
}

static void shutdown(void) {
	Mix_SetPostMix(NULL, NULL);
	stop();
	if (synth) {
		tsf_close(synth);
		synth = NULL;
	}
	if (mutex) {
		SDL_DestroyMutex(mutex);
		mutex = NULL;
	}
	rate = 0;
	volume = 100;
	fade_left = fade_total = 0;
	fade_stop = false;
}

static bool start(int no, int loop, const uint8_t *data, int length) {
	tml_message *messages = data && length > 0 ? tml_load_memory(data, length) : NULL;
	SDL_LockMutex(mutex);
	stop_locked();
	if (!messages || !rate) {
		SDL_UnlockMutex(mutex);
		tml_free(messages);
		WARNING("Cannot play MIDI: invalid data or audio output unavailable");
		return false;
	}
	unsigned int milliseconds;
	tml_get_info(messages, NULL, NULL, NULL, NULL, &milliseconds);
	duration = ((uint64_t)milliseconds * rate + 999) / 1000;
	if (!duration)
		duration = 1;
	song = messages;
	repeats = loop;
	track = no;
	volume = 100;
	fade_left = fade_total = 0;
	paused = fade_stop = false;
	playing = true;
	rewind_song();
	SDL_UnlockMutex(mutex);
	return true;
}

static void pause_midi(void) {
	SDL_LockMutex(mutex);
	paused = true;
	SDL_UnlockMutex(mutex);
}

static void resume_midi(void) {
	SDL_LockMutex(mutex);
	paused = false;
	SDL_UnlockMutex(mutex);
}

void midi_tsf_pause_all(bool value) {
	if (!mutex)
		return;
	SDL_LockMutex(mutex);
	backgrounded = value;
	SDL_UnlockMutex(mutex);
}

static bool position(midiplaystate *state) {
	SDL_LockMutex(mutex);
	state->in_play = playing;
	state->loc_ms = playing && rate ? frame * 1000 / rate : 0;
	state->play_no = playing ? track : 0;
	SDL_UnlockMutex(mutex);
	return true;
}

static int getflag(int mode, int index) { (void)mode; (void)index; return 0; }
static bool setflag(int mode, int index, int value) { (void)mode; (void)index; (void)value; return false; }

static bool fade(int milliseconds, int target, int stop_after) {
	SDL_LockMutex(mutex);
	fade_target = clamp_volume(target);
	fade_from = volume;
	fade_left = fade_total = milliseconds > 0 ? (uint64_t)milliseconds * rate / 1000 : 0;
	fade_stop = stop_after != 0;
	if (!fade_left) {
		volume = fade_target;
		if (fade_stop)
			playing = false;
	}
	SDL_UnlockMutex(mutex);
	return true;
}

static bool fading(void) {
	SDL_LockMutex(mutex);
	bool result = playing && fade_left != 0;
	SDL_UnlockMutex(mutex);
	return result;
}

static void reapply_volume(void) {
	SDL_LockMutex(mutex);
	balance = clamp_volume(prv.volval[BGM_VOLVAL_CH]) / 100.0f;
	SDL_UnlockMutex(mutex);
}

mididevice_t midi_tsf = {
	.init = initialize, .exit = shutdown, .reset = stop, .start = start,
	.stop = stop, .pause = pause_midi, .unpause = resume_midi, .getpos = position,
	.getflag = getflag, .setflag = setflag, .fadestart = fade, .fading = fading,
	.reapply_volume = reapply_volume,
};
