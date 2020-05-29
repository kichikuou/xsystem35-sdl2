/*
 * music.c  Sound module
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
#include "portab.h"

#include <stdio.h>

#include "music.h"
#include "music_private.h"

struct _musprvdat musprv;

int mus_init(int audio_buffer_size) {
	musbgm_init();
	muscd_init();
	musmidi_init();
	prv.pcm_valid = muspcm_init(audio_buffer_size) == OK;
	return OK;
}

int mus_exit() {
	musbgm_exit();
	if (prv.cd_valid) muscd_exit();
	if (prv.midi_valid) musmidi_exit();
	if (prv.pcm_valid) muspcm_exit();
	return OK;
}

/*
 * cdrom の演奏開始 
 *   track: トラック番号 (第一トラックは 1)
 *   loop : 繰り返し回数 (0の場合は無限)
 */
int mus_cdrom_start(int track, int loop) {
	if (!prv.cd_valid) return NG;
	muscd_start(track, loop);
	muscd_cb();
	return OK;
}

/*
 * cdrom の演奏停止
 */
int mus_cdrom_stop() {
	if (!prv.cd_valid) return NG;
	muscd_stop();
	muscd_cb();
	return OK;
}

/*
 * cdrom の演奏状態の取得
 *   info: 演奏時間(track/min/sec/frame)の状態を格納する場所
 *         停止している場合は 999/999/999/999 が返る
 */
int mus_cdrom_get_playposition(cd_time *tm) {
	if (!prv.cd_valid) return NG;
	*tm = muscd_getpos();
	return OK;
}

/*
 * cdrom の最大トラック数の取得
 *   
 */
int mus_cdrom_get_maxtrack() {
	if (!prv.cd_valid) return 0;
	return prv.cd_maxtrk;
}

/*
 * CDROM の有効/無効 フラグの取得
 *   return: FALASE -> 無効
 *           TRUE   -> 有効
 */
boolean mus_cdrom_get_state() {
	return prv.cd_valid;
}

/*
 * midi の演奏開始 
 *   no  : ファイル番号( no >= 1)
 *   loop: 繰り返し回数 (0の場合は無限)
 */
int mus_midi_start(int no, int loop) {
	return musmidi_start(no, loop);
}

/*
 * midi の演奏停止
 */
int mus_midi_stop(void) {
	return musmidi_stop();
}

/*
 * midi の一時停止
 */
int mus_midi_pause(void) {
	return musmidi_pause();
}

/*
 * midi の一時停止解除
 */
int mus_midi_unpause(void) {
	return musmidi_unpause();
}

/*
 * midi の演奏状態の取得
 *  state: 演奏時間や番号の状態を格納する場所
 *         停止している場合は 0 が入る
 */
int mus_midi_get_playposition(midiplaystate *state) {
	midiplaystate st = musmidi_getpos();
	*state = st;
	return OK;
}

/*
 * midi の演奏 flag/variable の状態を設定する
 *   mode : 0 -> flag mode
 *          1 -> variable mode
 *   index: flag/variable 番号
 *   val  : 書き込む値
 */
int mus_midi_set_flag(int mode, int index, int val) {
	return musmidi_setflag(mode, index, val);
}

/*
 * midi の演奏 flag/variable の状態を取得する
 *   mode : 0 -> flag mode
 *          1 -> variable mode
 *   index: flag/variable 番号
 *
 *   return : flag/variable の値
 */
int mus_midi_get_flag(int mode, int index) {
	return musmidi_getflag(mode, index);
}

/*
 * MIDI の有効/無効 フラグの取得
 *   return: FALASE -> 無効
 *           TRUE   -> 有効
 */
boolean mus_midi_get_state() {
	return prv.midi_valid;
}

/*
 * WAV の演奏開始 (command S?)
 *   no  : ファイル番号( no >= 1)
 *   loop: 繰り返し回数 (0の場合は無限)
 */
int mus_pcm_start(int no, int loop) {
	if (!prv.pcm_valid) return NG;
	if (muspcm_load_no(0, no) == NG)
		return NG;
	return muspcm_start(0, loop);
}

/*
 * WAV を左右 mix して演奏
 *   noL : 左用のファイル番号(noL >= 1)
 *   noR : 右用のファイル番号(noR >= 1)
 *   loop: 繰り返し数(0の場合は無限ループ)
 */
