/*
 * s39init.h  System39.ini read
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
/* $Id: s39init.h,v 1.1 2003/04/25 17:23:55 chikama Exp $ */

#ifndef __S39INIT_H__
#define __S39INIT_H__

#include <stdbool.h>

// One volume-valancer channel, as defined by VolumeValancer[] in System39.ini.
struct volume_channel {
	char *label;  // UTF-8 label, or NULL if this channel index is unused
	int vol;      // 0-100
	bool mute;
};

// Reads System39.ini / Volume.sav and applies the volumes. Returns true if the
// game defines any volume channels.
bool s39ini_init(void);

// Saves the current volumes to Volume.sav (called at exit).
void s39ini_remove(void);

// Applies the current vol/mute settings to the music server.
void s39ini_setvol(void);

// True if the loaded game defines any volume channels.
bool s39ini_available(void);

// Returns the channel table. Entries with a NULL label are unused; *count is
// set to the number of table entries to scan.
struct volume_channel *s39ini_channels(int *count);

#endif
