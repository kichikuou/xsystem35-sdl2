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
#include "music_server.h"
#include "music_cdrom.h"
#include "cdrom.h"

enum {
	CDROM_START,
	CDROM_STOPCHECK,
	CDROM_LOOPCHECK,
	CDROM_STOP,
	CDROM_NOP
};

int muscd_init() {
	int st = cd_init(&prv.cddev);
	int ret;
	
	if (st == -1) {
		prv.cd_valid = FALSE;
		ret = NG;
	} else {
		prv.cd_valid = TRUE;
		prv.cdrom.dev = &prv.cddev;
		prv.cdrom.st = CDROM_NOP;
		prv.cdrom.in_play = FALSE;
		ret = OK;
	}
	
	return ret;
}

int muscd_exit() {
	if (prv.cd_valid) {
		prv.cddev.exit();
	}
	return OK;
}

int muscd_start(int trk, int loop) {
	prv.cdrom.track = trk;
	prv.cdrom.loop = loop;
	prv.cdrom.cnt = 0;
	
	prv.cdrom.st = CDROM_START;
	prv.cdrom.in_play = FALSE;
	
	memset(&prv.cdrom.time, 0, sizeof(cd_time));
	
	return OK;
}

int muscd_stop() {
	prv.cdrom.st = CDROM_STOP;
	return OK;
}

cd_time muscd_getpos() {
	cd_time tm;
	prv.cddev.getpos(&tm);
	return tm;
}

int muscd_cb() {
	cdobj_t *obj = &prv.cdrom;
	
	switch(obj->st) {
	case CDROM_START:
		obj->in_play = TRUE;
		obj->dev->start(obj->track);
		obj->st = CDROM_STOPCHECK;
		break;
	case CDROM_STOPCHECK:
		if (NG == obj->dev->getpos(&(obj->time))) {
                        obj->st = CDROM_LOOPCHECK;
                }
		break;
	case CDROM_LOOPCHECK:
		obj->cnt++;
		if (obj->loop == 0) {
			obj->st = CDROM_START;
			break;
		}
		if (--obj->loop == 0) {
			obj->st = CDROM_STOP;
		} else {
			obj->st = CDROM_START;
		}
                break;
	case CDROM_STOP:
                obj->dev->stop();
                
                obj->in_play = FALSE;
                obj->st = CDROM_NOP;
                break;
        case CDROM_NOP:
                break;
	}
	
	return OK;
}
