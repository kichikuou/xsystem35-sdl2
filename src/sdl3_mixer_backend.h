#ifndef XSYSTEM35_SDL3_MIXER_BACKEND_H
#define XSYSTEM35_SDL3_MIXER_BACKEND_H

#include "portab.h"
#include "sdl_mixer_compat.h"

enum sdl3_mixer_music_owner {
	SDL3_MIXER_MUSIC_NONE,
	SDL3_MIXER_MUSIC_BGM,
	SDL3_MIXER_MUSIC_CD,
	SDL3_MIXER_MUSIC_MIDI,
};

bool sdl3_mixer_backend_init(int audio_buffer_size);
void sdl3_mixer_backend_exit(void);
MIX_Mixer *sdl3_mixer_backend_get(void);

MIX_Audio *sdl3_mixer_load_music_memory(const void *data, size_t size);
MIX_Audio *sdl3_mixer_load_music_file(const char *path);

/* play_music takes ownership of audio, including when playback fails. */
bool sdl3_mixer_play_music(enum sdl3_mixer_music_owner owner,
	MIX_Audio *audio, int loops, int fade_in_ms, float gain);
void sdl3_mixer_stop_music(enum sdl3_mixer_music_owner owner, int fade_out_ms);
void sdl3_mixer_pause_music(enum sdl3_mixer_music_owner owner);
void sdl3_mixer_resume_music(enum sdl3_mixer_music_owner owner);
bool sdl3_mixer_music_active(enum sdl3_mixer_music_owner owner);
void sdl3_mixer_set_music_gain(enum sdl3_mixer_music_owner owner, float gain);

#endif /* XSYSTEM35_SDL3_MIXER_BACKEND_H */
