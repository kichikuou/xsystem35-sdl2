/*
 * wavfile.h
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
/* $Id: wavfile.h,v 1.12 2003/11/09 15:06:13 chikama Exp $ */

#ifndef __WAVFILE_H__
#define __WAVFILE_H__

#include "portab.h"

enum {
        SND_WAVE,
        SND_WAVE_IMAADPCM,
        SND_MP3,
        SND_OGG,
        SND_PIPE,
};

typedef struct {
	int type;
	int rate; /* Hz */
	int ch;   /* 1: Mono, 2: Stereo */
	int bits; /* 8: 8bit, 16: 16bit */
	int samples; /* number of samples */
	int bytes;   /* total bytes (sample * bits * ch) */

	//  for IMA ADPCM
	int samples_per_block;
	int bytes_per_block;
	int block_align;
	
	void *data;
	
	void *dfile; /* for dridata */
} WAVFILE;

extern WAVFILE *wav_getinfo(char *data);
extern WAVFILE *wav_mix(WAVFILE *wl, WAVFILE *wr);

#endif /* WAVFILE_H__ */

