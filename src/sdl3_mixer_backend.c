#include "sdl3_mixer_backend.h"

#include "system.h"

#define SAMPLE_RATE 44100

static MIX_Mixer *mixer;

bool sdl3_mixer_backend_init(int audio_buffer_size)
{
	if (mixer)
		return true;

	if (audio_buffer_size > 0) {
		char value[32];
		SDL_snprintf(value, sizeof(value), "%d", audio_buffer_size);
		SDL_SetHint(SDL_HINT_AUDIO_DEVICE_SAMPLE_FRAMES, value);
	}

	if (!MIX_Init()) {
		WARNING("Cannot initialize SDL_mixer: %s", SDL_GetError());
		return false;
	}

	SDL_AudioSpec spec = {
		.format = SDL_AUDIO_S16LE,
		.channels = 2,
		.freq = SAMPLE_RATE,
	};
	mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
	if (!mixer) {
		WARNING("Cannot open audio device: %s", SDL_GetError());
		MIX_Quit();
		return false;
	}
	return true;
}

void sdl3_mixer_backend_exit(void)
{
	if (!mixer)
		return;
	MIX_DestroyMixer(mixer);
	mixer = NULL;
	MIX_Quit();
}

MIX_Mixer *sdl3_mixer_backend_get(void)
{
	return mixer;
}