int mus_pcm_mix(int noL, int noR, int loop) {
	if (!prv.pcm_valid) return NG;
	if (muspcm_load_mixlr(0, noL, noR) == NG)
		return NG;
	return muspcm_start(0, loop);
}

/*
 * WAV の演奏停止 (command S?)
 *   msec: 止まるまでの時間(msec), 0の場合はすぐに止まる
 */
int mus_pcm_stop(int msec) {
	if (!prv.pcm_valid) return NG;

	muspcm_fadeout(0, msec);
	return OK;
}

/*
 * WAV ファイルをメモリ上に載せる
 *   no  : ファイル番号( no >= 1)
 */
int mus_pcm_load(int no) {
	if (!prv.pcm_valid) return NG;

	return muspcm_load_no(0, no);
}

/*
 * WAV の演奏状態の取得
 *   pos: 演奏時間を格納する場所(msec)
 *        停止している場合は 0 が入る
 *        loopしている場合は合計時間
 */
int mus_pcm_get_playposition(int *pos) {
	if (!prv.pcm_valid) return NG;

	*pos = muspcm_getpos(0);
	return OK;
}

/* pcm (Scommand) related function */
/*
 * 指定のフォーマットで再生可能かどうか調べる
 *   bit : 8 or 16 bit
 *   rate: frequency
 *   ch  : Mono or Stereo
 *   able: 可能かどうかの状態を受け取る場所
 */
int mus_pcm_check_ability(int bit, int rate, int ch, boolean *able) {
	if (!prv.pcm_valid) {
		*able = FALSE;
		return NG;
	}
	*able = TRUE;
	return OK;
}

/*
 * PCM の有効/無効 フラグの取得
 *   return: FALASE -> 無効
 *           TRUE   -> 有効
 */
boolean mus_pcm_get_state() {
	return prv.pcm_valid;
}

/*
 * フェード開始
 *   device: フェードするデバイス(MIX_MAXTER/MIX_PCM/....)
 *   time  : 最終ボリュームまでに達する時間(msec)
 *   volume: 最終ボリューム
 *   stop:   フェード終了時に演奏をストップするかどうか？
 *           0: しない
 *           1: する
 */ 
int mus_mixer_fadeout_start(int device, int time, int volume, int stop) {
	if (device == MIX_MIDI)
		return musmidi_fadestart(time, volume, stop);
	printf("%s(%d, %d, %d, %d) not implemented\n", __func__, device, time, volume, stop);
	return NG;
}

/*
 * 指定のデバイスが現在フェード中かどうかを調べる
 *   device: 指定デバイス
 *
 *   return: TRUE  -> フェード中
 *           FALSE -> フェード中でない
 */
boolean mus_mixer_fadeout_get_state(int device) {
	if (device == MIX_MIDI)
		return musmidi_fading();
	printf("%s(%d) not implemented\n", __func__, device);
	return FALSE;
}

/*
 * 指定のデバイスのフェードを途中で止める
 *   device: 指定デバイス
 */
int mus_mixer_fadeout_stop(int device) {
	printf("%s not implemented\n", __func__);
	return NG;
}

/*
 * 指定のデバイスのミキサーレベルを取得する
 *   device: 指定デバイス
 *
 *   return: ミキサーレベル(0 - 100) (ゲーム内で設定された値)
 */
int mus_mixer_get_level(int device) {
	printf("%s not implemented\n", __func__);
	return 0;
}

/*
 * 指定のチャンネルに wave file をロード
 *   ch : channel (0-127)
 *   num: ファイル番号 (1-65535)
 */
int mus_wav_load(int ch, int num) {
	if (!prv.pcm_valid) return NG;

	if (ch < 0 || ch > 128) return NG;
	return muspcm_load_no(ch + 1, num);
}

/*
 * 指定のチャンネルから wave file を破棄
 *   ch : channel
 */
int mus_wav_unload(int ch) {
	if (!prv.pcm_valid) return NG;

	if (ch < 0 || ch > 128) return NG;
	return muspcm_unload(ch + 1);
}

