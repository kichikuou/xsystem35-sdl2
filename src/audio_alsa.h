/*
 * audio_alsa.h  alsa lowlevel acess
 *
 * Copyright (C) 1997-1998 Masaki Chikama (Wren) <chikama@kasumi.ipl.mech.nagoya-u.ac.jp>
 *               1998-                           <masaki-c@is.aist-nara.ac.jp>
 * rewrited      1999- Fumihiko Murata <fmurata@p1.tcnet.ne.jp>
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
/* $Id: audio_alsa.h,v 1.6 2002/08/23 14:56:41 chikama Exp $ */

#ifndef __AUDIO_ALSA__
#define __AUDIO_ALSA__

#include <sys/asoundlib.h>

typedef struct {
	snd_pcm_t *pcm_handle;
	boolean automixer;  // 自動的に pcmdevice に接続されている mixer を探すか？ 
	
	int card;
	int pcm_dev;

	snd_pcm_channel_info_t info;

	int silence;


	int frag,frags;
	struct {
		int mix_dev;
		snd_mixer_eid_t eid;
	} mixer;
	
	int mm_flag;
	snd_pcm_mmap_control_t *mm_ctl;
	char *mm_data;
} audio_alsa05_t;


typedef struct {
	int card;
	int mix_dev;
	int connect[MIX_NRDEVICES];

	snd_mixer_element_t e[MIX_NRDEVICES];
	snd_mixer_element_info_t ei[MIX_NRDEVICES];
} mixer_alsa05_t;

#endif /* __AUDIO_ALSA__ */
