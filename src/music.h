/*
 * music.h  music interface
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
/* $Id: music_client.h,v 1.7 2003/11/16 15:29:52 chikama Exp $ */

#ifndef __MUSIC_CLIENT_H__
#define __MUSIC_CLIENT_H__

#include "portab.h"
#include "bgm.h"
#include "midi.h"
#include "music_cdrom.h"

enum MixDevice {
	MIX_MASTER,
	MIX_CD,
	MIX_MIDI,
	MIX_PCM
};

/* init and exit */
bool mus_init(int audio_buffer_size);
void mus_exit(void);
void mus_reset(void);

/* midi related function */
bool mus_midi_start(int no, int loop);
void mus_midi_stop(void);
void mus_midi_pause(void);
void mus_midi_unpause(void);
void mus_midi_get_playposition(midiplaystate *state);
bool mus_midi_set_flag(int mode, int index, int val);
int mus_midi_get_flag(int mode, int index);
bool mus_midi_get_state(void);

/* pcm (Scommand) related function */
bool mus_pcm_start(int no, int loop);
bool mus_pcm_mix(int noL, int noR, int loop);
void mus_pcm_stop(int msec);
bool mus_pcm_load(int no);
bool mus_pcm_get_playposition(int *pos);
bool mus_pcm_get_state(void);

/* fader/mixer related function */
bool mus_mixer_fadeout_start(int device, int time, int volume, int stop);
bool mus_mixer_fadeout_get_state(int device);
void mus_mixer_fadeout_stop(int device);
int mus_mixer_get_level(int device);
bool mus_mixer_set_level(int device, int level);

/* wav (command2F) / ShSound related function */
bool mus_wav_load(int ch, int num);
bool mus_wav_load_data(int ch, uint8_t *buf, uint32_t len);
void mus_wav_unload(int ch);
bool mus_wav_play(int ch, int loop);
void mus_wav_stop(int ch);
int mus_wav_get_playposition(int ch);
bool mus_wav_fadeout_start(int ch, int time, int volume, int stop);
void mus_wav_fadeout_stop(int ch);
bool mus_wav_fadeout_get_state(int ch);
void mus_wav_waitend(int ch);
void mus_wav_waittime(int ch, int time);
int mus_wav_wavtime(int ch);
bool mus_wav_load_lrsw(int ch, int num);

/* volume valaner */
bool mus_vol_set_valance(int *vols, int num);

#endif /* MUSIC_CLIENT_H__ */
