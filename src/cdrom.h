/*
 * cdrom.h  cdrom control
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
/* $Id: cdrom.h,v 1.14 2002/08/18 09:35:29 chikama Exp $ */

#ifndef __CDROM_H__
#define __CDROM_H__

#include <stdbool.h>

typedef struct {
        int t,m,s,f;
} cd_time;

typedef struct {
	bool (*init)(char *playlist);
	void (*exit)(void);
	void (*reset)(void);
	// Play through the track loop times. If loop == 0, loops forever.
	bool (*start)(int trk, int loop);
	void (*stop)(void);
	bool (*getpos)(cd_time *);
	bool (*is_available)(void);
} cdromdevice_t;

extern cdromdevice_t cdrom_bgm;

#define CD_FPS 75

#endif /* __CDROM_H__ */
