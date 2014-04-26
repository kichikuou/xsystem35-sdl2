/*
 * audio.c  audio acesss wrapper
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
/* $Id: audio.c,v 1.17 2003/01/25 01:34:50 chikama Exp $ */


#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

#include "audio.h"

#ifndef AUDIODEV_OSS
#define AUDIODEV_OSS "/dev/dsp"
#endif
#ifndef MIXERDEV_OSS
#define MIXERDEV_OSS "/dev/mixer"
#endif


/* extern variable */
static char*  audio_dev_dsp;
static char*  audio_dev_mixer;

/* static variable */
static boolean mode_onlyone = FALSE; /* if true, only probe selected device */

#ifdef DEFAULT_AUDIO_MODE
static int mode = DEFAULT_AUDIO_MODE;
#else 
static int mode = AUDIO_PCM_ANY;
#endif

#ifdef ENABLE_OSS
extern int oss_init(audiodevice_t *dev, char *devdsp, char *devmix);
#endif /* ENABLE_OSS */

#ifdef ENABLE_ALSA
#ifdef ENABLE_ALSA05
extern int alsa_init(audiodevice_t *dev, int cardno, int pcmdev, int mixdev, boolean automix);
#endif /* ENABLE_ALSA05 */

#ifdef ENABLE_ALSA09
extern int alsa_init(audiodevice_t *dev, char *hw, boolean automix);
#endif /* ENABLE_ALSA09 */
#endif
	
#ifdef ENABLE_ESD
extern int esd_init(audiodevice_t *, char *);
#endif /* ENABLE_ESD */

#ifdef ENABLE_SUNAUDIO
extern int sunaudio_init(audiodevice_t *dev, char *devaudio, char *devaudioctl);
#endif /* ENABLE_SUNAUDIO */

int audio_init(audiodevice_t *a) {
	switch(mode) {
	case AUDIO_PCM_ANY:
#ifdef ENABLE_OSS
	case AUDIO_PCM_OSS:
		if (audio_dev_dsp == NULL) {
			audio_dev_dsp = AUDIODEV_OSS;
		}
		if (audio_dev_mixer == NULL) {
			audio_dev_mixer = MIXERDEV_OSS;
		}
		if (oss_init(a, audio_dev_dsp, audio_dev_mixer) == 0) break;
		if (mode_onlyone) break;
#endif
#ifdef ENABLE_SUNAUDIO
	case AUDIO_PCM_SUN:
		if (sunaudio_init(a, audio_dev_dsp, audio_dev_mixer) == 0) break;
		if (mode_onlyone) break;
#endif
#ifdef ENABLE_ALSA
#ifdef ENABLE_ALSA05
	case AUDIO_PCM_ALSA:
	{
		int card = -1, pcm = -1;
		if (audio_dev_dsp) {
			sscanf(audio_dev_dsp, "%d:%d\n", &card, &pcm);
		}
		if (alsa_init(a, card, pcm, -1, FALSE) == 0) break;
		if (mode_onlyone) break;
	}
#endif
#ifdef ENABLE_ALSA09
	case AUDIO_PCM_ALSA:
		if (audio_dev_dsp == NULL) {
			audio_dev_dsp = "hw:0";
		}
		if (alsa_init(a, audio_dev_dsp, FALSE) == 0) break;
		if (mode_onlyone) break;
#endif
#endif
#ifdef ENABLE_ESD
	case AUDIO_PCM_ESD:
		if (esd_init(a, NULL) == 0) break;
		if (mode_onlyone) break;
#endif
#ifdef ENABLE_SDL
	case AUDIO_PCM_SDL:
		// if (sdlaudio_initilize(a) == 0) break;
		// if (audiomode_strict) break;
#endif
#ifdef ENABLE_ARTS
	case AUDIO_PCM_ARTS:
		// if (arts_initilize(a) == 0) break;
		// if (audiomode_strict) break;
#endif
	default:
		return -1;
	}
	return OK;
}

void audio_set_output_device(char c) {
	switch(c) {
	case 'o':
		/* OSS */
		mode = AUDIO_PCM_OSS;
		break;
	case 'e':
		/* ESD */
		mode = AUDIO_PCM_ESD;
		break;
	case 's':
		/* ALSA */
		mode = AUDIO_PCM_ALSA;
		break;
	case '0':
		/* no audio */
		mode = AUDIO_PCM_DMY;
		break;
	default:
		mode = AUDIO_PCM_ANY;
	}
	
	mode_onlyone = TRUE;
}

void audio_set_pcm_devicename(char *name) {
	if (0 == strcmp("none", name)) {
		mode = AUDIO_PCM_DMY;
	} else {
		audio_dev_dsp = strdup(name);
	}
}

void audio_set_mixer_devicename(char *name) {
        audio_dev_mixer = strdup(name);
}
