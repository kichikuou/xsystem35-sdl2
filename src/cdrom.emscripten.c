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

#include <emscripten.h>
#include "portab.h"
#include "cdrom.h"

static int  cdrom_init(char *);
static int  cdrom_exit();
static int  cdrom_start(int);
static int  cdrom_stop();
static int  cdrom_getPlayingInfo(cd_time *);

#define cdrom cdrom_emscripten
cdromdevice_t cdrom = {
	cdrom_init,
	cdrom_exit,
	cdrom_start,
	cdrom_stop,
	cdrom_getPlayingInfo,
	NULL,
	NULL
};

int cdrom_init(char *name) {
	return OK;
}

int cdrom_exit() {
	cdrom_stop();
	return OK;
}

int cdrom_start(int trk) {
	EM_ASM_ARGS({ xsystem35.cdPlayer.play($0); }, trk);
	return OK;
}

int cdrom_stop() {
	EM_ASM( xsystem35.cdPlayer.stop(); );
	return OK;
}

int cdrom_getPlayingInfo (cd_time *info) {
	info->t = info->m = info->s = info->f = 999;
	return NG;
}
