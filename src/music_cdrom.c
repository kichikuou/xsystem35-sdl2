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
#include "ald_manager.h"
#include "music_private.h"
#include "music_cdrom.h"
#include "cdrom.h"

#ifdef __EMSCRIPTEN__
extern cdromdevice_t cdrom_emscripten;
#define NATIVE_CD_DEVICE &cdrom_emscripten
#else
extern cdromdevice_t cdrom_mp3;
#define NATIVE_CD_DEVICE &cdrom_mp3
#endif

static char *playlist = DEFAULT_PLAYLIST_PATH;

void muscd_set_playlist(const char *name) {
	playlist = strdup(name);
}

bool muscd_init(void) {
	// In the download edition of Daiakuji, SS command plays music from *BA.ald.
	if (ald_get_maxno(DRIFILE_BGM) > 0) {
		prv.cddev = &cdrom_bgm;
	} else {
		prv.cddev = NATIVE_CD_DEVICE;
	}
	prv.cd_current_track = 0;
	return prv.cddev->init(playlist);
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
}

void muscd_reset(void) {
	if (prv.cddev) {
		prv.cddev->reset();
		prv.cd_current_track = 0;
	}
}

bool muscd_start(int trk, int loop) {
	if (!prv.cddev)
		return false;
	if (trk == prv.cd_current_track)
		return true;
	prv.cddev->stop();

	prv.cd_current_track = trk;
	return prv.cddev->start(trk, loop);
}

void muscd_stop(void) {
	if (!prv.cddev)
		return;
	prv.cddev->stop();
	prv.cd_current_track = 0;
}

bool muscd_getpos(int *t, int *m, int *s, int *f) {
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
	return prv.cddev;
}
