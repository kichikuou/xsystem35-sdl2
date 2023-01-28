/*
 * cdrom.c  CD-ROM control wrapper
 *
 * Copyright (C) 2000-  Masaki Chikama (Wren) <masaki-c@is.aist-nara.ac.jp>
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
/* $Id: cdrom.c,v 1.20 2002/08/18 09:35:29 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

#include "portab.h"
#include "cdrom.h"
#include "system.h"

#if defined(ENABLE_CDROM_LINUX)
extern cdromdevice_t cdrom_linux;
#define NATIVE_CD_DEVICE &cdrom_linux

#elif defined(ENABLE_CDROM_BSD)
extern cdromdevice_t cdrom_bsd;
#define NATIVE_CD_DEVICE &cdrom_bsd

#elif defined(ENABLE_CDROM_EMSCRIPTEN)
extern cdromdevice_t cdrom_emscripten;
#define NATIVE_CD_DEVICE &cdrom_emscripten

#elif defined(ENABLE_CDROM_ANDROID)
extern cdromdevice_t cdrom_android;
#define NATIVE_CD_DEVICE &cdrom_android

#else

extern cdromdevice_t cdrom_empty;
#define NATIVE_CD_DEVICE &cdrom_empty
#endif

#ifdef ENABLE_CDROM_MP3
extern cdromdevice_t cdrom_mp3;
#endif

cdromdevice_t *cd_init(const char *dev) {
#if defined(ENABLE_CDROM_EMSCRIPTEN) || defined(ENABLE_CDROM_ANDROID)
	return NATIVE_CD_DEVICE;
#else
	struct stat st;
	if (dev && stat(dev, &st) && (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)))
		return NATIVE_CD_DEVICE;

#ifdef ENABLE_CDROM_MP3
	return &cdrom_mp3;
#else
	WARNING("no cdrom device available");
	return NULL;
#endif
#endif  // ENABLE_CDROM_EMSCRIPTEN || ENABLE_CDROM_ANDROID
}
