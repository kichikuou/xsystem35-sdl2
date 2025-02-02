/*
 * music_cdrom.h  music server CDROM part
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
/* $Id: music_cdrom.h,v 1.1 2002/08/18 09:35:29 chikama Exp $ */

#ifndef __MUSIC_CDROM_H__
#define __MUSIC_CDROM_H__

#include "portab.h"
#include "ald_manager.h"

void muscd_set_playlist(const char *);
bool muscd_init(void);
bool muscd_init_bgm(DRIFILETYPE type, int base_no);
void muscd_exit(void);
void muscd_reset(void);
bool muscd_start(int trk, int loop);
void muscd_stop(void);
bool muscd_getpos(int *t, int *m, int *s, int *f);
int muscd_get_maxtrack(void);
bool muscd_is_available(void);

#endif /* __MUSIC_CDROM_H__ */
