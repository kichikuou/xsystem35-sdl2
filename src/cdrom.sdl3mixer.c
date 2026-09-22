/*
 * cdrom.sdl3mixer.c  CD audio replacement using SDL3_mixer
 *
 * Copyright (C) 1997-1998 Masaki Chikama (Wren) <chikama@kasumi.ipl.mech.nagoya-u.ac.jp>
 *               1998-                           <masaki-c@is.aist-nara.ac.jp>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "config.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ald_manager.h"
#include "cdrom.h"
#include "music_private.h"
#include "sdl3_mixer_backend.h"
#include "sdl3_mixer_utils.h"
#include "system.h"

#define PLAYLIST_MAX 256

static bool enabled;
static char *playlist[PLAYLIST_MAX];
static int track_no;
static uint32_t start_time;

static void cdrom_stop(void);

static float music_gain(void)
{
	return sdl3_mixer_gain(100, prv.volval[BGM_VOLVAL_CH]);
}

static void apply_volume(void)
{
	sdl3_mixer_set_music_gain(SDL3_MIXER_MUSIC_CD, music_gain());
}

static void free_playlist(void)
{
	for (int track = 0; track < PLAYLIST_MAX; track++) {
		free(playlist[track]);
		playlist[track] = NULL;
	}
	prv.cd_maxtrk = 0;
}

static bool cdrom_init(char *playlist_path)
{
	if (!playlist_path || !playlist_path[0])
		return false;

	if (enabled)
		cdrom_stop();
	enabled = false;
	free_playlist();
	FILE *fp = fopen(playlist_path, "r");
	char buf[256];
	if (fp) {
		/* The first line represents track 1, which is data on mixed CDs. */
		fgets(buf, sizeof(buf), fp);
	} else {
		fp = fopen("_inmm.ini", "r");
	}
	if (!fp) {
		if (ald_get_maxno(DRIFILE_MIDI) == 0)
			NOTICE("cdrom: Cannot open playlist %s", playlist_path);
		return false;
	}

	for (int track = 2; track < PLAYLIST_MAX; track++) {
		if (!fgets(buf, sizeof(buf), fp))
			break;
		for (char *s = buf; *s; s++) {
			if (*s == '\\')
				*s = '/';
			else if (*s == '\r' || *s == '\n') {
				*s = '\0';
				break;
			}
		}
		if (*buf) {
			playlist[track] = strdup(buf);
			if (playlist[track])
				prv.cd_maxtrk = track;
		}
	}
	fclose(fp);
	NOTICE("cdrom: Loaded playlist from %s", playlist_path);

	track_no = 0;
	enabled = true;
	return true;
}

static void cdrom_exit(void)
{
	if (enabled)
		cdrom_stop();
	enabled = false;
	free_playlist();
}

static void cdrom_reset(void)
{
	if (enabled)
		cdrom_stop();
}

static bool cdrom_start(int track, int loop)
{
	if (!enabled || track < 0 || track >= PLAYLIST_MAX || !playlist[track])
		return false;
	cdrom_stop();

	const char *path = playlist[track];
#ifdef __ANDROID__
	/* SDL's Android file IO requires an absolute path for installed tracks. */
	char absolute_path[PATH_MAX];
	if (!realpath(path, absolute_path))
		return false;
	path = absolute_path;
#endif

	MIX_Audio *audio = sdl3_mixer_load_music_file(path);
	if (!audio) {
		WARNING("Cannot load %s: %s", path, SDL_GetError());
		return false;
	}

	/* CD playback passes loop directly; this differs from BGM's loop - 1. */
	int loops = loop == 0 ? -1 : loop;
	if (!sdl3_mixer_play_music(SDL3_MIXER_MUSIC_CD, audio,
		loops, 0, music_gain()))
		return false;

	track_no = track;
	start_time = sys_get_ticks();
	return true;
}

static void cdrom_stop(void)
{
	if (!enabled)
		return;
	sdl3_mixer_stop_music(SDL3_MIXER_MUSIC_CD, 0);
	track_no = 0;
}

static bool cdrom_get_playing_info(cd_time *info)
{
	if (!enabled || !track_no ||
	    !sdl3_mixer_music_active(SDL3_MIXER_MUSIC_CD))
		return false;

	int ms = sys_get_ticks() - start_time;
	info->t = track_no;
	info->m = ms / (60 * 1000);
	ms %= 60 * 1000;
	info->s = ms / 1000;
	ms %= 1000;
	info->f = ms * CD_FPS / 1000;
	return true;
}

static bool cdrom_is_available(void)
{
	return enabled;
}

static void cdrom_reapply_volume(void)
{
	if (enabled && track_no)
		apply_volume();
}

cdromdevice_t cdrom_mp3 = {
	.init = cdrom_init,
	.exit = cdrom_exit,
	.reset = cdrom_reset,
	.start = cdrom_start,
	.stop = cdrom_stop,
	.getpos = cdrom_get_playing_info,
	.is_available = cdrom_is_available,
	.reapply_volume = cdrom_reapply_volume,
};
