/*
 * musstream_wav_ima.c music stream for .wav IMA ADPCM file (from sox/ima_rw.c)
 *
 * Copyright (C) 1997-1998 Masaki Chikama (Wren) <chikama@kasumi.ipl.mech.nagoya
-u.ac.jp>
 *               1998-                           <masaki-c@is.aist-nara.ac.jp>
 *
 *  Based on wavfile.c (c)  Erik de Castro Lopo  erikd@zip.com.au
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
/* $Id: musstream_wav_ima.c,v 1.1 2003/11/09 15:06:13 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <errno.h>
#include <signal.h>

#include "portab.h"
#include "musstream.h"
#include "wavfile.h"

/*
 *
 * Lookup tables for IMA ADPCM format
 *
 */
#define ISSTMAX 88

static const int imaStepSizeTable[ISSTMAX + 1] = {
	7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34,
	37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130, 143,
	157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408, 449, 494,
	544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552,
	1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327, 3660, 4026,
	4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442,
	11487, 12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623,
	27086, 29794, 32767
};

#define imaStateAdjust(c) (((c)<4)? -1:(2*(c)-6))
/* +0 - +3, decrease step size */
/* +4 - +7, increase step size */
/* -0 - -3, decrease step size */
/* -4 - -7, increase step size */

static unsigned char imaStateAdjustTable[ISSTMAX+1][8];

void Ima_initImaTable(void) {
	int i, j, k;
	for (i = 0; i <= ISSTMAX; i++) {
		for (j = 0; j<8; j++) {
			k = i + imaStateAdjust(j);
			if      (k < 0)       k = 0;
			else if (k > ISSTMAX) k = ISSTMAX;
			imaStateAdjustTable[i][j] = k;
		}
	}
}

static void ImaExpandS(int ch, int chans, const unsigned char *ibuff, short *obuff, int n, int o_inc) {
	const unsigned char *ip;
	int i_inc;
	short *op;
	int i, val, state;

	ip = ibuff + 4*ch;     /* input pointer to 4-byte block state-initializer   */
	i_inc = 4*(chans-1);   /* amount by which to incr ip after each 4-byte read */
	val = (short)(ip[0] + (ip[1]<<8)); /* need cast for sign-extend */
	state = ip[2];
	if (state > ISSTMAX) {
		fprintf(stderr, "IMA_ADPCM block ch%d initial-state (%d) out of range\n", ch, state);
		state = 0;
	}
	/* specs say to ignore ip[3] , but write it as 0 */
	ip += 4+i_inc;

	op = obuff;
	*op = val;      /* 1st output sample for this channel */
	op += o_inc;

	for (i = 1; i < n; i++) {
		int step,dp,c,cm;

		if (i&1) {         /* 1st of pair */
 			cm = *ip & 0x0f;
		} else {
			cm = (*ip++)>>4;
			if ((i&7) == 0)  /* ends the 8-sample input block for this channel */
				ip += i_inc;   /* skip ip for next group */ 
		}
		step = imaStepSizeTable[state];
		/* Update the state for the next sample */
		c = cm & 0x07;
		state = imaStateAdjustTable[state][c];

		dp = 0;
		if (c & 4) dp += step;
		step = step >> 1;
		if (c & 2) dp += step;
		step = step >> 1;
		if (c & 1) dp += step;
		step = step >> 1;
		dp += step;
		if (c != cm) {
			val -= dp;
			if (val<-0x8000) val = -0x8000;
		} else {
			val += dp;
			if (val>0x7fff) val = 0x7fff;
		}
		*op = val;
		op += o_inc;
	}
	return;
}

int ImaBytesPerBlock(WORD chans, WORD samplesPerBlock) {
	int n;
	n = (samplesPerBlock + 14)/8 * 4 * chans;
	return n;
}

int ImaSamplesIn(int dataLen, WORD chans, WORD blockAlign, WORD samplesPerBlock) {
	int n, m;
	
	if (samplesPerBlock) {
		n = (dataLen / blockAlign) * samplesPerBlock;
		m = (dataLen % blockAlign);
	} else {
		n = 0;
		m = blockAlign;
	}
	if (m >= 4*chans) {
		m -= 4 * chans;
		m /= 4 * chans;
		m = 8 * m + 1;
		if (samplesPerBlock && m > samplesPerBlock) m = samplesPerBlock;
		n += m;
	}
	return n;
}

static int wav_read(musstream_t *this, void *ptr, int size, int nmemb) {
	int len = MIN(size * nmemb, this->hidden.ima.end - this->hidden.ima.cur);
	WAVFILE *w = (WAVFILE *)this->hidden.ima.wf;
	char *obuff;
	int i, offset = 0;
	
	if (len <= 0) return 0;
	
	obuff = (char *)ptr;
	
	if (this->hidden.ima.left < len) {
		int n;
		memcpy(obuff, this->hidden.ima.obufcur, this->hidden.ima.left);
		offset += this->hidden.ima.left;
		obuff += this->hidden.ima.left;
		len -= this->hidden.ima.left;
		n = w->samples_per_block;
		ImaExpandS(0, 1, this->hidden.ima.cur, this->hidden.ima.obuf, n, 1);
		this->hidden.ima.cur += w->bytes_per_block;
		this->hidden.ima.obufcur = this->hidden.ima.obuf;
		this->hidden.ima.left = n*2;
	}
	memcpy(obuff, this->hidden.ima.obufcur, len);
	this->hidden.ima.left -= len;
	this->hidden.ima.obufcur += len;
	offset += len;
	
	if (this->lrswap8) {
		BYTE d;
		BYTE *src = ptr;
		int sample = offset / 2;
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
		int sample = offset / 4;
		for (i = 0; i < sample; i++) {
			d          = *src;
			*src       = *(src + 1);
			*(src + 1) = d;
			src += 2;
		}
	}
	
	return offset;
}

static int wav_seek(musstream_t *this, int offset, int whence) {
	void *newpos = NULL;
	
	switch(whence) {
	case SEEK_SET:
		newpos = this->hidden.ima.base + offset;
		break;
	case SEEK_CUR:
		return 0;
	case SEEK_END:
		return 0;
	}
	
	if (newpos < this->hidden.ima.base) {
		newpos = this->hidden.ima.base;
	}
	if (newpos > this->hidden.ima.end) {
		newpos = this->hidden.ima.end;
	}
	
	this->hidden.ima.cur = newpos;
	
	return (this->hidden.ima.cur - this->hidden.ima.base); 
}

static int wav_close(musstream_t *this) {
	g_free(this->hidden.ima.obuf);
	g_free(this);
	return 0;
}

musstream_t *ms_wav_ima(WAVFILE *snd) {
	musstream_t *ms = g_new0(musstream_t, 1);

	ms->hidden.ima.wf = snd;
	ms->hidden.ima.base = snd->data;
	ms->hidden.ima.cur = snd->data;
	ms->hidden.ima.end = snd->data + snd->bytes;
	ms->hidden.ima.obuf = g_malloc(snd->samples_per_block * 2);
	ms->hidden.ima.obufcur = ms->hidden.ima.obuf;
	ms->hidden.ima.left = 0;
	
	ms->read = wav_read;
	ms->seek = wav_seek;
	ms->close = wav_close;

	return ms;
}

