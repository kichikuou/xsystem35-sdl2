/*
 * mixer_oss.c  oss mixer lowlevel access
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
/* $Id: mixer_oss.c,v 1.13 2003/01/04 17:01:02 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#if defined(__FreeBSD__)
#  if __FreeBSD__ < 4
#    include <machine/soundcard.h>
#  else
#    include <sys/soundcard.h>
#  endif
#elif defined(__OpenBSD__) || defined(__NetBSD__)
#  include <soundcard.h>
#else
#  include <sys/soundcard.h>
#endif
#include <glib.h>

#include "audio.h"
#include "audio_oss.h"
#include "system.h"

static char *mixlabels[] = SOUND_DEVICE_LABELS;

/*
  level: 0-100
*/
static void mixer_set_level(audiodevice_t *dev, int ch, int level) {
	mixer_oss_t *oss = (mixer_oss_t *)dev->data_mix;
	int fd, lv;
	
	if (level < 0)   level = 0;
	if (level > 100) level = 100;
	
	if ((fd = open(oss->mdev, O_RDWR)) < 0) return;
	
	lv = (((oss->vols_org[oss->connect[ch]] & 0xff) * level) / 100) & 0xff;
	lv |= (lv << 8);
	
	if (ioctl(fd, MIXER_WRITE(oss->connect[ch]), &lv) < 0) {
		WARNING("mixer write failed (%s)\n", mixlabels[oss->connect[ch]]);
	}
	close(fd);
}

/*
  0-100
*/
static int mixer_get_level(audiodevice_t *dev, int ch) {
	mixer_oss_t *oss = (mixer_oss_t *)dev->data_mix;
	int fd, vol;
	
	if ((fd = open(oss->mdev, O_RDWR)) < 0) return 0;
	
	if (ioctl(fd, MIXER_READ(oss->connect[ch]), &vol) < 0) {
		WARNING("mixer read failed (%s)\n", mixlabels[oss->connect[ch]]);
		return 0;
	}
	close(fd);
	
	return ((vol & 0xff) * 100) / 256;
}

static int mixer_init(audiodevice_t *dev, char *devmix) {
	mixer_oss_t *mix;
	int i, fd;
	
	if ((fd = open(devmix, O_RDWR)) < 0) return NG;
	
	mix = g_new0(mixer_oss_t, 1);
	mix->mdev = devmix;
	dev->data_mix = mix;
	
	for (i = 0; i < SOUND_MIXER_NRDEVICES; i++) {
		int lv;
		if (0 > ioctl(fd, MIXER_READ(i), &lv)) {
			WARNING("mixer read failed (%s)\n", mixlabels[i]);
		} else {
			NOTICE("mixer read ok (%s)(%d)\n", mixlabels[i], lv & 0xff);
		}
		
		mix->vols_org[i] = lv;
	}
	
	/* 初期接続設定 */
	mix->connect[MIX_MASTER] = SOUND_MIXER_VOLUME;
	mix->connect[MIX_CD]     = SOUND_MIXER_CD;
	mix->connect[MIX_MIDI]   = SOUND_MIXER_SYNTH;
	//mix->connect[MIX_MIDI]   = SOUND_MIXER_LINE;
	mix->connect[MIX_PCM]    = SOUND_MIXER_PCM;
	
	return OK;
}

static int mixer_exit(audiodevice_t *dev) {
	mixer_oss_t *mix = dev->data_mix;
	int i, fd;
	
	if ((fd = open(mix->mdev, O_RDWR)) < 0) return NG;
	
	for (i = 0; i < SOUND_MIXER_NRDEVICES; i++) {
		int lv = mix->vols_org[i];
		if (ioctl(fd, MIXER_WRITE(i), &lv) < 0) {
			WARNING("mixer read failed (%s)\n", mixlabels[i]);
		}
	}
	
	return OK;
}

static void mixer_get_allelements(audiodevice_t *dev, char **list[], int *n) {
	*list = mixlabels;
	*n    = SOUND_MIXER_NRDEVICES;
}

static void mixer_set_element(audiodevice_t *dev, int src, int dst) {
	mixer_oss_t *mix = dev->data_mix;
	mix->connect[src]= dst;
}

