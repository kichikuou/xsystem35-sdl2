/* Regression tests for BGM-backed CD-ROM (cdrom_bgm) volume and fade handling.
 * Links real music.c, music_cdrom.c, cdrom.bgm.c, and bgm.sdlmixer.c against
 * deterministic clock, archive, and mixer boundary stubs.
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ald_manager.h"
#include "audio_meta.h"
#include "mixer.h"
#include "music.h"
#include "music_private.h"
#include "nact.h"
#include "sdl_compat.h"
#include "unittest.h"

struct mixer_music {
	int id;
	bool playing;
	float gain;
};

static NACTINFO nact_state;
NACTINFO *nact = &nact_state;

static uint32_t fake_ticks;
static int fake_bgm_maxno = 10;
static int ald_getdata_calls;
static int ald_active_allocs;
static int mixer_active_allocs;
static int mixer_play_calls;
static int mixer_fade_out_calls;
static int mixer_last_fade_out_ms;
static float mixer_gain_at_fade_out = -1.0f;
static mixer_music *active_music;

#define GAIN_HISTORY_CAP 64
static float gain_history[GAIN_HISTORY_CAP];
static int gain_history_len;

static void assert_near_float(float actual, float expected, const char *file, int line) {
	if (fabsf(actual - expected) > 0.0005f) {
		fprintf(stderr, "%s:%d expected %.4f, got %.4f\n", file, line, expected, actual);
		exit(1);
	}
}
#define ASSERT_NEAR(actual, expected) assert_near_float((actual), (expected), __FILE__, __LINE__)

/* --- SDL timing/error stubs --- */

#if XSYSTEM35_SDL_VERSION == 2
Uint32 SDL_GetTicks(void) { return fake_ticks; }
#else
Uint64 SDL_GetTicks(void) { return fake_ticks; }
#endif

const char *SDL_GetError(void) { return "mock sdl error"; }
void SDL_Delay(Uint32 ms) { fake_ticks += ms; }

void sys_message(int level, char *format, ...) {
	(void)level;
	(void)format;
}

/* --- Archive & BGI stubs --- */

int ald_get_maxno(DRIFILETYPE type) {
	return type == DRIFILE_BGM ? fake_bgm_maxno : 0;
}

dridata *ald_getdata(DRIFILETYPE type, int no) {
	(void)type;
	ald_getdata_calls++;
	if (no < 0 || no >= fake_bgm_maxno)
		return NULL;
	dridata *d = calloc(1, sizeof(*d));
	ASSERT_TRUE(d != NULL);
	d->data = calloc(1, 16);
	d->size = 16;
	ald_active_allocs++;
	return d;
}

void ald_freedata(dridata *data) {
	if (!data)
		return;
	free(data->data);
	free(data);
	ald_active_allocs--;
}

bool bgi_read(const char *path) {
	(void)path;
	return true;
}

bgi_t *bgi_find(int no) {
	(void)no;
	return NULL;
}

void bgi_clear(void) {}

/* --- Mixer stubs --- */

mixer_music *mixer_music_load(const void *data, size_t size) {
	(void)data;
	(void)size;
	mixer_music *m = calloc(1, sizeof(*m));
	ASSERT_TRUE(m != NULL);
	m->id = ++mixer_play_calls;
	m->gain = 1.0f;
	mixer_active_allocs++;
	return m;
}

void mixer_music_free(mixer_music *music) {
	if (!music)
		return;
	if (active_music == music)
		active_music = NULL;
	free(music);
	mixer_active_allocs--;
}

bool mixer_music_play(mixer_music *music, int play_count, int fade_in_ms) {
	(void)play_count;
	(void)fade_in_ms;
	ASSERT_TRUE(music != NULL);
	music->playing = true;
	active_music = music;
	return true;
}

void mixer_music_fade_out(mixer_music *music, int ms) {
	ASSERT_TRUE(music != NULL);
	mixer_fade_out_calls++;
	mixer_last_fade_out_ms = ms;
	mixer_gain_at_fade_out = music->gain;
	music->playing = false;
}

bool mixer_music_playing(mixer_music *music) {
	ASSERT_TRUE(music != NULL);
	return music->playing;
}

void mixer_music_set_gain(mixer_music *music, float gain) {
	ASSERT_TRUE(music != NULL);
	music->gain = gain;
	if (gain_history_len < GAIN_HISTORY_CAP)
		gain_history[gain_history_len++] = gain;
}

/* --- Unused cdrom_mp3 / MIDI / PCM boundary stubs required by music_cdrom.c & music.c --- */

