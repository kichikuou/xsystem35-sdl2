/*
 * audio_alsa.c  alsa lowlevel acess (for 0.9.x)
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
/* $Id: audio_alsa09.c,v 1.7 2004/10/31 04:18:06 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <glib.h>
#include <sys/poll.h>

#include "system.h"
#include "audio.h"
#include "audio_alsa.h"
#include "mixer_alsa.c"
#include "music_pcm.h"

#ifndef SND_PROTOCOL_VERSION
#define SND_PROTOCOL_VERSION(major, minor, subminor) (((major)<<16)|((minor)<<8)|(subminor))
#endif

#define BUFFERSIZE 1536

static int audio_open(audiodevice_t *audio, chanfmt_t fmt);
static int audio_close(audiodevice_t *audio);
static int audio_write(audiodevice_t *audio, unsigned char *buf,int cnt);

static int audio_open(audiodevice_t *dev, chanfmt_t fmt) {
	audio_alsa_t *alsa = (audio_alsa_t *)dev->data_pcm;
	struct pollfd pfds;
	snd_pcm_hw_params_t *hwparams;
#if SND_LIB_VERSION >= SND_PROTOCOL_VERSION(1,0,0)
	snd_pcm_uframes_t len;
#else
	snd_pcm_sframes_t len;
#endif
	int err, periods;
	
	if (0 > snd_pcm_open(&alsa->handle, alsa->dev, SND_PCM_STREAM_PLAYBACK, SND_PCM_ASYNC)) {
		WARNING("Opening audio device %d failed\n", alsa->dev);
		goto _err_exit;
	}
	snd_pcm_hw_params_alloca(&hwparams);
	
	if (0 > snd_pcm_hw_params_any(alsa->handle, hwparams)) {
		WARNING("param get failed\n");
		goto _err_exit;
	}
	
	if (0 > snd_pcm_hw_params_set_access(alsa->handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED)) {
		WARNING("set access fail\n");
		goto _err_exit;
	}
	
	if (0 > snd_pcm_hw_params_set_format(alsa->handle, hwparams, fmt.bit == 16 ? SND_PCM_FORMAT_S16 : SND_PCM_FORMAT_U8)) {
		WARNING("set format fail\n");
		goto _err_exit;
	}
	
	if (0 > snd_pcm_hw_params_set_channels(alsa->handle, hwparams, fmt.ch)) {
		WARNING("set channel fail\n");
		goto _err_exit;
	}
	
#if SND_LIB_VERSION >= SND_PROTOCOL_VERSION(1,0,0)
	int tmp = fmt.rate;
	err = snd_pcm_hw_params_set_rate_near(alsa->handle, hwparams, &tmp, 0);
#else
	err = snd_pcm_hw_params_set_rate_near(alsa->handle, hwparams, fmt.rate, 0);
#endif
	if (err < 0) {
		WARNING("set rate fail\n");
		goto _err_exit;
	}

	if (tmp != fmt.rate) {
		WARNING("set rate fail\n");
		goto _err_exit;
	}

	if (0 > snd_pcm_hw_params_set_periods_integer(alsa->handle, hwparams)) {
		WARNING("set periods fail\n");
		goto _err_exit;
	}
	
	periods = 2;
	if (0 > snd_pcm_hw_params_set_periods_min(alsa->handle, hwparams, &periods, 0)) {
		WARNING("set priods min fail\n");
		goto _err_exit;
	}
	
	if (0 > snd_pcm_hw_params_set_buffer_size(alsa->handle, hwparams, BUFFERSIZE)) {
		WARNING("set buffer fail\n");
		goto _err_exit;
	}	  
	
	if (0 > snd_pcm_hw_params(alsa->handle, hwparams)) {
		WARNING("set hw parmas fail\n");
		goto _err_exit;
	}	  
#if SND_LIB_VERSION >= SND_PROTOCOL_VERSION(1,0,0)
	
        snd_pcm_hw_params_get_buffer_size(hwparams, &len);
#else
	len = snd_pcm_hw_params_get_buffer_size(hwparams);
#endif
	dev->buf.len = (int)len;
	snd_pcm_poll_descriptors(alsa->handle, &pfds, 1);
	dev->fd = pfds.fd;
	return OK;
	
 _err_exit:
	if (alsa->handle) {
		snd_pcm_close(alsa->handle);
	}
	dev->fd = -1;
	return NG;
}

static int audio_close(audiodevice_t *dev) {
	audio_alsa_t *alsa = (audio_alsa_t *)dev->data_pcm;
	
	dev->fd = -1;
	if (alsa->handle) {
		snd_pcm_close(alsa->handle);
	}
	return OK;
}

static int audio_write(audiodevice_t *dev, unsigned char *buf, int cnt) {
	audio_alsa_t *alsa = (audio_alsa_t *)dev->data_pcm;
	int len, err;
	
	if (NULL == alsa->handle) return NG;
	
	if (cnt == 0) return 0;
	
	len = snd_pcm_bytes_to_frames(alsa->handle, cnt);
	while(0 > (err = snd_pcm_writei(alsa->handle, buf, len))) {
		if (err == -EPIPE) {
			if (0 > snd_pcm_prepare(alsa->handle)) {
				return -1;
			}
			continue;
		} else if (err == -ESTRPIPE) {
			while(-EAGAIN == (err = snd_pcm_resume(alsa->handle))) {
				sleep(1);
			}
			if (err < 0) {
				if (0 > snd_pcm_prepare(alsa->handle)) {
					return -1;
				}
			}
			continue;
		}
		if (0 > snd_pcm_prepare(alsa->handle)) {
			return -1;
		}
	}
	
	return (snd_pcm_frames_to_bytes(alsa->handle, err));
}

static int alsa_exit(audiodevice_t *dev) {
	if (dev == NULL) return OK;
	
	mixer_exit(dev);
	g_free(dev->data_pcm);
	g_free(dev->data_mix);
	
	return OK;
}

int alsa_init(audiodevice_t *dev, char *devname, boolean automix) {
	audio_alsa_t *alsa;
	
	alsa = g_new0(audio_alsa_t, 1);
	alsa->dev = devname;
	dev->data_pcm = alsa;
	
	mixer_init(dev);
	
	dev->id      = AUDIO_PCM_ALSA;
	dev->fd      = -1;
	dev->open    = audio_open;
	dev->close   = audio_close;
	dev->write   = audio_write;
	dev->mix_set = mixer_set_level;
	dev->mix_get = mixer_get_level;
	dev->exit    = alsa_exit;
	
	NOTICE("ALSA Initilize OK\n");
	return OK;
}
