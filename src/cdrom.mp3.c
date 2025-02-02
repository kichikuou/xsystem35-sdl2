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
#include <SDL_timer.h>
#include <SDL_mixer.h>

#include "portab.h"
#include "system.h"
#include "cdrom.h"
#include "music_private.h"

static int cdrom_init(char *);
static int cdrom_exit(void);
static int cdrom_reset(void);
static int cdrom_start(int, int);
static int cdrom_stop();
static int cdrom_getPlayingInfo(cd_time *);

#define cdrom cdrom_mp3
cdromdevice_t cdrom = {
	cdrom_init,
	cdrom_exit,
	cdrom_reset,
	cdrom_start,
	cdrom_stop,
	cdrom_getPlayingInfo,
	NULL,
	NULL
};

#define PLAYLIST_MAX 256

static bool      enabled = false;
static char         *playlist[PLAYLIST_MAX];
static Mix_Music    *mix_music;
static int          trackno; // 現在演奏中のトラック
static int          start_time;

static int cdrom_init(char *playlist_path) {
	char buf[256];

	if (!playlist_path || !playlist_path[0])
		return NG;

	FILE *fp = fopen(playlist_path, "r");
	if (fp) {
		// Skip the first line
		fgets(buf, sizeof(buf), fp);
	} else {
		fp = fopen("_inmm.ini", "r");
	}
	if (!fp) {
		// If the game has MIDI music, lack of the playlist is not a problem.
		if (ald_get_maxno(DRIFILE_MIDI) == 0)
			NOTICE("cdrom: Cannot open playlist %s", playlist_path);
		return NG;
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
	return OK;
}

static int cdrom_exit(void) {
	if (enabled) {
		cdrom_stop();
	}
	return OK;
}

static int cdrom_reset(void) {
	if (enabled) {
		cdrom_stop();
	}
	return OK;
}

/* トラック番号 trk の演奏 trk = 1~ */
static int cdrom_start(int trk, int loop) {
	if (!enabled) return 0;
	
	if (trk >= PLAYLIST_MAX || !playlist[trk])
		return NG;
	
	if (mix_music)
		Mix_FreeMusic(mix_music);

#ifdef __ANDROID__
	// Mix_LoadMUS uses SDL_RWFromFile which requires absolute path on Android
	char path[PATH_MAX];
	if (!realpath(playlist[trk], path))
		return NG;
	mix_music = Mix_LoadMUS(path);
#else
	mix_music = Mix_LoadMUS(playlist[trk]);
#endif

	if (!mix_music) {
		WARNING("Cannot load %s: %s", playlist[trk], Mix_GetError());
		return NG;
	}
	if (Mix_PlayMusic(mix_music, loop == 0 ? -1 : loop) != 0) {
		Mix_FreeMusic(mix_music);
		mix_music = NULL;
		return NG;
	}

	trackno = trk;
	start_time = SDL_GetTicks();
	
	return OK;
}

/* 演奏停止 */
static int cdrom_stop() {
	if (!enabled || !mix_music) {
		return OK;
	}
	Mix_FreeMusic(mix_music);
	mix_music = NULL;
	trackno = 0;
	
	return OK;
}

/* 現在演奏中のトラック情報の取得 */
static int cdrom_getPlayingInfo (cd_time *inf) {
	if (!enabled || !mix_music)
		return NG;
	
	if (!Mix_PlayingMusic()) {
		Mix_FreeMusic(mix_music);
		mix_music = NULL;
		return NG;
	}
	int ms = SDL_GetTicks() - start_time;
	
	inf->t = trackno;
	inf->m = ms / (60*1000); ms %= (60*1000);
	inf->s = ms / 1000;      ms %= 1000;
	inf->f = (ms * CD_FPS) / 1000;
	
	return OK;
}
