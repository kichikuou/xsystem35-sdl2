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
#include "music_pcm.h"


int muspcm_init() {
	return OK;
}

int muspcm_exit() {
	return OK;
}

// 番号指定のPCMファイル読み込み
int muspcm_load_no(int slot, int no) {
	EM_ASM_({ xsystem35.audio.pcm_load($0, $1); }, slot, no);
#ifdef EMTERPRETIFY_ADVISE
	emscripten_sleep(0);
#endif
	return OK;
}

int muspcm_load_mixlr(int slot, int noL, int noR) {

	EM_ASM_({ xsystem35.audio.pcm_load_mixlr($0, $1, $2); }, slot, noL, noR);
#ifdef EMTERPRETIFY_ADVISE
	emscripten_sleep(0);
#endif
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
	return OK;
}
