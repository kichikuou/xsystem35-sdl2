/*
 * audio_oss.h  oss lowlevel acess
 *
 * Copyright (C) 1997-1998 Masaki Chikama (Wren) <chikama@kasumi.ipl.mech.nagoya-u.ac.jp>
 *               1998-                           <masaki-c@is.aist-nara.ac.jp>
 * rewrited      2000-     Fumihiko Murata <fmurata@p1.tcnet.ne.jp>
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
/* $Id: audio_oss.h,v 1.3 2002/08/18 09:35:29 chikama Exp $ */

#ifndef __AUDIO_OSS__
#define __AUDIO_OSS__

typedef struct {
	int fd;
	char *dev;
} audio_oss_t;

typedef struct {
	char *mdev;
	int vols_org[SOUND_MIXER_NRDEVICES];
	int connect[MIX_NRDEVICES];
	int mid;
} mixer_oss_t;

#if 0
typedef struct {
	int mdev;
	int mmask;
} mixer_item_data_oss_t;
#endif

extern int oss_init(audiodevice_t *dev, char *devdsp, char *devmix);

#endif /* !__AUDIO_OSS__ */