static bool dummy_mp3_init(char *playlist) { (void)playlist; return false; }
static void dummy_mp3_void(void) {}
static bool dummy_mp3_start(int trk, int loop) { (void)trk; (void)loop; return false; }
static bool dummy_mp3_getpos(cd_time *t) { (void)t; return false; }
static bool dummy_mp3_avail(void) { return false; }

cdromdevice_t cdrom_mp3 = {
	.init = dummy_mp3_init,
	.exit = dummy_mp3_void,
	.reset = dummy_mp3_void,
	.start = dummy_mp3_start,
	.stop = dummy_mp3_void,
	.getpos = dummy_mp3_getpos,
	.is_available = dummy_mp3_avail,
};

bool musmidi_init(void) { return false; }
void musmidi_exit(void) {}
void musmidi_reset(void) {}
bool musmidi_start(int no, int loop) { (void)no; (void)loop; return false; }
void musmidi_stop(void) {}
void musmidi_pause(void) {}
void musmidi_unpause(void) {}
midiplaystate musmidi_getpos(void) { return (midiplaystate){false, 0, 0}; }
bool musmidi_setflag(int mode, int index, int val) { (void)mode; (void)index; (void)val; return false; }
int musmidi_getflag(int mode, int index) { (void)mode; (void)index; return 0; }
bool musmidi_fadestart(int time, int volume, int stop) { (void)time; (void)volume; (void)stop; return false; }
bool musmidi_fading(void) { return false; }
void musmidi_reapply_valance(void) {}

bool muspcm_init(int audio_buffer_size) { (void)audio_buffer_size; return false; }
void muspcm_exit(void) {}
void muspcm_reset(void) {}
bool muspcm_load_no(int slot, int no) { (void)slot; (void)no; return false; }
bool muspcm_load_mixlr(int slot, int noL, int noR) { (void)slot; (void)noL; (void)noR; return false; }
bool muspcm_load_data(int slot, uint8_t *buf, uint32_t len) { (void)slot; (void)buf; (void)len; return false; }
void muspcm_unload(int slot) { (void)slot; }
bool muspcm_start(int slot, int loop) { (void)slot; (void)loop; return false; }
void muspcm_stop(int slot) { (void)slot; }
void muspcm_fadeout(int slot, int msec) { (void)slot; (void)msec; }
int muspcm_getpos(int slot) { (void)slot; return 0; }
void muspcm_setvol(int slot, int lv) { (void)slot; (void)lv; }
int muspcm_getwavelen(int slot) { (void)slot; return 0; }
void muspcm_waitend(int slot) { (void)slot; }
void muspcm_reapply_valance(void) {}

/* --- Test helpers --- */

static void reset_env(int bgm_maxno) {
	mus_exit();
	ASSERT_EQUAL(ald_active_allocs, 0);
	ASSERT_EQUAL(mixer_active_allocs, 0);
	memset(&nact_state, 0, sizeof(nact_state));
	fake_ticks = 1000;
	fake_bgm_maxno = bgm_maxno;
	ald_getdata_calls = 0;
	mixer_play_calls = 0;
	mixer_fade_out_calls = 0;
	mixer_last_fade_out_ms = -1;
	mixer_gain_at_fade_out = -1.0f;
	active_music = NULL;
	gain_history_len = 0;
	ASSERT_TRUE(mus_init(4096));
}

