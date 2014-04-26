/*
 * mixer_esd.c  esd mixer lowlevel acess
 *
 * Copyright (C) 2000-     Fumihiko Murata <fmurata@p1.tcnet.ne.jp>
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
/* $Id: mixer_esd.c,v 1.10 2002/12/31 04:11:19 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <esd.h>
#include <glib.h>

#include "audio.h"
#include "audio_esd.h"

static char *mixlabels[] = {"esd "};


static void mixer_set_level(audiodevice_t *dev, int ch, int level) {
	audio_esd_t *esd = (audio_esd_t *)dev->data_pcm;
	int lv;
	
	if (ch == MIX_CD || ch == MIX_MIDI) return;
	
	if (level < 0)   level = 0;
	if (level > 100) level = 100;
	
	lv = (level * 256) / 100;
	
	esd_set_stream_pan(esd->fdh, esd->sid, lv, lv);
	
	esd->curvol = lv;
}

static int mixer_get_level(audiodevice_t *dev, int ch) {
	audio_esd_t *esd = (audio_esd_t *)dev->data_pcm;
	return esd->curvol;
}

static int mixer_init(audiodevice_t *dev) {
	audio_esd_t *esd = (audio_esd_t *)dev->data_pcm;
	esd->curvol = 256;
	return OK;
}

static void mixer_get_allelements(audiodevice_t *dev, char **list[], int *n) {
	*list = mixlabels;
	*n = 1;
}

static void mixer_set_element(audiodevice_t *dev, int src, int dst) {
	return;
}
