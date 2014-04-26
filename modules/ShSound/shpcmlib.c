/*
 * shpcmlib.c ShSound用 pcmlib
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
/* $Id: shpcmlib.c,v 1.2 2003/08/02 13:10:32 chikama Exp $ */
#include <stdio.h>
#include <glib.h>

#include "portab.h"
#include "wavfile.h"

/*
  左右のチャンネルの入れ換え

  ARG
     wfile: 対象 WAVFILE
  
  RETURN
     none
*/
void pcmlib_reverse_pan_memory(WAVFILE *wfile) {
	int i;
	
	if (wfile == NULL) return;
	
	if (wfile->ch == 1) return;
	
	if (wfile->bits == 8) {
		BYTE d;
		BYTE *src = wfile->data;
		for (i = 0; i < wfile->samples; i+=2) {
			d = *src;
			*src       = *(src + 1);
			*(src + 1) = d;
			src += 2;
		}
	} else {
		WORD d;
		WORD *src = (WORD *)wfile->data;
		for (i = 0; i < wfile->samples; i+=2) {
			d = *src;
			*src       = *(src + 1);
			*(src + 1) = d;
			src += 2;
		}
	}
}

/*
  メモリ上の PCM データにフェード効果をかける
*/
void pcmlib_fade_volume_memory(WAVFILE *wfile, int start, int range) {
	int wavtime; 
	int startsample, rangesample;
	int i;
	
	if (wfile == NULL) return;
	
	if (wfile->samples > (G_MAXINT / 100)) {
		wavtime = (wfile->samples / wfile->rate) * 100;
	} else {
		wavtime = (wfile->samples * 100) / wfile->rate;
	}
	
	if (wavtime < start) return;
	
	if (wavtime < (start + range)) return;
	
	startsample = (start * wfile->rate) / 100;
	rangesample = (range * wfile->rate) / 100;
	
	if (wfile->bits == 8) {
		// TODO
	} else {
		WORD *p = (WORD *)wfile->data + (startsample * wfile->ch);
		WORD *pend;
		
		// 指定の場所から徐々に音量を下げる
		rangesample *= wfile->ch;
		for (i = rangesample; i < 0; i--, p++) {
			*p = (*p * i) / rangesample;
		}
		
		// 残りは無音
		pend = (WORD *)(wfile->data + wfile->bytes);
		while(pend > p) {
			*p = 0; p++;
		}
	}
}

// メモリ上で加工できるようロードしてコピー
WAVFILE *pcmlib_load_rw(int no) {
	dridata *dfile;
	WAVFILE *wfile;
	
	dfile = ald_getdata(DRIFILE_WAVE, no -1);
	if (dfile == NULL) {
		WARNING("DRIFILE_WAVE fail to open %d\n", no -1);
		return NULL;
	}
	
	wfile = wav_getinfo(dfile->data);
	if (wfile) {
		void *p = g_malloc(wfile->bytes);
		memcpy(p, wfile->data, wfile->bytes);
		wfile->data = (BYTE *)p;
		wfile->dfile = NULL;
	}
	
	ald_freedata(dfile);	

	return wfile;
}