static void cdrom_bgm_fade_without_stop_test(void) {
	reset_env(10);
	ASSERT_EQUAL_PTR(prv.cddev, &cdrom_bgm);
	ASSERT_TRUE(prv.cddev->set_volume != NULL);
	ASSERT_EQUAL(muscd_get_maxtrack(), 11);

	ASSERT_TRUE(muscd_start(2, 0));
	ASSERT_TRUE(active_music != NULL);
	ASSERT_TRUE(active_music->playing);
	ASSERT_NEAR(active_music->gain, 1.0f);
	ASSERT_EQUAL(mus_mixer_get_level(MIX_CD), 100);

	// Starting a fade immediately on the same tick (0ms elapsed) must work.
	int t = 0, m = 0, s = 0, f = 0;
	ASSERT_TRUE(muscd_getpos(&t, &m, &s, &f));
	ASSERT_EQUAL(t, 2);

	gain_history_len = 0;
	ASSERT_TRUE(mus_mixer_fadeout_start(MIX_CD, 1000, 40, 0));
	ASSERT_TRUE(mus_mixer_fadeout_get_state(MIX_CD));

	// Intermediate updates must propagate actual volumes to the BGM mixer.
	fake_ticks += 250;
	mus_update();
	ASSERT_TRUE(mus_mixer_fadeout_get_state(MIX_CD));
	ASSERT_EQUAL(mus_mixer_get_level(MIX_CD), 85);
	ASSERT_NEAR(active_music->gain, 0.85f);
	ASSERT_TRUE(active_music->playing);

	fake_ticks += 250;
	ASSERT_TRUE(mus_mixer_fadeout_get_state(MIX_CD));
	ASSERT_EQUAL(mus_mixer_get_level(MIX_CD), 70);
	ASSERT_NEAR(active_music->gain, 0.70f);
	ASSERT_TRUE(active_music->playing);

	// Completing a non-stopping fade keeps playback active at target volume.
	fake_ticks += 500;
	ASSERT_FALSE(mus_mixer_fadeout_get_state(MIX_CD));
	ASSERT_EQUAL(mus_mixer_get_level(MIX_CD), 40);
	ASSERT_NEAR(active_music->gain, 0.40f);
	ASSERT_TRUE(active_music->playing);
	ASSERT_EQUAL(mixer_fade_out_calls, 0);
	ASSERT_EQUAL(gain_history_len, 3);
	ASSERT_NEAR(gain_history[0], 0.85f);
	ASSERT_NEAR(gain_history[1], 0.70f);
	ASSERT_NEAR(gain_history[2], 0.40f);

	// VolumeValancer must compose with the faded volume instead of resetting it.
	int vols[1] = {50};
	ASSERT_TRUE(mus_vol_set_valance(vols, 1));
	ASSERT_EQUAL(mus_mixer_get_level(MIX_CD), 40);
	ASSERT_NEAR(active_music->gain, 0.20f);

	// Subsequent fade-up starts from 40 and respects the 50% VolumeValancer.
	ASSERT_TRUE(mus_mixer_fadeout_start(MIX_CD, 400, 80, 0));
	fake_ticks += 200;
	ASSERT_TRUE(mus_mixer_fadeout_get_state(MIX_CD));
	ASSERT_EQUAL(mus_mixer_get_level(MIX_CD), 60);
	ASSERT_NEAR(active_music->gain, 0.30f);

	fake_ticks += 200;
	ASSERT_FALSE(mus_mixer_fadeout_get_state(MIX_CD));
	ASSERT_EQUAL(mus_mixer_get_level(MIX_CD), 80);
	ASSERT_NEAR(active_music->gain, 0.40f);
	ASSERT_TRUE(active_music->playing);
}

static void cdrom_bgm_fade_to_zero_with_stop_test(void) {
	reset_env(10);
	ASSERT_TRUE(muscd_start(3, 0));
	mixer_music *trk3 = active_music;
	ASSERT_TRUE(trk3 != NULL && trk3->playing);

	gain_history_len = 0;
	ASSERT_TRUE(mus_mixer_fadeout_start(MIX_CD, 1000, 0, 1));
	ASSERT_TRUE(mus_mixer_fadeout_get_state(MIX_CD));

	fake_ticks += 250;
	ASSERT_TRUE(mus_mixer_fadeout_get_state(MIX_CD));
	ASSERT_EQUAL(mus_mixer_get_level(MIX_CD), 75);
	ASSERT_NEAR(trk3->gain, 0.75f);
	ASSERT_TRUE(trk3->playing);
	ASSERT_EQUAL(mixer_fade_out_calls, 0);

	fake_ticks += 250;
	ASSERT_TRUE(mus_mixer_fadeout_get_state(MIX_CD));
	ASSERT_EQUAL(mus_mixer_get_level(MIX_CD), 50);
	ASSERT_NEAR(trk3->gain, 0.50f);
	ASSERT_TRUE(trk3->playing);

	fake_ticks += 250;
	ASSERT_TRUE(mus_mixer_fadeout_get_state(MIX_CD));
	ASSERT_EQUAL(mus_mixer_get_level(MIX_CD), 25);
	ASSERT_NEAR(trk3->gain, 0.25f);
	ASSERT_TRUE(trk3->playing);

	// Deadline must ramp gain to 0.0, stop playback, and not spike gain back to 1.0.
	fake_ticks += 250;
	ASSERT_FALSE(mus_mixer_fadeout_get_state(MIX_CD));
	ASSERT_EQUAL(mixer_fade_out_calls, 1);
	ASSERT_EQUAL(mixer_last_fade_out_ms, 0);
	ASSERT_NEAR(mixer_gain_at_fade_out, 0.0f);
	ASSERT_FALSE(trk3->playing);
	ASSERT_NEAR(trk3->gain, 0.0f);
	ASSERT_FALSE(musbgm_isplaying(3));
	int t, m, s, f;
	ASSERT_FALSE(muscd_getpos(&t, &m, &s, &f));
	ASSERT_EQUAL(mus_mixer_get_level(MIX_CD), 100);

	// Starting the next track plays cleanly at full volume.
	fake_ticks += 100;
	ASSERT_TRUE(muscd_start(4, 0));
	ASSERT_TRUE(active_music != NULL && active_music->playing);
	ASSERT_NEAR(active_music->gain, 1.0f);
	ASSERT_EQUAL(mus_mixer_get_level(MIX_CD), 100);
}

