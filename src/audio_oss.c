/*
 * audio_oss.c  oss lowlevel acess
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
/* $Id: audio_oss.c,v 1.20 2003/04/22 16:34:28 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
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

#include "system.h"
#include "audio.h"
#include "audio_oss.h"
#include "mixer_oss.c"

#ifndef AFMT_S16_NE
# ifdef WORDS_BIGENDIAN
#  define AFMT_S16_NE AFMT_S16_BE
# else
#  define AFMT_S16_NE AFMT_S16_LE
# endif
#endif


#ifndef AUDIODEV_OSS
#define AUDIODEV_OSS "/dev/dsp"
#endif


static int  audio_open(audiodevice_t *audio, chanfmt_t fmt);
static int  audio_close(audiodevice_t *audio);
static int  audio_wait_for_finish(audio_oss_t *oss);
static int  audio_write(audiodevice_t *audio, unsigned char *buf, int cnt);
static void audio_flush(audiodevice_t *audio);

static int audio_open(audiodevice_t *audio, chanfmt_t fmt) {
	audio_oss_t *oss = (audio_oss_t *)audio->data_pcm;
	int frag, blk_size;
	int tmp;
	
	if (oss->fd > -1) audio_close(audio);
	
	if ((oss->fd = open(oss->dev, O_WRONLY, 0)) < 0) {
		perror("open");
		WARNING("Opening audio device %s failed\n", oss->dev);
		return -1;
	}
	
	ioctl(oss->fd, SNDCTL_DSP_RESET, 0);
	
	tmp = (fmt.bit == 16 ? AFMT_S16_NE : AFMT_U8);
	if (ioctl(oss->fd, SNDCTL_DSP_SAMPLESIZE, &tmp) < 0) {
		WARNING("Setting DSP to %d bits\n", tmp);
		goto _err_exit;
	}
	
	tmp = fmt.ch - 1;
	if (ioctl(oss->fd, SNDCTL_DSP_STEREO, &tmp) < 0) {
		WARNING("Unable to set DSP to %s mode\n", fmt.ch == 2 ? "Stereo" : "Mono");
		goto _err_exit;
	}
	
	tmp = fmt.rate;
	if (ioctl(oss->fd, SNDCTL_DSP_SPEED, &tmp) < 0) {
		WARNING("Unable to set audio sampling rate\n");
		goto _err_exit;
	}
#if !defined(__powerpc__) /* does not exist(work well?) on powerpc */
#ifdef SNDCTL_DSP_SETFRAGMENT
	// for (frag = 0; (0x01 << frag) < audio_pcm->Bps; frag++);
	// frag = ( 0x0100 << 16 ) + 0x0008;
	frag = 0x0002000d;        /* two fragments, for low latency */
	if ( ioctl(oss->fd, SNDCTL_DSP_SETFRAGMENT, &frag) < 0 ) {
		WARNING("Unable to set fragsize\n");
	}
#endif	
#endif
	if (ioctl(oss->fd, SNDCTL_DSP_GETBLKSIZE, &blk_size) < 0) {
		WARNING("Optaining DSP's block size\n");
		goto _err_exit;
	}
	//printf("block size = %d\n", blk_size);
	audio->buf.len = blk_size;
	// audio_pcm->writed_bytes = 0;
	
	audio->fd = oss->fd;
	
	return OK;
	
 _err_exit:
	close(oss->fd);
	oss->fd = -1;
	return NG;
}

static int audio_close(audiodevice_t *audio) {
	audio_oss_t *oss = (audio_oss_t *)audio->data_pcm;
	int ret = OK;
	
	if (oss->fd > -1) {
		audio_wait_for_finish(oss);
		audio_flush(audio);
		ret = close(oss->fd);
	}
	
	audio->fd = oss->fd = -1;
	
	return ret;
}

static int audio_wait_for_finish(audio_oss_t *oss) {
	char limit;
	audio_buf_info buf_info;
	
	for (limit = 0; limit < CHAR_MAX; limit++) {
		if (ioctl(oss->fd, SNDCTL_DSP_GETOSPACE, &buf_info) < 0) {
			perror("ioctl(SNDCTL_DSP_GETOSPACE");
			break;
		}
#if 0
		printf("GETOSPACE:fragments = %d, "
		       "fragstotal = %d, "
		       "fragsize = %d, "
		       "bytes = %d\n",
		       buf_info.fragments,
		       buf_info.fragstotal,
		       buf_info.fragsize,
		       buf_info.bytes);  
#endif
		if (buf_info.fragments == buf_info.fragstotal
		    || buf_info.fragsize == 0) {
			break;
		}
	}
	
	return (limit == CHAR_MAX);
}	

static int audio_write(audiodevice_t *dev, unsigned char *buf, int cnt) {
	audio_oss_t *oss = (audio_oss_t *)dev->data_pcm;
	int rt = 0;
	
	if (cnt == 0) return 0;
	
	if (oss->fd > -1) {
		rt = write(oss->fd, buf, cnt);
		if (rt < 0) {
			perror("write");
		}
		// audio_pcm->writed_bytes += cnt;
	}
	return rt;
}

static void audio_flush(audiodevice_t *dev) {
	audio_oss_t *oss = (audio_oss_t *)dev->data_pcm;
	
	if (oss->fd > -1) {
#if 1
		if (ioctl(oss->fd, SNDCTL_DSP_SYNC) < 0) {
			perror("ioctl(SNDCTL_DSP_SYNC)");
		}
#endif
	}
}

int oss_exit(audiodevice_t *dev) {
	if (dev == NULL) return OK;
	
	mixer_exit(dev);
	
	g_free(dev->data_pcm);
	g_free(dev->data_mix);
	return OK;
}

int oss_init(audiodevice_t *dev, char *devdsp, char *devmix) {
	audio_oss_t *oss;
	
	oss = g_new0(audio_oss_t, 1);
	
	oss->fd = -1;
	oss->dev = devdsp;
	mixer_init(dev, devmix);
	
	dev->data_pcm = oss;
	
	dev->id   = AUDIO_PCM_OSS;
	dev->fd   = -1;
	dev->open = audio_open;
	dev->close = audio_close;
	dev->write = audio_write;
	dev->mix_set = mixer_set_level;
	dev->mix_get = mixer_get_level;
	dev->exit    = oss_exit;
	
	NOTICE("OSS Initilize OK\n");
	return OK;
}
