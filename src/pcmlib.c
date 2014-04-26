/*
 * pcmlib.c  PCM miscライブラリ
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
/* $Id: pcmlib.c,v 1.8 2003/08/02 13:10:32 chikama Exp $ */

#include "config.h"

#include <glib.h>
#include <string.h>

#include "portab.h"
#include "system.h"
#include "ald_manager.h"
#include "dri.h"
#include "wavfile.h"
#include "pcmlib.h"

extern WAVFILE *ogg_getinfo(char *data, long size);

/**
 * noL と noR の .WAV をロードし、左右合成
 *
 * @param noL: 左の WAV ファイルの番号
 * @param noR: 右の WAV ファイルの番号
 * @return   : 合成後の WAVFILE データ
 */
WAVFILE *pcmlib_mixlr(int noL, int noR) {
	WAVFILE *wfileL, *wfileR, *wfile;
	
	wfileL = pcmlib_load(noL);
	wfileR = pcmlib_load(noR);
	if (wfileL == NULL || wfileR == NULL) {
		goto eexit;
	}
	
	wfile = wav_mix(wfileL, wfileR);
	if (wfile == NULL) {
		goto eexit;
	}
	
	pcmlib_free(wfileL);
	pcmlib_free(wfileR);
	
	wfile->dfile = NULL;
	
	return wfile;
	    
 eexit:
	pcmlib_free(wfileL);
	pcmlib_free(wfileR);
	
	return NULL;
}

/**
 * 指定の番号の .WAV|.OGG をロードする。
 * @param no: DRIファイル番号
 * @return: WAVFILE object
 */
WAVFILE *pcmlib_load(int no) {
	dridata *dfile;
	WAVFILE *wfile;
	
	dfile = ald_getdata(DRIFILE_WAVE, no -1);
	if (dfile == NULL) {
		WARNING("DRIFILE_WAVE fail to open %d\n", no -1);
		return NULL;
	}
	
	wfile = wav_getinfo(dfile->data);
	if (wfile == NULL) {
		wfile = ogg_getinfo(dfile->data, dfile->size);
		if (wfile == NULL) {
			WARNING("not .wav or .ogg file\n");
			ald_freedata(dfile);
			return NULL;
		}
	}
	
	wfile->dfile = dfile;
	
	return wfile;
}

/**
 * pcmlib_{load|mixlr}で読み込んだWAVFILEの解放
 * @param wfile: 解放するデータ
 * @return なし
 */
void pcmlib_free(WAVFILE *wfile) {
	if (wfile == NULL) return;
	
	if (wfile->dfile) {
		ald_freedata((dridata *)(wfile->dfile));
	} else {
		g_free(wfile->data);
	}
	
	g_free(wfile);
}