/*
 * WAV の演奏開始 (wavXXXX)
 *   ch  : 再生するチャンネル (0-127)
           (あらかじめ mus_wav_loadでloadしておく)
 *   loop: 繰り返し回数       (0の場合は無限, それ以外は１回のみ)
 */
int mus_wav_play(int ch, int loop) {
	if (!prv.pcm_valid) return NG;

	if (ch < 0 || ch > 128) return NG;
	return muspcm_start(ch + 1, loop);
}

/*
 * 指定のチャンネルのWAVの演奏停止 (wavXXX)
 *   ch: channel
 */
int mus_wav_stop(int ch) {
	if (!prv.pcm_valid) return NG;

	if (ch < 0 || ch > 128) return NG;
	return muspcm_stop(ch + 1);
}

/*
 * 指定のチャンネルの演奏状態の取得
 *   ch: channel (0-127)
 *   
 *   return: 演奏時間(msec) 65535ms で飽和
 */
int mus_wav_get_playposition(int ch) {
	return muspcm_getpos(ch + 1);
}

/*
 * 指定のチャンネルのWAVのフェード
 *   ch: channel(0-127)
 *   time  : 最終ボリュームまでに達する時間(msec)
 *   volume: 最終ボリューム
 *   stop  : フェード終了時に演奏をストップするかどうか？
 *             0: しない
 *             1: する
 */
int mus_wav_fadeout_start(int ch, int time, int volume, int stop) {
	if (volume == 0 && stop)
		return muspcm_fadeout(ch + 1, time);

	printf("%s not implemented\n", __func__);
	return NG;
}

/*
 * 指定のチャンネルのフェードを途中で止める
 *   ch: channel (0-127)
 */
int mus_wav_fadeout_stop(int ch) {
	printf("%s not implemented\n", __func__);
	return NG;
}

/*
 * 指定のチャンネルが現在フェード中かどうかを調べる
 *   ch: channel
 *
 *   return: TRUE  -> フェード中
 *           FALSE -> フェード中でない
 */
boolean mus_wav_fadeout_get_state(int ch) {
	printf("%s not implemented\n", __func__);
	return FALSE;
}

/*
 * 指定のチャンネルの再生が終了するまで待つ
 *   ch: channel (0-127)
 */
int mus_wav_waitend(int ch) {
	if (!prv.pcm_valid) return NG;

	if (ch < 0 || ch > 128) return NG;
	return muspcm_waitend(ch + 1);
}

/*
 * 指定のチャンネルで時間待ち
 *     再生していないならすぐに戻る。コマンドが発行された瞬間に演奏中で
 *     あれば、演奏が終っても指定時間経過するまで待つ。
 *   ch  : channel (0-127)
 *   time: 待ち時間(msec)
 */
int mus_wav_waittime(int ch, int time) {
	printf("%s not implemented\n", __func__);
	return NG;
}

/*
 * 指定のチャンネルのWAVデータの演奏時間の取得
 *   ch: channel
 *   
 *   return: 時間(msec) 65535ms で飽和
 */
int mus_wav_wavtime(int ch) {
	return muspcm_getwavelen(ch + 1);
}

/*
 * 指定のチャンネルに wave file をLR反転してロード
 *   ch : channel (0-127)
 *   num: ファイル番号 (1-65535)
 */
int mus_wav_load_lrsw(int ch, int num) {
	printf("%s not implemented\n", __func__);
	return NG;
}

int mus_bgm_play(int no, int time, int vol) {
	return musbgm_play(no, time, vol);
}

int mus_bgm_stop(int no, int time) {
	return musbgm_stop(no, time);
}

int mus_bgm_stopall(int time) {
	return musbgm_stopall(time);
}

int mus_bgm_fade(int no, int time, int vol) {
	return musbgm_fade(no, time, vol);
}

int mus_bgm_getpos(int no) {
	return musbgm_getpos(no);
}

int mus_bgm_getlength(int no) {
	return musbgm_getlen(no);
}

int mus_bgm_wait(int no, int timeout) {
	return musbgm_wait(no, timeout);
}

int mus_bgm_waitpos(int no, int index) {
	printf("%s not implemented\n", __func__);
	return NG;
}

int mus_vol_set_valance(int *vols, int num) {
	printf("%s not implemented\n", __func__);
	return NG;
}
