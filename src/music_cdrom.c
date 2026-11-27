/*
 * music_cdrom.c  music server CDROM part
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
/* $Id: music_cdrom.c,v 1.2 2002/09/01 11:54:51 chikama Exp $ */

#include <stdio.h>
#include <string.h>

#include "portab.h"
#include "sdl_compat.h"
#include "ald_manager.h"
#include "music_private.h"
#include "music_cdrom.h"
#include "cdrom.h"

static struct {
	bool active;
	bool stop_on_end;
	uint32_t start_ms;
	int duration_ms;
	int start_vol;
	int target_vol;
	int current_vol;
} cd_fade = { .current_vol = 100 };

static int clamp_vol(int vol) {
	if (vol < 0) return 0;
	if (vol > 100) return 100;
	return vol;
}

static void reset_cd_fade(void) {
	cd_fade.active = false;
	cd_fade.stop_on_end = false;
	cd_fade.current_vol = 100;
	if (prv.cddev && prv.cddev->set_volume)
		prv.cddev->set_volume(100);
}

static bool update_cd_fade(uint32_t now) {
	if (!cd_fade.active)
		return false;

	uint32_t elapsed = now - cd_fade.start_ms;
	if (elapsed >= (uint32_t)cd_fade.duration_ms) {
		bool should_stop = cd_fade.stop_on_end;
		cd_fade.active = false;
		cd_fade.stop_on_end = false;
		cd_fade.current_vol = cd_fade.target_vol;
		if (prv.cddev && prv.cddev->set_volume)
			prv.cddev->set_volume(cd_fade.current_vol);
		if (should_stop)
			muscd_stop();
		return false;
	}

	int vol = cd_fade.start_vol +
	          (int)(((int64_t)(cd_fade.target_vol - cd_fade.start_vol) * elapsed) / cd_fade.duration_ms);
	if (vol != cd_fade.current_vol) {
		cd_fade.current_vol = vol;
		if (prv.cddev && prv.cddev->set_volume)
			prv.cddev->set_volume(vol);
	}
	return true;
}

#ifdef __EMSCRIPTEN__
extern cdromdevice_t cdrom_emscripten;
#define NATIVE_CD_DEVICE &cdrom_emscripten
#else
extern cdromdevice_t cdrom_mp3;
#define NATIVE_CD_DEVICE &cdrom_mp3
#endif

static char *custom_playlist = NULL;

void muscd_set_playlist(const char *name) {
	free(custom_playlist);
	custom_playlist = name ? strdup(name) : NULL;
}

void muscd_reset_playlist(void) {
	free(custom_playlist);
	custom_playlist = NULL;
}

bool muscd_init(void) {
	// In the download edition of Daiakuji, SS command plays music from *BA.ald.
	if (ald_get_maxno(DRIFILE_BGM) > 0) {
		prv.cddev = &cdrom_bgm;
	} else {
		prv.cddev = NATIVE_CD_DEVICE;
	}
	prv.cd_current_track = 0;
	cd_fade.active = false;
	cd_fade.stop_on_end = false;
	cd_fade.current_vol = 100;
	return prv.cddev->init(custom_playlist ? custom_playlist : DEFAULT_PLAYLIST_PATH);
}

bool muscd_init_bgm(DRIFILETYPE type, int base_no) {
	muscd_exit();
	prv.cddev = &cdrom_bgm;
	return musbgm_init(type, base_no);
}

void muscd_exit(void) {
	if (prv.cddev) {
		prv.cddev->exit();
		prv.cddev = NULL;
	}
	prv.cd_current_track = 0;
	prv.cd_maxtrk = 0;
	cd_fade.active = false;
	cd_fade.stop_on_end = false;
	cd_fade.current_vol = 100;
}

void muscd_reset(void) {
	if (prv.cddev) {
		prv.cddev->reset();
		prv.cd_current_track = 0;
		reset_cd_fade();
	}
}

bool muscd_start(int trk, int loop) {
	if (!prv.cddev)
		return false;
	if (trk == prv.cd_current_track)
		return true;
	prv.cddev->stop();
	reset_cd_fade();

	if (!prv.cddev->start(trk, loop)) {
		prv.cd_current_track = 0;
		return false;
	}
	prv.cd_current_track = trk;
	return true;
}

void muscd_stop(void) {
	if (!prv.cddev)
		return;
	prv.cddev->stop();
	prv.cd_current_track = 0;
	reset_cd_fade();
}

bool muscd_getpos(int *t, int *m, int *s, int *f) {
	update_cd_fade(SDL_GetTicks());
	if (!prv.cddev)
		return false;
	cd_time info;
	if (!prv.cddev->getpos(&info))
		return false;
	*t = info.t;
	*m = info.m;
	*s = info.s;
	*f = info.f;
	return true;
}

int muscd_get_maxtrack(void) {
	if (!prv.cddev)
		return 0;
	return prv.cd_maxtrk;
}

bool muscd_is_available(void) {
	return prv.cddev && prv.cddev->is_available();
}

void muscd_reapply_valance(void) {
	if (prv.cddev && prv.cddev->reapply_volume)
		prv.cddev->reapply_volume();
}

bool muscd_fadestart(int time, int volume, int stop) {
	volume = clamp_vol(volume);
	if (!prv.cddev || !prv.cddev->is_available() || prv.cd_current_track == 0) {
		reset_cd_fade();
		return false;
	}

	cd_time info;
	if (!prv.cddev->getpos(&info)) {
		prv.cd_current_track = 0;
		reset_cd_fade();
		return false;
	}

	uint32_t now = SDL_GetTicks();
	update_cd_fade(now);
	if (prv.cd_current_track == 0)
		return false;

	if (time <= 0 || (volume == cd_fade.current_vol && !stop)) {
		cd_fade.active = false;
		cd_fade.stop_on_end = false;
		cd_fade.current_vol = volume;
		if (prv.cddev->set_volume)
			prv.cddev->set_volume(volume);
		if (stop)
			muscd_stop();
		return true;
	}

	cd_fade.active = true;
	cd_fade.stop_on_end = (stop != 0);
	cd_fade.start_ms = now;
	cd_fade.duration_ms = time;
	cd_fade.start_vol = cd_fade.current_vol;
	cd_fade.target_vol = volume;
	return true;
}

bool muscd_fading(void) {
	return update_cd_fade(SDL_GetTicks());
}

void muscd_fadestop(void) {
	cd_fade.active = false;
	cd_fade.stop_on_end = false;
}

int muscd_get_volume(void) {
	update_cd_fade(SDL_GetTicks());
	return cd_fade.current_vol;
}

bool muscd_set_volume(int volume) {
	if (!prv.cddev)
		return false;
	volume = clamp_vol(volume);
	cd_fade.active = false;
	cd_fade.stop_on_end = false;
	cd_fade.current_vol = volume;
	if (prv.cddev->set_volume)
		prv.cddev->set_volume(volume);
	return true;
}