static void cdrom_bgm_fadestop_and_set_level_test(void) {
	reset_env(10);
	ASSERT_TRUE(muscd_start(2, 0));
	ASSERT_TRUE(mus_mixer_fadeout_start(MIX_CD, 1000, 0, 1));

	fake_ticks += 400;
	mus_update();
	ASSERT_TRUE(mus_mixer_fadeout_get_state(MIX_CD));
	ASSERT_EQUAL(mus_mixer_get_level(MIX_CD), 60);
	ASSERT_NEAR(active_music->gain, 0.60f);

	// Cancelling the fade freezes volume and prevents stop at deadline.
	mus_mixer_fadeout_stop(MIX_CD);
	ASSERT_FALSE(mus_mixer_fadeout_get_state(MIX_CD));
	fake_ticks += 2000;
	mus_update();
	ASSERT_TRUE(active_music->playing);
	ASSERT_EQUAL(mus_mixer_get_level(MIX_CD), 60);
	ASSERT_NEAR(active_music->gain, 0.60f);

	// Direct mixer level set updates gain immediately.
	ASSERT_TRUE(mus_mixer_set_level(MIX_CD, 25));
	ASSERT_EQUAL(mus_mixer_get_level(MIX_CD), 25);
	ASSERT_NEAR(active_music->gain, 0.25f);
	ASSERT_TRUE(active_music->playing);

	// Zero-duration fade with stop halts immediately at target volume.
	mixer_music *trk2 = active_music;
	ASSERT_TRUE(mus_mixer_fadeout_start(MIX_CD, 0, 0, 1));
	ASSERT_FALSE(mus_mixer_fadeout_get_state(MIX_CD));
	ASSERT_FALSE(trk2->playing);
	ASSERT_NEAR(mixer_gain_at_fade_out, 0.0f);
	ASSERT_NEAR(trk2->gain, 0.0f);
}

static void cdrom_bgm_isolation_from_direct_musbgm_test(void) {
	reset_env(10);

	// Direct BGM playback must not be disrupted by idle CD calls.
	ASSERT_TRUE(musbgm_play(7, 0, 75, 0));
	ASSERT_TRUE(active_music != NULL && active_music->playing);
	ASSERT_NEAR(active_music->gain, 0.75f);

	muscd_stop();
	ASSERT_FALSE(mus_mixer_fadeout_start(MIX_CD, 500, 0, 1));
	muscd_reset();
	ASSERT_TRUE(musbgm_isplaying(7));
	ASSERT_NEAR(active_music->gain, 0.75f);

	// If a game switches from CD playback to direct BGM playback on another track,
	// subsequent CD volume/stop calls must not clobber the direct BGM track.
	ASSERT_TRUE(muscd_start(2, 0));
	ASSERT_TRUE(musbgm_play(8, 0, 80, 0));
	ASSERT_TRUE(musbgm_isplaying(8));
	ASSERT_NEAR(active_music->gain, 0.80f);

	ASSERT_TRUE(mus_mixer_set_level(MIX_CD, 10));
	muscd_stop();
	ASSERT_TRUE(musbgm_isplaying(8));
	ASSERT_NEAR(active_music->gain, 0.80f);
}

static void cdrom_bgm_failed_start_test(void) {
	reset_env(5);

	ASSERT_FALSE(muscd_start(99, 0));
	ASSERT_EQUAL(prv.cd_current_track, 0);
	ASSERT_FALSE(mus_mixer_fadeout_start(MIX_CD, 500, 0, 1));
	ASSERT_FALSE(mus_mixer_fadeout_get_state(MIX_CD));

	// Retrying a failed track must attempt loading again rather than returning true.
	int prev_calls = ald_getdata_calls;
	ASSERT_FALSE(muscd_start(99, 0));
	ASSERT_EQUAL(ald_getdata_calls, prev_calls + 1);
}

int main(void) {
	cdrom_bgm_fade_without_stop_test();
	cdrom_bgm_fade_to_zero_with_stop_test();
	cdrom_bgm_fadestop_and_set_level_test();
	cdrom_bgm_isolation_from_direct_musbgm_test();
	cdrom_bgm_failed_start_test();
	mus_exit();
	ASSERT_EQUAL(ald_active_allocs, 0);
	ASSERT_EQUAL(mixer_active_allocs, 0);
	puts("5 cdrom_bgm volume/fade regression cases passed");
	return 0;
}
