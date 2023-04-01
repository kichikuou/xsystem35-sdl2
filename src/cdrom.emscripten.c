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
#include "nact.h"

static int frame_of_getpos = -1;

int cdrom_init(char *name) {
	return OK;
}

EM_JS(int, cdrom_start, (int trk, int loop), {
	xsystem35.cdPlayer.play(trk, loop == 0 ? 1 : 0);
	return xsystem35.Status.OK;
});

EM_JS(int, cdrom_stop, (), {
	xsystem35.cdPlayer.stop();
	return xsystem35.Status.OK;
});

int cdrom_exit(void) {
	cdrom_stop();
	return OK;
}

int cdrom_reset(void) {
	cdrom_stop();
	return OK;
}

int cdrom_getPlayingInfo(cd_time *info) {
	if (nact->frame_count == frame_of_getpos)
		nact->wait_vsync = TRUE;
	frame_of_getpos = nact->frame_count;

	int t = EM_ASM_INT_V( return xsystem35.cdPlayer.getPosition(); );
	if (!t)
		return NG;
	info->t = t & 0xff;
	FRAMES_TO_MSF(t >> 8, &info->m, &info->s, &info->f);
	return OK;
}

cdromdevice_t cdrom_emscripten = {
	cdrom_init,
	cdrom_exit,
	cdrom_reset,
	cdrom_start,
	cdrom_stop,
	cdrom_getPlayingInfo,
	NULL,
	NULL
};
