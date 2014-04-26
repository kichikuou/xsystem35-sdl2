/*
 * wavmix.c WAV ファイルのミックス
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
/* $Id: wavmix.c,v 1.14 2003/04/22 16:34:28 chikama Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include "portab.h"
#include "audio.h"
#include "system.h"
#include "wavfile.h"

WAVFILE *wav_mix(WAVFILE *wfileL, WAVFILE *wfileR) {
	WAVFILE *wfileM;

	int rate;
	int channel;
	int bits;
	int    i;
	int max_samples;
	int min_samples;
	
	if (wfileL->rate != wfileR->rate) return NULL;
	if (wfileL->ch   != wfileR->ch)   return NULL;
	if (wfileL->bits != wfileR->bits) return NULL;
	
	rate    = wfileL->rate;
        channel = 2;
	bits    = wfileL->bits;
	
	if (wfileL->bytes > wfileR->bytes) {
		max_samples = wfileL->samples;
		min_samples = wfileR->samples;
	} else {
		max_samples = wfileR->samples;
		min_samples = wfileL->samples;
	}
	
	// printf("max = %d, min = %d\n",max_samples, min_samples);
	
	wfileM = (WAVFILE *)malloc(sizeof(WAVFILE));
	if (wfileM == NULL) {
		NOMEMERR();
	}
	
	wfileM->rate    = rate;
	wfileM->ch      = channel;
	wfileM->samples = max_samples;
	wfileM->bits    = bits;
	wfileM->bytes   = max_samples * 2 * (bits/8);
	wfileM->data    = (char *)malloc(max_samples * 2 * (bits/8));
	
	if (wfileM->data == NULL) {
		NOMEMERR();
	}
	
	switch(bits) {
	case 8: {
		BYTE *srcR = wfileR->data;
		BYTE *srcL = wfileL->data;
		BYTE *dst  = wfileM->data;
		
		if (channel == 1) {
			for (i = 0; i < min_samples; i++) {
				*(dst + i) = (*(srcL + i) + *(srcR + i)) >> 1;
			}
			if (min_samples == max_samples) break;
			if (wfileL->samples > wfileR->samples) {
				for (i = min_samples; i < max_samples; i++) {
					*(dst + i) = *(srcL + i);
				}
			} else {
				for (i = min_samples; i < max_samples; i++) {
					*(dst + i) = *(srcR + i);
				}
			}
		} else {
			for (i = 0; i < min_samples; i++) {
				*(dst + i * 2    ) = *(srcL + i);
				*(dst + i * 2 + 1) = *(srcR + i);
			}
			if (min_samples == max_samples) break;
			if (wfileL->samples > wfileR->samples) {
				for (i = min_samples; i < max_samples; i++) {
					*(dst + i * 2    ) = *(srcL + i);
					*(dst + i * 2 + 1) = 0x80;
				}
			} else {
				for (i = min_samples; i < max_samples; i++) {
					*(dst + i * 2    ) = 0x80;
					*(dst + i * 2 + 1) = *(srcR + i);
				}
			}
		}
		break;
	}
	case 16: {
		WORD *srcR = (WORD *)wfileR->data;
		WORD *srcL = (WORD *)wfileL->data;
		WORD *dst  = (WORD *)wfileM->data;
		
		if (channel == 1) {
			for (i = 0; i < min_samples; i++) {
				*(dst + i) = (*(srcL + i) + *(srcR + i)) >> 1;
			}
			if (min_samples == max_samples) break;
			if (wfileL->samples > wfileR->samples) {
				for (i = min_samples; i < max_samples; i++) {
					*(dst + i) = *(srcL + i);
				}
			} else {
				for (i = min_samples; i < max_samples; i++) {
					*(dst + i) = *(srcR + i);
				}
			}
		} else {
			for (i = 0; i < min_samples; i++) {
				*(dst + i * 2    ) = *(srcL + i);
				*(dst + i * 2 + 1) = *(srcR + i);
			}
			if (min_samples == max_samples) break;
			if (wfileL->samples > wfileR->samples) {
				for (i = min_samples; i < max_samples; i++) {
					*(dst + i * 2    ) = *(srcL + i);
					*(dst + i * 2 + 1) = 0;
				}
			} else {
				for (i = min_samples; i < max_samples; i++) {
					*(dst + i * 2    ) = 0;
					*(dst + i * 2 + 1) = *(srcR + i);
				}
			}
		}
		
		break;
	}
	default:
		break;
	}
	return wfileM;
}
