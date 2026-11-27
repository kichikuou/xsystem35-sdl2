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
static int fade_volume = 100;

static int clamp_volume(int level) {
	if (level < 0) return 0;
	if (level > 100) return 100;
	return level;
}

static void cdrom_bgm_stop(void);

static bool cdrom_bgm_init(char *playlist) {
	current_track = 0;
	fade_volume = 100;
	prv.cd_maxtrk = ald_get_maxno(DRIFILE_BGM) + 1;
	return true;
}

static void cdrom_bgm_exit(void) {
	cdrom_bgm_stop();
	fade_volume = 100;
}

static void cdrom_bgm_reset(void) {
	cdrom_bgm_stop();
	fade_volume = 100;
}

static bool cdrom_bgm_start(int trk, int loop) {
	if (!musbgm_play(trk, 0, fade_volume, loop))
		return false;
	current_track = trk;
	return true;
}

static void cdrom_bgm_stop(void) {
	if (current_track) {
		musbgm_stop(current_track, 0);
		current_track = 0;
	}
}

static bool cdrom_bgm_getPlayingInfo(cd_time *info) {
	if (!current_track || !musbgm_isplaying(current_track))
		return false;
	int t = musbgm_getpos(current_track);  // in 10ms
	info->t = current_track;
	info->m = t / (60*100); t %= (60*100);
	info->s = t / 100;      t %= 100;
	info->f = t * CD_FPS / 100;
	return true;
}

static bool cdrom_bgm_is_available(void) {
	return true;
}

static void cdrom_bgm_set_volume(int level) {
	fade_volume = clamp_volume(level);
	if (current_track && musbgm_isplaying(current_track))
		musbgm_fade(current_track, 0, fade_volume);
}

cdromdevice_t cdrom_bgm = {
	.init = cdrom_bgm_init,
	.exit = cdrom_bgm_exit,
	.reset = cdrom_bgm_reset,
	.start = cdrom_bgm_start,
	.stop = cdrom_bgm_stop,
	.getpos = cdrom_bgm_getPlayingInfo,
	.is_available = cdrom_bgm_is_available,
	.set_volume = cdrom_bgm_set_volume,
};
