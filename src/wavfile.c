/*
 * wavfile.c  check and load .WAV file
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
/* $Id: wavfile.c,v 1.16 2003/11/16 15:29:52 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <glib.h>

#include "portab.h"
#include "system.h"
#include "wavfile.h"
#include "LittleEndian.h"

#define PCM_WAVE_FORMAT   	0x0001
#define IMA_ADPCM_FORMAT        0x0011

extern int ImaBytesPerBlock(WORD chans, WORD samplesPerBlock);
extern int ImaSamplesIn(int dataLen, WORD chans, WORD blockAlign, WORD samplesPerBlock);

static int header_check(char *wave_buf, WAVFILE *wfile) {
	char *ptr;
	int l, fmt_len, extsize;
	int fmt, dwAvgBytesPerSec, wBitsPerSample;
	
	ptr = wave_buf;
	if (0 != strncmp(ptr, "RIFF", 4)) {
		// don't  print for ogg
		//NOTICE("Cannot find RIFF file marker\n"); 
		return NG;
	}
	
	l = LittleEndian_getDW(ptr, 4);
	ptr += 8;
	
	if (0 != strncmp(ptr, "WAVE", 4)) {
		WARNING("Cannot find WAV file marker\n");
		return NG;
	}
	
	ptr += 4;
	
	if (0 != strncmp(ptr, "fmt ", 4)) {
		WARNING("Cannot find fmt file marker\n");
		return NG;
	}
	
	fmt_len = l = LittleEndian_getDW(ptr, 4);
	if (l < 16) {
		WARNING("fmt length (%d) must be >= 16\n", l);
		return NG;
	}
	
	ptr += 8;
	fmt         = LittleEndian_getW(ptr, 0);
	wfile->ch   = LittleEndian_getW(ptr, 2);
	wfile->rate = LittleEndian_getDW(ptr, 4);
	dwAvgBytesPerSec   = LittleEndian_getDW(ptr, 8);
	wfile->block_align = LittleEndian_getW(ptr, 12);
	wBitsPerSample     = LittleEndian_getW(ptr, 14);
	
	l -= 16;
	
	extsize = 0;
	if (fmt != PCM_WAVE_FORMAT) {
		if (l >= 2) {
			extsize = LittleEndian_getW(ptr, 16);
			l -= 2;
		} else {
			WARNING("Cannot find fmtext file marker\n");
			return NG;
		}
	}
	if  (extsize > l) {
		WARNING("wExtSize inconsistent with wFmtLen\n");
		return NG;
	}
	
	switch (fmt) {
	case IMA_ADPCM_FORMAT:
		if (extsize < 2) {
			WARNING("wExtSize must >= 2 \n");
			return NG;
		}
		if (wBitsPerSample != 4) {
			WARNING("Only handle 4bit-IMA ADPCM\n");
			return NG;
		}
		wfile->samples_per_block = LittleEndian_getW(ptr, 18);
		wfile->bytes_per_block  = ImaBytesPerBlock(wfile->ch, wfile->samples_per_block);
		if (wfile->bytes_per_block > wfile->block_align || wfile->samples_per_block % 8 != 1) {
			WARNING("Sample per block =%d, block align = %d\n",
				wfile->samples_per_block, wfile->block_align);
			return NG;
		}
		l -= 2;
		break;
	case PCM_WAVE_FORMAT:
		break;
	default:
		WARNING("Not supported format %d\n", fmt);
		return NG;
	}
	
	ptr += fmt_len;

	// IMA ADPCM has "fact" 
	if (0 == strncmp(ptr, "fact", 4)) {
		l = LittleEndian_getDW(ptr, 4);
		ptr += (l+8);
	}
	
	if (0 != strncmp(ptr, "data", 4)) {
		WARNING("Cannot find data file marker\n");
		return NG;
	}
	
	wfile->bytes = LittleEndian_getDW(ptr, 4);
	ptr += 8;
	wfile->data = ptr;
	
	switch (fmt) {
	case IMA_ADPCM_FORMAT:
	{
		int num_samples;
		num_samples = ImaSamplesIn(wfile->bytes, wfile->ch, wfile->block_align, wfile->samples_per_block);
		wfile->samples = num_samples;
		wfile->bits = 16;
		wfile->type = SND_WAVE_IMAADPCM;
		break;
	}
	case PCM_WAVE_FORMAT:
		wfile->samples = wfile->bytes / wfile->block_align;
		wfile->bits = wBitsPerSample;
		wfile->type = SND_WAVE;
		break;
	}

	return OK;
}

WAVFILE * wav_getinfo(char *data) {
	WAVFILE *wfile = g_new(WAVFILE, 1);
	
	if (NG == header_check(data, wfile)) {
		g_free(wfile);
		return NULL;
	}
	
	return wfile;
}
