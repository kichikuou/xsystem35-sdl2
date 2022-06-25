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
#include "music_private.h"
#include "music_cdrom.h"
#include "cdrom.h"

int muscd_init(void) {
	int st = cd_init(&prv.cddev);
	if (st == -1) {
		prv.cd_valid = FALSE;
		return NG;
	}
	prv.cd_valid = TRUE;
	prv.cd_current_track = 0;
	return OK;
}

int muscd_exit(void) {
	if (prv.cd_valid) {
		prv.cddev.exit();
	}
	return OK;
}

int muscd_reset(void) {
	if (prv.cd_valid) {
		prv.cddev.reset();
		prv.cd_current_track = 0;
	}
	return OK;
}

int muscd_start(int trk, int loop) {
	if (trk == prv.cd_current_track)
		return OK;
	prv.cddev.stop();

	prv.cddev.start(trk, loop);
	prv.cd_current_track = trk;
	return OK;
}

int muscd_stop(void) {
	prv.cddev.stop();
	prv.cd_current_track = 0;
	return OK;
}

int muscd_getpos(cd_time *tm) {
	return prv.cddev.getpos(tm);
}
