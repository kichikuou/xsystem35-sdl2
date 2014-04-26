/*
 * audio.h  audiodevice control
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
/* $Id: audio.h,v 1.13 2003/01/04 17:01:02 chikama Exp $ */

#ifndef __AUDIO_H__
#define __AUDIO_H__

#include "portab.h"

#define MIX_NRDEVICES 4
#define MIX_MASTER 0
#define MIX_CD     1
#define MIX_MIDI   2
#define MIX_PCM    3


struct _chanfmt {
        int rate; /* Hz */
        int bit;  /* 1 or 2 */ 
        int ch;   /* 1 or 2 */
};
typedef struct _chanfmt chanfmt_t;


struct _audiodevbuf {
	int len;    // buffere length
	BYTE *cur;  // current filling buffer adderss
	BYTE *end;  // current buffer end 

	int sw; // cur is b[sw]

	BYTE *b[2];  // buffer
	boolean ready[2]; // buffer is ready to send device
	
	int lenmax; // work
};
typedef struct _audiodevbuf audiodevbuf_t;


struct _audiodevice {
	int id;  // audio type id

	int fd;  // discpriter for write
	
	void *data_pcm;
	void *data_mix;
	
	audiodevbuf_t buf;
	
	boolean opened;
	
	int  (*open)(struct _audiodevice *dev, chanfmt_t fmt);
	int  (*close)(struct _audiodevice *dev);
	int  (*write)(struct _audiodevice *dev, BYTE *buf, int cnt);
	void (*mix_set)(struct _audiodevice *dev, int ch, int val);
	int  (*mix_get)(struct _audiodevice *dev, int ch);
	int  (*exit)(struct _audiodevice *dev);
};
typedef struct _audiodevice audiodevice_t;


enum {
	AUDIO_PCM_OSS,
	AUDIO_PCM_ALSA,
	AUDIO_PCM_ALSA09,
	AUDIO_PCM_ESD,
	AUDIO_PCM_SDL,
	AUDIO_PCM_ARTS,
	AUDIO_PCM_SUN,
	AUDIO_PCM_ANY,
	AUDIO_PCM_DMY
};

extern int audio_init(audiodevice_t *dev);
extern void audio_set_output_device(char c);
extern void audio_set_pcm_devicename(char *name);
extern void audio_set_mixer_devicename(char *name);
extern void audio_set_alsa_mixser_element(char *fn);


#endif /* __AUDIO_H__ */
