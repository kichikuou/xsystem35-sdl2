/*
 * cdrom.bgm.c  CD->bgm bridge
 *
 * Copyright (C) 2023 <KichikuouChrome@gmail.com>
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

#include <stdio.h>

#include "cdrom.h"
#include "ald_manager.h"
#include "music.h"
#include "music_private.h"

static int current_track;

static bool cdrom_bgm_init(char *playlist) {
	prv.cd_maxtrk = ald_get_maxno(DRIFILE_BGM) + 1;
	return true;
}

static void cdrom_bgm_exit(void) {
	// Do nothing
}

static void cdrom_bgm_reset(void) {
	// Do nothing
}

static bool cdrom_bgm_start(int trk, int loop) {
	if (!musbgm_play(trk, 0, 100, loop))
		return false;
	current_track = trk;
	return true;
}

static void cdrom_bgm_stop(void) {
	musbgm_stop(current_track, 0);
}

static bool cdrom_bgm_getPlayingInfo(cd_time *info) {
	int t = musbgm_getpos(current_track);  // in 10ms
	if (!t)
		return false;
	info->t = current_track;
	info->m = t / (60*100); t %= (60*100);
	info->s = t / 100;      t %= 100;
	info->f = t * CD_FPS / 100;
	return true;
}

cdromdevice_t cdrom_bgm = {
	.init = cdrom_bgm_init,
	.exit = cdrom_bgm_exit,
	.reset = cdrom_bgm_reset,
	.start = cdrom_bgm_start,
	.stop = cdrom_bgm_stop,
	.getpos = cdrom_bgm_getPlayingInfo,
};
