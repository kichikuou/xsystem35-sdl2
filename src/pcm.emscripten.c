/*
 * pcm.emscripten.c  Emscripten PCM backend
 *
 * Copyright (C) 2017 <KichikuouChrome@gmail.com>
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

#include <emscripten.h>

#include "portab.h"
#include "system.h"
#include "ald_manager.h"
#include "dri.h"
#include "music_pcm.h"
#include "music_private.h"
#include "nact.h"


int muspcm_init() {
	prv.pcm_valid = TRUE;
	return OK;
}

int muspcm_exit() {
	return OK;
}

// 番号指定のPCMファイル読み込み
int muspcm_load_no(int slot, int no) {
	dridata *dfile = ald_getdata(DRIFILE_WAVE, no -1);
	if (dfile == NULL) {
		WARNING("DRIFILE_WAVE fail to open %d\n", no -1);
		return NG;
	}

	EM_ASM_({
		xsystem35.waitPromise(function(){
			return xsystem35.audio.pcm_load($0, $1, $2);
		});
	}, slot, dfile->data, dfile->size);
#ifdef EMTERPRETIFY_ADVISE
	emscripten_sleep(0);
#endif

	ald_freedata(dfile);
	return OK;
}

int muspcm_load_mixlr(int slot, int noL, int noR) {
	dridata *dfileL = ald_getdata(DRIFILE_WAVE, noL -1);
	if (dfileL == NULL) {
		WARNING("DRIFILE_WAVE fail to open %d\n", noL -1);
		return NG;
	}
	dridata *dfileR = ald_getdata(DRIFILE_WAVE, noR -1);
	if (dfileR == NULL) {
		ald_freedata(dfileL);
		WARNING("DRIFILE_WAVE fail to open %d\n", noR -1);
		return NG;
	}

	EM_ASM_({
		xsystem35.waitPromise(function() {
			return xsystem35.audio.pcm_load_mixlr($0, $1, $2, $3, $4);
		});
	}, slot, dfileL->data, dfileL->size, dfileR->data, dfileR->size);
#ifdef EMTERPRETIFY_ADVISE
	emscripten_sleep(0);
#endif

	ald_freedata(dfileL);
	ald_freedata(dfileR);
	return OK;
}

// PCMデータを再生
int muspcm_start(int slot, int loop) {
	return EM_ASM_INT({
		return xsystem35.audio.pcm_start($0, $1);
	}, slot, loop) ? OK : NG;
}

// PCMデータの再生停止
int muspcm_stop(int slot) {
	return EM_ASM_INT({
		return xsystem35.audio.pcm_stop($0);
	}, slot) ? OK : NG;
}

// 指定時間のフェードアウトの後に再生停止
int muspcm_fadeout(int slot, int msec) {
	return EM_ASM_INT({
		return xsystem35.audio.pcm_fadeout($0, $1);
	}, slot, msec) ? OK : NG;
}

// PCMデータ再生一時停止
int muspcm_pause(int slot) {
	printf("%s not implemented\n", __func__);
	return NG;
}

// PCMデータ再生一時停止解除
int muspcm_unpause(int slot) {
	printf("%s not implemented\n", __func__);
	return NG;
}

// 現在の再生位置を返す
int muspcm_getpos(int slot) {
	return EM_ASM_INT({
		return xsystem35.audio.pcm_getpos($0);
	}, slot) ? OK : NG;
}

// PCMオブジェクトに対してボリュームをセット
int muspcm_setvol(int dev, int slot, int lv) {
	return EM_ASM_INT({
		return xsystem35.audio.pcm_setvol($0, $1);
	}, slot, lv) ? OK : NG;
}

// PCMデータの長さを取得
int muspcm_getwavelen(int slot) {
	int len = EM_ASM_INT({
		return xsystem35.audio.pcm_getwavelen($0);
	}, slot);

	return len > 65535 ? 65535 : len;
}

// 指定のスロットが現在演奏中かどうかを取得
boolean muspcm_isplaying(int slot) {
	return EM_ASM_INT({
		return xsystem35.audio.pcm_isplaying($0);
	}, slot) ? TRUE : FALSE;
}

int musfade_setvolval_all(int val) {
	EM_ASM_(xsystem35.audio.setVolume($0), val);
}
