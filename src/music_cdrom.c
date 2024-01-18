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

static char *dev = CDROM_DEVICE;

void muscd_set_devicename(const char *name) {
	if (0 == strcmp("none", name)) dev = NULL;
	else                           dev = strdup(name);
}

int muscd_init(void) {
	// In the download edition of Daiakuji, SS command plays music from *BA.ald.
	if (ald_get_maxno(DRIFILE_BGM) > 0) {
		prv.cddev = &cdrom_bgm;
	} else {
		prv.cddev = cd_init(dev);
		if (!prv.cddev) {
			return NG;
		}
	}
	prv.cddev->init(dev);
	prv.cd_current_track = 0;
	return OK;
}

int muscd_init_bgm(DRIFILETYPE type, int base_no) {
	muscd_exit();
	prv.cddev = &cdrom_bgm;
	return musbgm_init(type, base_no);
}

int muscd_exit(void) {
	if (prv.cddev) {
		prv.cddev->exit();
		prv.cddev = NULL;
	}
	return OK;
}

int muscd_reset(void) {
	if (prv.cddev) {
		prv.cddev->reset();
		prv.cd_current_track = 0;
	}
	return OK;
}

int muscd_start(int trk, int loop) {
	if (!prv.cddev)
		return NG;
	if (trk == prv.cd_current_track)
		return OK;
	prv.cddev->stop();

	prv.cddev->start(trk, loop);
	prv.cd_current_track = trk;
	return OK;
}

int muscd_stop(void) {
	if (!prv.cddev)
		return NG;
	prv.cddev->stop();
	prv.cd_current_track = 0;
	return OK;
}

int muscd_getpos(int *t, int *m, int *s, int *f) {
	if (!prv.cddev)
		return NG;
	cd_time info;
	if (prv.cddev->getpos(&info) != OK)
		return NG;
	*t = info.t;
	*m = info.m;
	*s = info.s;
	*f = info.f;
	return OK;
}

int muscd_get_maxtrack(void) {
	if (!prv.cddev)
		return 0;
	return prv.cd_maxtrk;
}

bool muscd_is_available(void) {
	return prv.cddev;
}
