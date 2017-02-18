/*
 * mixer_alsa.c  ALSA mixer lowlevel access (for 0.9.x)
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
/* $Id: mixer_alsa09.c,v 1.3 2002/09/01 11:54:51 chikama Exp $ */

#include "config.h"
#include "audio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alsa/asoundlib.h>

#include "audio_alsa.h"

#define MIXER_ELEMENT_MAX 100

/*
  level: 0-100
*/
static void mixer_set_level(audiodevice_t *dev, int ch, int level) {
	mixer_alsa_t *amix = (mixer_alsa_t *)dev->data_mix;
}

static int mixer_get_level(audiodevice_t *dev, int ch) {
	mixer_alsa_t *amix = (mixer_alsa_t *)dev->data_mix;

	return 0;
}

void mixer_exit(audiodevice_t *dev) {
	free(dev->data_mix);
}

int mixer_init(audiodevice_t *dev) {
        mixer_alsa_t *mix = calloc(1, sizeof(mixer_alsa_t));
	mix->card = 0;
	mix->mix_dev = 0;
	dev->data_mix = mix;
	return 0;
}

