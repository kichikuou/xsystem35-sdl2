/*
 * cdrom.emscripten.c  CD-ROMアクセス
 *
 * Copyright (C) 2017 <KichikuouChrome@gmail.com>
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
#include <emscripten.h>
#include "portab.h"
#include "cdrom.h"
#include "scheduler.h"
#include "system.h"

static bool cdrom_init(char *playlist) {
	return true;
}

EM_JS(bool, cdrom_start, (int trk, int loop), {
	xsystem35.cdPlayer.play(trk, loop == 0 ? 1 : 0);
	return 1;
});

EM_JS(void, cdrom_stop, (void), {
	xsystem35.cdPlayer.stop();
});

static void cdrom_exit(void) {
	cdrom_stop();
}

static void cdrom_reset(void) {
	cdrom_stop();
}

static bool cdrom_getPlayingInfo(cd_time *info) {
	scheduler_on_event(SCHEDULER_EVENT_AUDIO_CHECK);

	int t = EM_ASM_INT_V( return xsystem35.cdPlayer.getPosition(); );
	if (!t)
		return false;
	info->t = t & 0xff;
	t >>= 8;
	info->f = t % CD_FPS;
	t /= CD_FPS;
	info->s = t % 60;
	info->m = t / 60;
	return true;
}

static bool cdrom_is_available(void) {
	return EM_ASM_INT({ return xsystem35.cdPlayer.hasAudioTrack(); });
}

cdromdevice_t cdrom_emscripten = {
	.init = cdrom_init,
	.exit = cdrom_exit,
	.reset = cdrom_reset,
	.start = cdrom_start,
	.stop = cdrom_stop,
	.getpos = cdrom_getPlayingInfo,
	.is_available = cdrom_is_available,
};
