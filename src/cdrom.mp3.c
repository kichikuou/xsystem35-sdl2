/*
 * cdrom.mp3.c  CD-ROMのかわりにMP3fileだ！
 *
 * Copyright (C) 1997-1998 Masaki Chikama (Wren) <chikama@kasumi.ipl.mech.nagoya-u.ac.jp>
 *               1998-                           <masaki-c@is.aist-nara.ac.jp>
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
/* $Id: cdrom.mp3.c,v 1.24 2003/01/31 12:58:28 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <dirent.h>
#include <limits.h>
#include <strings.h>
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
#ifndef NAME_MAX
#define NAME_MAX 255
#endif
#endif
#include "sdl_compat.h"

#include "portab.h"
#include "system.h"
#include "cdrom.h"
#include "music_private.h"
#include "mixer.h"

static void cdrom_stop(void);

#define PLAYLIST_MAX 256

static bool      enabled = false;
static char         *playlist[PLAYLIST_MAX];
static mixer_music  *music;
static int          trackno; // 現在演奏中のトラック
static int          start_time;
static int          fade_volume = 100;

static void apply_volume(void) {
	mixer_music_set_gain(music, (fade_volume * prv.volval[BGM_VOLVAL_CH]) / 10000.0f);
}

#ifndef _WIN32
static bool resolve_case_insensitive_path(const char *src, char *out, size_t out_cap);
#endif

static void clear_playlist(void) {
	for (int i = 0; i < PLAYLIST_MAX; i++) {
		free(playlist[i]);
		playlist[i] = NULL;
	}
	prv.cd_maxtrk = 0;
}

static bool cdrom_init(char *playlist_path) {
	char buf[256];

	clear_playlist();
	enabled = false;
	trackno = 0;
	start_time = 0;
	fade_volume = 100;

	if (!playlist_path || !playlist_path[0])
		return false;

	FILE *fp = fopen(playlist_path, "r");
#ifndef _WIN32
	if (!fp) {
		char pl_resolved[PATH_MAX];
		if (resolve_case_insensitive_path(playlist_path, pl_resolved, sizeof(pl_resolved)))
			fp = fopen(pl_resolved, "r");
	}
#endif
	if (fp) {
		// Skip the first line
		fgets(buf, sizeof(buf), fp);
	} else {
		fp = fopen("_inmm.ini", "r");
#ifndef _WIN32
		if (!fp) {
			char inmm_resolved[PATH_MAX];
			if (resolve_case_insensitive_path("_inmm.ini", inmm_resolved, sizeof(inmm_resolved)))
				fp = fopen(inmm_resolved, "r");
		}
#endif
	}
	if (!fp) {
		// If the game has MIDI music, lack of the playlist is not a problem.
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
			prv.cd_maxtrk = track;
		}
	}
	fclose(fp);
	NOTICE("cdrom: Loaded playlist from %s", playlist_path);
	
	trackno = 0;
	enabled = true;
	return true;
}

static void cdrom_exit(void) {
	if (enabled) {
		cdrom_stop();
	}
	clear_playlist();
	enabled = false;
	trackno = 0;
	start_time = 0;
	fade_volume = 100;
}

static void cdrom_reset(void) {
	if (enabled) {
		cdrom_stop();
	}
	fade_volume = 100;
}

#ifndef _WIN32
static bool resolve_case_insensitive_path(const char *src, char *out, size_t out_cap) {
	if (!src || !src[0] || out_cap < 4)
		return false;
	char copy[PATH_MAX];
	if (strlen(src) >= sizeof(copy))
		return false;
	strcpy(copy, src);

	size_t len = 0;
	if (copy[0] == '/') {
		out[0] = '/';
		out[1] = '\0';
		len = 1;
	} else {
		out[0] = '.';
		out[1] = '\0';
		len = 1;
	}

	char *saveptr = NULL;
	for (char *tok = strtok_r(copy, "/", &saveptr); tok; tok = strtok_r(NULL, "/", &saveptr)) {
		if (!tok[0] || strcmp(tok, ".") == 0)
			continue;
		const char *match = tok;
		char matched_name[NAME_MAX + 1];
		if (strcmp(tok, "..") != 0) {
			DIR *d = opendir(out);
			if (d) {
				struct dirent *de;
				while ((de = readdir(d)) != NULL) {
					if (strcasecmp(de->d_name, tok) == 0) {
						snprintf(matched_name, sizeof(matched_name), "%s", de->d_name);
						match = matched_name;
						break;
					}
				}
				closedir(d);
			}
		}
		if (len > 1 && out[len - 1] != '/') {
			if (len + 1 >= out_cap)
				return false;
			out[len++] = '/';
			out[len] = '\0';
		} else if (len == 1 && out[0] == '.') {
			len = 0;
		}
		size_t mlen = strlen(match);
		if (len + mlen >= out_cap)
			return false;
		memcpy(out + len, match, mlen + 1);
		len += mlen;
	}
	return len > 0;
}
#endif

/* トラック番号 trk の演奏 trk = 1~ */
static bool cdrom_start(int trk, int loop) {
	if (!enabled) return 0;
	
	if (trk >= PLAYLIST_MAX || !playlist[trk])
		return false;
	
	if (music) {
		mixer_music_free(music);
		music = NULL;
	}

#ifdef __ANDROID__
	// mixer_music_load_file() opens the file with SDL_IOFromFile(), which
	// requires an absolute path on Android.
	char path[PATH_MAX];
	char resolved[PATH_MAX];
	const char *cand = playlist[trk];
	if (!realpath(cand, path)) {
		if (!resolve_case_insensitive_path(cand, resolved, sizeof(resolved)) ||
		    !realpath(resolved, path))
			return false;
	}
	music = mixer_music_load_file(path);
#elif !defined(_WIN32)
	music = mixer_music_load_file(playlist[trk]);
	if (!music) {
		char resolved[PATH_MAX];
		if (resolve_case_insensitive_path(playlist[trk], resolved, sizeof(resolved)))
			music = mixer_music_load_file(resolved);
	}
#else
	music = mixer_music_load_file(playlist[trk]);
#endif

	if (!music) {
		WARNING("Cannot load %s: %s", playlist[trk], SDL_GetError());
		return false;
	}
	apply_volume();
	if (!mixer_music_play(music, loop == 0 ? -1 : loop, 0)) {
		mixer_music_free(music);
		music = NULL;
		return false;
	}

	trackno = trk;
	start_time = SDL_GetTicks();
	
	return true;
}

