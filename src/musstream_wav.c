/*
 * musstream_wav.c  music strema for .wav file
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
/* $Id: musstream_wav.c,v 1.3 2003/08/22 17:09:23 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <errno.h>
#include <signal.h>

#include "portab.h"
#include "musstream.h"
#include "wavfile.h"

static int wav_read(musstream_t *this, void *ptr, int size, int nmemb) {
	int len = MIN(size * nmemb, this->hidden.mem.end - this->hidden.mem.cur);
	int i;
	
	if (len <= 0) return 0;
	
#ifdef WORDS_BIGENDIAN
	if (this->byteswap) { //  16bitのときのみ立てる
		WORD *src = (WORD *)this->hidden.mem.cur;
		WORD *dst = (WORD *)ptr;
		int sample = len /2;
		for (i = 0; i < sample; i++) {
			*dst = GINT16_FROM_LE(*src);
			src++; dst++;
		}
	} else
#endif
	memcpy(ptr, this->hidden.mem.cur, len);
	
	if (this->lrswap8) {
		BYTE d;
		BYTE *src = ptr;
		int sample = len / 2;
		for (i = 0; i < sample; i++) {
			d          = *src;
			*src       = *(src + 1);
			*(src + 1) = d;
			src += 2;
		}
	}
	
	if (this->lrswap16) {
		WORD d;
		WORD *src = ptr;
		int sample = len / 4;
		for (i = 0; i < sample; i++) {
			d          = *src;
			*src       = *(src + 1);
			*(src + 1) = d;
			src += 2;
		}
	}
	
	this->hidden.mem.cur += len;
	
	return len;
}

static int wav_seek(musstream_t *this, int offset, int whence) {
	void *newpos = NULL;
	
	switch(whence) {
	case SEEK_SET:
		newpos = this->hidden.mem.base + offset;
		break;
	case SEEK_CUR:
		newpos = this->hidden.mem.cur  + offset;
		break;
	case SEEK_END:
		newpos = this->hidden.mem.end  + offset;
		break;
	}
	
	if (newpos < this->hidden.mem.base) {
		newpos = this->hidden.mem.base;
	}
	if (newpos > this->hidden.mem.end) {
		newpos = this->hidden.mem.end;
	}
	
	this->hidden.mem.cur = newpos;
	
	return (this->hidden.mem.cur - this->hidden.mem.base); 
}

static int wav_seek2(struct _musstream *this, int offset, int where) {
	void *newpos = NULL;
	
	switch(where) {
	case SEEK_SET:
		newpos = this->hidden.mem.lpp + offset;  // うひひ
		break;
	case SEEK_CUR:
		newpos = this->hidden.mem.cur + offset;
		break;
	case SEEK_END:
		newpos = this->hidden.mem.end + offset;
		break;
	}
	
	if (newpos < this->hidden.mem.base) {
		newpos = this->hidden.mem.lpp;
	}
	if (newpos > this->hidden.mem.end) {
		newpos = this->hidden.mem.end;
	}
	
	this->hidden.mem.cur = newpos;
	
	return (this->hidden.mem.cur - this->hidden.mem.base); 
}


static int wav_close(musstream_t *this) {
	g_free(this);
	return 0;
}


musstream_t *ms_wav(WAVFILE *snd) {
	musstream_t *ms = g_new0(musstream_t, 1);

	ms->hidden.mem.base = snd->data;
	ms->hidden.mem.cur = snd->data;
	ms->hidden.mem.end = snd->data + snd->bytes;
	
#ifdef WORDS_BIGENDIAN
	if (snd->bits == 16) {
		ms->byteswap = TRUE;
	}
#endif
	ms->read = wav_read;
	ms->seek = wav_seek;
	ms->close = wav_close;

	return ms;
}

musstream_t *ms_wav2(WAVFILE *snd, int size, int looptop) {
	musstream_t *ms = g_new0(musstream_t, 1);
	
	ms->hidden.mem.base = snd->data;
	ms->hidden.mem.cur = snd->data;
	ms->hidden.mem.end = snd->data + size;
	ms->hidden.mem.lpp = snd->data + looptop;
	
#ifdef WORDS_BIGENDIAN
	if (snd->bits == 16) {
		ms->byteswap = TRUE;
	}
#endif
	ms->seek = wav_seek2;
	ms->read = wav_read;
	ms->close = wav_close;
	
	return ms;
}
