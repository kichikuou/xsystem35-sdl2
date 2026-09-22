#include "sdl3_mixer_backend.h"

#include "system.h"

#define SAMPLE_RATE 44100

static MIX_Mixer *mixer;
static MIX_Track *music_track;
static MIX_Audio *music_audio;
static enum sdl3_mixer_music_owner music_owner;

static void clear_music(void)
{
	if (!music_audio)
		return;
	MIX_StopTrack(music_track, 0);
	MIX_SetTrackAudio(music_track, NULL);
	MIX_DestroyAudio(music_audio);
	music_audio = NULL;
	music_owner = SDL3_MIXER_MUSIC_NONE;
}

static void reap_finished_music(void)
{
	if (music_audio && !MIX_TrackPlaying(music_track) &&
	    !MIX_TrackPaused(music_track))
		clear_music();
}

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
	music_track = MIX_CreateTrack(mixer);
	if (!music_track) {
		WARNING("Cannot create music track: %s", SDL_GetError());
		MIX_DestroyMixer(mixer);
		mixer = NULL;
		MIX_Quit();
		return false;
	}
	return true;
}

void sdl3_mixer_backend_exit(void)
{
	if (!mixer)
		return;
	clear_music();
	MIX_DestroyTrack(music_track);
	music_track = NULL;
	MIX_DestroyMixer(mixer);
	mixer = NULL;
	MIX_Quit();
}

MIX_Mixer *sdl3_mixer_backend_get(void)
{
	return mixer;
}

MIX_Audio *sdl3_mixer_load_music_memory(const void *data, size_t size)
{
	if (!mixer)
		return NULL;
	SDL_IOStream *io = SDL_IOFromConstMem(data, size);
	if (!io)
		return NULL;
	return MIX_LoadAudio_IO(mixer, io, false, true);
}

MIX_Audio *sdl3_mixer_load_music_file(const char *path)
{
	return mixer ? MIX_LoadAudio(mixer, path, false) : NULL;
}

bool sdl3_mixer_play_music(enum sdl3_mixer_music_owner owner,
	MIX_Audio *audio, int loops, int fade_in_ms, float gain)
{
	if (!music_track || !audio || owner == SDL3_MIXER_MUSIC_NONE) {
		MIX_DestroyAudio(audio);
		return false;
	}

	clear_music();
	if (!MIX_SetTrackAudio(music_track, audio)) {
		MIX_DestroyAudio(audio);
		return false;
	}
	music_audio = audio;
	music_owner = owner;

	SDL_PropertiesID props = SDL_CreateProperties();
	bool success = props &&
		MIX_SetTrackGain(music_track, gain) &&
		SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, loops) &&
		SDL_SetNumberProperty(props,
			MIX_PROP_PLAY_FADE_IN_MILLISECONDS_NUMBER, fade_in_ms) &&
		MIX_PlayTrack(music_track, props);
	if (props)
		SDL_DestroyProperties(props);
	if (!success)
		clear_music();
	return success;
}

void sdl3_mixer_stop_music(enum sdl3_mixer_music_owner owner, int fade_out_ms)
{
	reap_finished_music();
	if (music_owner != owner)
		return;
	if (fade_out_ms <= 0) {
		clear_music();
		return;
	}
	Sint64 frames = MIX_TrackMSToFrames(music_track, fade_out_ms);
	if (frames <= 0 || !MIX_StopTrack(music_track, frames))
		clear_music();
}

void sdl3_mixer_pause_music(enum sdl3_mixer_music_owner owner)
{
	reap_finished_music();
	if (music_owner == owner)
		MIX_PauseTrack(music_track);
}

void sdl3_mixer_resume_music(enum sdl3_mixer_music_owner owner)
{
	reap_finished_music();
	if (music_owner == owner)
		MIX_ResumeTrack(music_track);
}

bool sdl3_mixer_music_active(enum sdl3_mixer_music_owner owner)
{
	reap_finished_music();
	return music_owner == owner &&
		(MIX_TrackPlaying(music_track) || MIX_TrackPaused(music_track));
}

void sdl3_mixer_set_music_gain(enum sdl3_mixer_music_owner owner, float gain)
{
	reap_finished_music();
	if (music_owner == owner)
		MIX_SetTrackGain(music_track, gain);
}
