/*
 * audio_alsa.c  alsa lowlevel acess (for 0.5.x)
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
/* $Id: audio_alsa.c,v 1.19 2003/04/22 16:34:28 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/asoundlib.h>
#include <glib.h>

#include "system.h"
#include "audio.h"
#include "audio_alsa.h"
#include "mixer_alsa.c"

static int audio_open(audiodevice_t *audio, chanfmt_t fmt);
static int audio_close(audiodevice_t *audio);
static int audio_write(audiodevice_t *audio, unsigned char *buf,int cnt);

static int audio_open(audiodevice_t *dev, chanfmt_t fmt) {
	audio_alsa05_t *alsa = (audio_alsa05_t *)dev->data_pcm;
	
	snd_pcm_channel_params_t p;
	snd_pcm_channel_setup_t  s;
	
	if (0 > snd_pcm_open(&alsa->pcm_handle, alsa->card, alsa->pcm_dev, SND_PCM_OPEN_PLAYBACK)) {
		WARNING("Opening audio device %d failed\n", alsa->pcm_dev);
		goto _err_exit;
	}
	
	memset(&alsa->info, 0, sizeof(snd_pcm_channel_info_t));
	
	if (0 > snd_pcm_channel_info(alsa->pcm_handle, &alsa->info)) {
		WARNING("param get failed\n");
		goto _err_exit;
	}
	
	memset(&p, 0, sizeof(p));
	p.mode = SND_PCM_MODE_BLOCK;
	p.start_mode = SND_PCM_START_DATA;
	p.channel    = SND_PCM_CHANNEL_PLAYBACK;
	p.stop_mode  = SND_PCM_STOP_STOP;
	p.buf.block.frag_size = 1536;
	p.buf.block.frags_max =  6;
	p.buf.block.frags_min =  1;
	p.format.rate   = fmt.rate;
	p.format.format = fmt.bit == 8 ? SND_PCM_SFMT_U8 : SND_PCM_SFMT_S16;
	p.format.voices = fmt.ch;
	p.format.interleave = 1;
	alsa->silence  = fmt.bit == 8 ? 0x80 : 0;
	
	if (0 > snd_pcm_channel_params(alsa->pcm_handle, &p)) {
		WARNING("Unable to set channel params\n");
		goto _err_exit;
	}
	
	if (0 > snd_pcm_channel_prepare(alsa->pcm_handle, SND_PCM_CHANNEL_PLAYBACK)) {
		WARNING("Unable to prepare channel\n");
		goto _err_exit;
	}
	
	memset(&s, 0, sizeof(s));
	s.mode    = SND_PCM_MODE_BLOCK;
	s.channel = SND_PCM_CHANNEL_PLAYBACK;
	
	if (0 > snd_pcm_channel_setup(alsa->pcm_handle, &s)) {
		WARNING("Unable to obtain setup\n");
		goto _err_exit;
	}
	
	dev->buf.len = s.buf.block.frag_size;
	dev->fd = snd_pcm_file_descriptor(alsa->pcm_handle, SND_PCM_CHANNEL_PLAYBACK); 
	return OK;
	
 _err_exit:
	dev->fd = -1;
	return NG;
}

static int audio_close(audiodevice_t *dev) {
	audio_alsa05_t *alsa = (audio_alsa05_t *)dev->data_pcm;
	snd_pcm_channel_status_t st;
	int err, bc;

	dev->fd = -1;
	if (alsa->pcm_handle) {
		memset(&st, 0, sizeof(st));
		if (0 > (err = snd_pcm_channel_status(alsa->pcm_handle, &st))) {
			WARNING("playback status err(%s)\n", snd_strerror(err));
			return snd_pcm_close(alsa->pcm_handle);
		}
		while (st.status == SND_PCM_STATUS_RUNNING) {
			memset(&st, 0, sizeof(st));
			bc = st.count;
			snd_pcm_channel_status(alsa->pcm_handle, &st);
			if (bc == st.count) {
				snd_pcm_channel_flush(alsa->pcm_handle, SND_PCM_CHANNEL_PLAYBACK);
			}
			usleep(1000);
		}
		err = snd_pcm_close(alsa->pcm_handle);
		alsa->pcm_handle = NULL;
		return err;
	} else {
		return OK;
	}
}

static int audio_write(audiodevice_t *dev, unsigned char *buf, int cnt) {
	audio_alsa05_t *alsa = (audio_alsa05_t *)dev->data_pcm;
	snd_pcm_channel_status_t st;
	int ret;
	
	if (alsa->pcm_handle == NULL) return NG;
	
	if (cnt == 0) return 0;
	
	if (cnt < dev->buf.len) {
		memset(buf + cnt, alsa->silence, dev->buf.len - cnt);
	}
	
	memset(&st, 0, sizeof(st));
	if (0 > (ret = snd_pcm_channel_status(alsa->pcm_handle, &st))) {
		WARNING("cannot get status %s\n", snd_strerror(ret));
		return 0;
	}
	
	if (st.status != SND_PCM_STATUS_RUNNING &&
	    st.status != SND_PCM_STATUS_PREPARED) {
		snd_pcm_channel_prepare(alsa->pcm_handle, SND_PCM_CHANNEL_PLAYBACK);
	}
	
	ret = snd_pcm_write(alsa->pcm_handle, buf, dev->buf.len);
	if (ret < 0) {
		WARNING("write %s\n", snd_strerror(ret));
	}
	
	return dev->buf.len;
}

static int alsa_exit(audiodevice_t *dev) {
	if (dev == NULL) return OK;
	
	mixer_exit(dev);
	g_free(dev->data_pcm);
	g_free(dev->data_mix);
	
	return OK;
}

int alsa_init(audiodevice_t *dev, int cardno, int pcmdev, int mixdev, boolean automix) {
	snd_ctl_t *ctl_handle;
	snd_ctl_hw_info_t *hwinfo;
	audio_alsa05_t *alsa;
	int i, j, ncards;
	snd_pcm_t *pcm_handle;
	
	/* サウンドカードが存在するかチェック */
	ncards = snd_cards();
	if (ncards < 1) {
		WARNING("No ALSA device found\n");
		return NG;
	}
	
	if (cardno == -1 && pcmdev == -1) {
		/* 最初に見つかった使用可能なカード */
		if (NULL == (hwinfo = (snd_ctl_hw_info_t *)malloc(sizeof(*hwinfo) * ncards))) {
			NOMEMERR();
			return NG;
		}
		for (i = 0; i < ncards; i++) {
			if (0 > snd_ctl_open(&ctl_handle, i)) {
				WARNING("Can't Open Card %d\n", i);
				free(hwinfo);
				return NG;
			}
			if (0 > snd_ctl_hw_info(ctl_handle, hwinfo + i)) {
				WARNING("Can't Get Card(%d) info\n", i);
				free(hwinfo);
				return NG;
			}
			snd_ctl_close(ctl_handle);
			for (j = 0; j < hwinfo[i].pcmdevs; j++) {
				//open してチェック OK なら outへ
				if (0 > snd_pcm_open(&pcm_handle, i, j, SND_PCM_OPEN_PLAYBACK)) continue;
				cardno = i; pcmdev  = j;
				snd_pcm_close(pcm_handle);
				NOTICE("ALSA Use(%d:%d) device\n", i, j);
				goto out;
			}
		}
		// 使用可能なデバイスが１つもない場合
		WARNING("Can't Get Card(%d) info\n", i);
		return NG;
	out:
		free(hwinfo);
	} else {
		/* 指定のカード */
		if (0 > snd_ctl_open(&ctl_handle, cardno)) {
			WARNING("Can't Open Card %d\n", cardno);
			return NG;
		}
		snd_ctl_close(ctl_handle);
		
		/* 指定の pcmdevice がカードの中にあるかどうか */
		if (pcmdev >= 0 && pcmdev < hwinfo[cardno].pcmdevs) {
			//opne してチェック
			if (0 > snd_pcm_open(&pcm_handle, cardno, pcmdev, SND_PCM_OPEN_PLAYBACK)) {
				WARNING("Can't Open (%d:%d)\n", cardno, pcmdev);
				return NG;
			}
			snd_pcm_close(pcm_handle);
			NOTICE("ALSA Use(%d:%d) device\n", cardno, pcmdev);
		} else {
			WARNING("Can't Open (%d:%d)\n", cardno, pcmdev);
			return NG;
		}
	}
	
	if (mixer_init(dev, cardno, mixdev, &hwinfo) < 0) {
		return NG;
	}
	
	alsa = g_new0(audio_alsa05_t, 1);
	
	alsa->card = cardno;
	alsa->pcm_dev = pcmdev;
	alsa->automixer = automix;
	
	dev->data_pcm = alsa;
	
	dev->id      = AUDIO_PCM_ALSA;
	dev->fd      = -1;
	dev->open    = audio_open;
	dev->close   = audio_close;
	dev->write   = audio_write;
	dev->mix_set = mixer_set_level;
	dev->mix_get = mixer_get_level;
	dev->exit    = alsa_exit;
	
	NOTICE("ALSA Initilize OK\n");
	return 0;
}
