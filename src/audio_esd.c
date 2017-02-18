/*
 * audio_esd.c  esd lowlevel acess
 *
 * Copyright (C) 1999-2000 <tajiri@wizard.elec.waseda.ac.jp>
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
/* $Id: audio_esd.c,v 1.11 2003/04/22 16:34:28 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <esd.h>

#include "system.h"
#include "audio.h"
#include "audio_esd.h"
#include "mixer_esd.c"

static int audio_open(audiodevice_t *audio, chanfmt_t fmt);
static int audio_close(audiodevice_t *audio);
static int audio_write(audiodevice_t *audio, unsigned char *buf, int cnt);
static int esd_exit(audiodevice_t *dev);

/*
 * Get stream source id with name 'sig'
 *
 * Argument
 *    fd:  esd socket to host
 *    sig: stream name
 *
 * Return
 *    stream source_id
*/
static int get_stream_id(int fd, char *sig) {
	esd_info_t *all_info = NULL;
	esd_player_info_t *p_list;
	int id = -1;
	
	all_info = esd_get_all_info(fd);
	p_list = all_info->player_list;
	
	while(p_list) {
		if (strcmp(p_list->name, sig) == 0) {
			id = p_list->source_id;
			break;
		}
		p_list = p_list->next;
	}
	esd_free_all_info(all_info);
	
	return id;
}

/*
 * select settled format stream
 *
 * Argument
 *    audio: audio control object
 *    bit  : bits of esd stream
 *    rate : rate of esd stream
 *    ch   : channel of esd stream
 *
 * Return
 *     0: OK
 *    -1: Error
*/
static int audio_open(audiodevice_t *audio, chanfmt_t fmt) {
	audio_esd_t *esd = (audio_esd_t *)audio->data_pcm;
	esd_format_t efmt = ESD_STREAM;

	efmt |= fmt.bit == 8 ? ESD_BITS8 : ESD_BITS16;
	efmt |= fmt.ch  == 1 ? ESD_MONO  : ESD_STEREO;
	
	esd->fds = esd_play_stream(efmt, fmt.rate, NULL, ESDNAME_BASE);
	
	if (esd->fds < 0) {
		WARNING("Opening audio server failed\n");
		return NG;
	}
	
	esd->sid = get_stream_id(esd->fdh, ESDNAME_BASE);
	
	audio->fd = esd->fds;

	return OK;
}

/*
 * close stream (dont close, keep opened)
 *
 * Argument
 *    audio: audio control object
 *
 * Return
 *     0: OK
*/
static int audio_close(audiodevice_t *audio) {
	audio_esd_t *esd = (audio_esd_t *)audio->data_pcm;
	int ret = OK;
	
	if (esd->fds > -1) {
		ret = close(esd->fds);
	}
	
	audio->fd = esd->fds = -1;
	return ret;
}

/*
 * write stream data
 *
 * Argument
 *    audio: audio control object
 *    buf  : stream data
 *    cnt  : data length
 *
 * Return
 *     0: OK
 *    -1: NG
*/
static int audio_write(audiodevice_t *audio, unsigned char *buf, int cnt) {
	audio_esd_t *esd = (audio_esd_t *)audio->data_pcm;
	int rt = 0;
	
	if (cnt == 0) return 0;

	if (esd->fds > -1) {
		rt = write(esd->fds, buf, cnt);
		if (rt < 0) {
			perror("write");
		}
	}
	return rt;
}

int esd_exit(audiodevice_t *dev) {
	audio_esd_t *esd;
	
	if (dev == NULL) return OK;
	
	esd = (audio_esd_t *)dev->data_pcm;
	
	esd_close(esd->fdh);
	
	free(dev->data_pcm);
	free(dev->data_mix);
	return OK;
}

int esd_init(audiodevice_t *dev, char *host) {
	audio_esd_t *esd;
	int fd;
	
	if (0 > (fd = esd_open_sound(host))) {
		return NG;
	}
	
	esd = calloc(1, sizeof(audio_esd_t));
	esd->fdh = fd;  /* socket to host */
	dev->data_pcm = esd;
	
	mixer_init(dev);
	
	dev->id    = AUDIO_PCM_ESD;
	dev->fd    = -1;
	dev->open  = audio_open;
	dev->close = audio_close;
	dev->write = audio_write;
	dev->mix_set = mixer_set_level;
	dev->mix_get = mixer_get_level;
	dev->exit    = esd_exit;
	dev->buf.len = ESD_BUF_SIZE;
	
	NOTICE("ESD Initilize OK\n");
	return OK;
}
