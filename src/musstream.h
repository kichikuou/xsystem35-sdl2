/*
 * musstream.h  wrapper for file/pipe/memory 
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
/* $Id: musstream.h,v 1.3 2003/11/09 15:06:13 chikama Exp $ */

#ifndef __MUSSTREAM_H__
#define __MUSSTREAM_H__

#include <stdio.h>
#include "wavfile.h"

struct _musstream {
	int (*seek)(struct _musstream *this, int off, int where);
        int (*read)(struct _musstream *this, void *ptr, int size, int maxnum);
        int (*close)(struct _musstream *this);
	int type;
	
	boolean byteswap;
	
	boolean lrswap8;
	boolean lrswap16;
	
	boolean lrmix8;
	boolean lrmix16;
	
	union {
		struct {
			FILE *fp;
		} stdio;
		struct {
			void *base;
			void *cur;
			void *end;
			void *lpp;
		} mem;
		struct {
			FILE *fp;
			char *cmd;
			char *buf;
		} pipe;
		
		struct {
			void *base;
			void *cur;
			void *end;
			void *lpp;
		} memm[2]; // multi stream
		
		struct {
			void *vf; /* OggVorbis_File */
		} ogg;

		struct {
			void *base;
			void *cur;
			void *end;
			void *wf;
			void *obuf;
			void *obufcur;
			int left;
		} ima;
        } hidden;
};
typedef struct _musstream musstream_t;

extern musstream_t *ms_file(char *filename);
extern musstream_t *ms_pipe(char *cmd);
extern musstream_t *ms_memory(void *prt, int size);

/* musstream_wav.c */
extern musstream_t *ms_wav(WAVFILE *snd);
extern musstream_t *ms_wav2(WAVFILE *snd, int size, int looppoint);

/* musstream_ogg.c */
extern musstream_t *ms_ogg(WAVFILE *snd);

/* musstream_wav_ima.c */
extern musstream_t *ms_wav_ima(WAVFILE *snd);

#endif /* __MUSSTREAM_H__*/