static void cdrom_stop(void) {
	if (!enabled || !music)
		return;
	mixer_music_free(music);
	music = NULL;
	trackno = 0;
}

/* 現在演奏中のトラック情報の取得 */
static bool cdrom_getPlayingInfo (cd_time *inf) {
	if (!enabled || !music)
		return false;
	
	if (!mixer_music_playing(music)) {
		mixer_music_free(music);
		music = NULL;
		return false;
	}
	int ms = SDL_GetTicks() - start_time;
	
	inf->t = trackno;
	inf->m = ms / (60*1000); ms %= (60*1000);
	inf->s = ms / 1000;      ms %= 1000;
	inf->f = (ms * CD_FPS) / 1000;
	
	return true;
}

static bool cdrom_is_available(void) {
	return enabled;
}

static void cdrom_reapply_volume(void) {
	if (music)
		apply_volume();
}

static void cdrom_set_volume(int level) {
	fade_volume = level < 0 ? 0 : (level > 100 ? 100 : level);
	if (music)
		apply_volume();
}

cdromdevice_t cdrom_mp3 = {
	.init = cdrom_init,
	.exit = cdrom_exit,
	.reset = cdrom_reset,
	.start = cdrom_start,
	.stop = cdrom_stop,
	.getpos = cdrom_getPlayingInfo,
	.is_available = cdrom_is_available,
	.reapply_volume = cdrom_reapply_volume,
	.set_volume = cdrom_set_volume,
};
