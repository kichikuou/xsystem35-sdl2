/*
 * music_client.h  music client …Ù ¨
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
#include "cdrom.h"
#include "midi.h"
#include "audio.h"
#include "wavfile.h"

extern int musclient_init();
extern int musclient_exit(void);

/* cdrom related function */
extern int mus_cdrom_start(int track, int loop);
extern int mus_cdrom_stop(void);
extern int mus_cdrom_get_playposition(cd_time *tm);
extern int mus_cdrom_get_maxtrack(void);
extern boolean mus_cdrom_get_state(void);

/* midi related function */
extern int mus_midi_start(int no, int loop);
extern int mus_midi_stop(void);
extern int mus_midi_pause(void);
extern int mus_midi_unpause(void);
extern int mus_midi_get_playposition(midiplaystate *state);
extern int mus_midi_set_flag(int mode, int index, int val);
extern int mus_midi_get_flag(int mode, int index);
extern boolean mus_midi_get_state(void);

/* pcm (Scommand) related function */
extern int mus_pcm_start(int no, int loop);
extern int mus_pcm_mix(int noL, int noR, int loop);
extern int mus_pcm_stop(int msec);
extern int mus_pcm_load(int no);
extern int mus_pcm_get_playposition(int *pos);
extern int mus_pcm_check_ability(int bit, int rate, int ch, boolean *able);
extern boolean mus_pcm_get_state(void);

/* fader/mixer related function */
extern int mus_mixer_fadeout_start(int device, int time, int volume, int stop);
extern boolean mus_mixer_fadeout_get_state(int device);
extern int mus_mixer_fadeout_stop(int device);
extern int mus_mixer_get_level(int device);

/* wav (command2F) related function */
extern int mus_wav_load(int ch, int num);
extern int mus_wav_unload(int ch);
extern int mus_wav_play(int ch, int loop);
extern int mus_wav_stop(int ch);
extern int mus_wav_get_playposition(int ch);
extern int mus_wav_fadeout_start(int ch, int time, int volume, int stop);
extern int mus_wav_fadeout_stop(int ch);
extern boolean mus_wav_fadeout_get_state(int ch);
extern int mus_wav_waitend(int ch);
extern int mus_wav_waittime(int ch, int time);
extern int mus_wav_wavtime(int ch);
extern int mus_wav_sendfile(int ch, WAVFILE *wfile);
extern int mus_wav_load_lrsw(int ch, int num);

/* Music Stream */
extern int mus_bgm_play(int no, int time, int vol);
extern int mus_bgm_stop(int no, int time);
extern int mus_bgm_stopall(int time);
extern int mus_bgm_fade(int no, int time, int vol);
extern int mus_bgm_getpos(int no);
extern int mus_bgm_getlen(int no);
extern int mus_bgm_wait(int no, int timeout);
extern int mus_bgm_waitpos(int no, int index);
extern int mus_bgm_getlength(int no);

/* volume valaner */
extern int mus_vol_set_valance(int *vols, int num);

#endif /* MUSIC_CLIENT_H__ */
