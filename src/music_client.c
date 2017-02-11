/*
 * music_client.c  music client 部
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
/* $Id: music_client.c,v 1.18 2004/10/31 04:18:06 chikama Exp $ */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <glib.h>

#include "portab.h"
#include "music.h"
#include "music_client.h"
#include "counter.h"
#include "pcmlib.h"
#include "nact.h"

/* それぞれの sub system が使用可能かどうか */
static boolean cdrom_available;
static boolean midi_available;
static boolean audio_available;

static int connect_to_server() {
	int fd;
	struct sockaddr_un saddr;

	saddr.sun_family = AF_UNIX;
	strcpy(saddr.sun_path, nact->tmpdir);
	strcat(saddr.sun_path, XSYS35MUSSOCK);
	if (-1 != (fd = socket(AF_UNIX, SOCK_STREAM, 0))) {
                if (-1 != connect(fd, (struct sockaddr *)&saddr, sizeof(saddr))) {
                        return fd;
		} else {
			perror("connect");
		}
	} else {
		perror("socket");
	}
	close(fd);
	
	return -1;
}

static void *cl_read_packet(int fd, ServerPktHeader *pkt_hdr) {
	void *data = NULL;
	
	if (sizeof(ServerPktHeader) == read(fd, pkt_hdr, sizeof (ServerPktHeader))) {
                if (pkt_hdr->data_length) {
                        data = g_malloc0(pkt_hdr->data_length);
                        read(fd, data, pkt_hdr->data_length);
                }
        }
	
        return data;
}

static void cl_read_ack(int fd) {
        void *data;
	ServerPktHeader pkt_hdr;
	
	data = cl_read_packet(fd, &pkt_hdr);
	if (data) {
		g_free(data);
	}
}

static void cl_send_packet(int fd, int command, void * data, int data_length) {
	ClientPktHeader pkt_hdr;
	
	pkt_hdr.version = XSYS35_PROTOCOL_VERSION;
	pkt_hdr.command = command;
	pkt_hdr.data_length = data_length;
	
	write(fd, &pkt_hdr, sizeof(ClientPktHeader));
	
        if (data_length && data) {
                write(fd, data, data_length);
	}
}

static void cl_send_guint32(int cmd, guint32 val) {
	int fd;

	if (-1 == (fd = connect_to_server())) {
		return;
	}
	
	cl_send_packet(fd, cmd, &val, sizeof(guint32));
	cl_read_ack(fd);
	close(fd);
}

static void cl_send_boolean(int cmd, boolean val) {
	int fd;
	
	if (-1 == (fd = connect_to_server())) {
		return;
	}
	
	cl_send_packet(fd, cmd, &val, sizeof(boolean));
	cl_read_ack(fd);
	close(fd);
}

static void cl_send_cmd(int cmd) {
	int fd;
	
	if (-1 == (fd = connect_to_server())) {
		return;
	}
	
	cl_send_packet(fd, cmd, NULL, 0);
	cl_read_ack(fd);
	close(fd);
}

static boolean cl_get_boolean(int cmd) {
        ServerPktHeader pkt_hdr;
	boolean ret = FALSE;
        void *data;
        int fd;
	
        if (-1 == (fd = connect_to_server())) {
                return ret;
	}
	
        cl_send_packet(fd, cmd, NULL, 0);
        data = cl_read_packet(fd, &pkt_hdr);
        if (data) {
                ret = *((boolean *) data);
                g_free(data);
        }
        cl_read_ack(fd);
        close(fd);
	
        return ret;
}

static int cl_get_guint32(int cmd) {
        ServerPktHeader pkt_hdr;
        void *data;
        int fd, ret = 0;
	
        if (-1 == (fd = connect_to_server())) {
                return ret;
	}
	
        cl_send_packet(fd, cmd, NULL, 0);
        data = cl_read_packet(fd, &pkt_hdr);
        if (data) {
                ret = *((int *) data);
                g_free(data);
        }
        cl_read_ack(fd);
        close(fd);
	
        return ret;
}

int musclient_init() {
	ServerPktHeader pkt_hdr;
	int fd;
	void *data;
	int i;
	
	cdrom_available = FALSE;
	midi_available  = FALSE;
	audio_available = FALSE;
	
	usleep(100*1000); // initial wait
	
	for (i = 0; i < 10; i++) { // retry 10 times
		if (-1 != (fd = connect_to_server())) {
			break;
		}
		usleep(1000*1000); // The cdrom retry takes many clock.
	}
	
	cl_send_packet(fd, MUS_GET_VALIDSUBSYSTEM, NULL, 0);
	data = cl_read_packet(fd, &pkt_hdr);
	if (data) {
		cdrom_available = ((ValidSubsystem *)data)->cdrom;
		midi_available  = ((ValidSubsystem *)data)->midi;
		audio_available = ((ValidSubsystem *)data)->pcm;
		g_free(data);
	}
	cl_read_ack(fd);
	close(fd);
	
	return OK;
}

int musclient_exit() {
	cl_send_cmd(MUS_EXIT);
	return OK;
}

/*
 * cdrom の演奏開始 
 *   track: トラック番号 (第一トラックは 1)
 *   loop : 繰り返し回数 (0の場合は無限)
 */
int mus_cdrom_start(int track, int loop) {
	int v[2];
	int fd;
	
	if (!cdrom_available) return NG;
	
        if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
                return NG;
	}
	
	v[0] = track;
	v[1] = loop;
	cl_send_packet(fd, MUS_CDROM_START, v, 2 * sizeof(int));
	cl_read_ack(fd);
	close(fd);
	
	return OK;
}

/*
 * cdrom の演奏停止
 */
int mus_cdrom_stop() {
	if (!cdrom_available) return NG;
	
	cl_send_cmd(MUS_CDROM_STOP);
	return OK;
}

/*
 * cdrom の演奏状態の取得
 *   info: 演奏時間(track/min/sec/frame)の状態を格納する場所
 *         停止している場合は 999/999/999/999 が返る
 */
int mus_cdrom_get_playposition(cd_time *tm) {
	ServerPktHeader pkt_hdr;
	int fd;
	void *data;
	
	if (!cdrom_available) return NG;
	
	if (-1 == (fd = connect_to_server())) {
		tm->t = tm->m = tm->s = tm->f = 999;
		puts("fail to connect");
		return NG;
	}
	
	cl_send_packet(fd, MUS_CDROM_GETPOSITION, NULL, 0);
	data = cl_read_packet(fd, &pkt_hdr);
	if (data) {
		*tm = *((cd_time *)data);
		g_free(data);
	}
	cl_read_ack(fd);
	close(fd);
	return OK;
}

/*
 * cdrom の最大トラック数の取得
 *   
 */
int mus_cdrom_get_maxtrack() {
	int trk;
	
	if (!cdrom_available) return 0;
	
	trk = cl_get_guint32(MUS_CDROM_GETMAXTRACK);
	
	return trk;
}

/*
 * CDROM の有効/無効 フラグの取得
 *   return: FALASE -> 無効
 *           TRUE   -> 有効
 */
boolean mus_cdrom_get_state() {
	return cdrom_available;
}

/*
 * midi の演奏開始 
 *   no  : ファイル番号( no >= 1)
 *   loop: 繰り返し回数 (0の場合は無限)
 */
int mus_midi_start(int no, int loop) {
	int v[2];
	int fd;
	
	if (!midi_available) return NG;
	
        if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
                return NG;
	}

	v[0] = no;
	v[1] = loop;
	cl_send_packet(fd, MUS_MIDI_START, v, 2 * sizeof(int));
	cl_read_ack(fd);
	close(fd);
	return OK;
}

/*
 * midi の演奏停止
 */
int mus_midi_stop(void) {
	if (!midi_available) return NG;

	cl_send_cmd(MUS_MIDI_STOP);
	return OK;
}

/*
 * midi の一時停止
 */
int mus_midi_pause(void) {
	if (!midi_available) return NG;

	cl_send_cmd(MUS_MIDI_PAUSE);
	return OK;
}

/*
 * midi の一時停止解除
 */
int mus_midi_unpause(void) {
	if (!midi_available) return NG;

	cl_send_cmd(MUS_MIDI_UNPAUSE);
	return OK;
}

/*
 * midi の演奏状態の取得
 *  state: 演奏時間や番号の状態を格納する場所
 *         停止している場合は 0 が入る
 */
int mus_midi_get_playposition(midiplaystate *state) {
	ServerPktHeader pkt_hdr;
	int fd;
	void *data;
	
	if (!midi_available) return NG;

	if (-1 == (fd = connect_to_server())) {
		state->loc_ms = state->play_no = 0;
		puts("fail to connect");
		return NG;
	}
	
	cl_send_packet(fd, MUS_MIDI_GETPOSITION, NULL, 0);
	data = cl_read_packet(fd, &pkt_hdr);
	if (data) {
		*state = *(midiplaystate *)data;
		g_free(data);
	}
	cl_read_ack(fd);
	close(fd);
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
	int v[3];
	int fd;
	
	if (!midi_available) return NG;

        if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
                return NG;
	}

	v[0] = mode;
	v[1] = index;
	v[2] = val;
	cl_send_packet(fd, MUS_MIDI_SETFLAG, v, 3 * sizeof(int));
	cl_read_ack(fd);
	close(fd);
	return OK;
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
	ServerPktHeader pkt_hdr;
	int v[2];
	int fd, val = 0;
	void *data;
	
	if (!midi_available) return NG;

        if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
                return NG;
	}

	v[0] = mode;
	v[1] = index;
	cl_send_packet(fd, MUS_MIDI_GETFLAG, v, 2 * sizeof(int));
	data = cl_read_packet(fd, &pkt_hdr);
	if (data) {
		val = *((int *)data);
		g_free(data);
	}
	cl_read_ack(fd);
	close(fd);
	
	return val;
}

/*
 * MIDI の有効/無効 フラグの取得
 *   return: FALASE -> 無効
 *           TRUE   -> 有効
 */
boolean mus_midi_get_state() {
	return midi_available;
}

/*
 * WAV の演奏開始 (command S?)
 *   no  : ファイル番号( no >= 1)
 *   loop: 繰り返し回数 (0の場合は無限)
 */
int mus_pcm_start(int no, int loop) {
	int v[2];
	int fd;
	
	if (!audio_available) return NG;

	if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
		return NG;
	}
	
	/* load file */
	v[0] = 0;
	v[1] = no;
	cl_send_packet(fd, MUS_PCM_LOAD_NO, v, 2 * sizeof(int));
	cl_read_ack(fd);
	close(fd);
	
	if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
		return NG;
	}
	/* play start */
	v[0] = 0;
	v[1] = loop;
	cl_send_packet(fd, MUS_PCM_START, v, 2 * sizeof(int));
	cl_read_ack(fd);
	close(fd);

	return OK;
}

/*
 * WAV を左右 mix して演奏
 *   noL : 左用のファイル番号(noL >= 1)
 *   noR : 右用のファイル番号(noR >= 1)
 *   loop: 繰り返し数(0の場合は無限ループ)
 */
int mus_pcm_mix(int noL, int noR, int loop) {
	int v[2];
	int fd, len;
	WAVFILE *wfile;
	
	if (!audio_available) return NG;

	if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
		return NG;
	}
	
	/* mix 2 wave files */
	wfile = pcmlib_mixlr(noL, noR);
	if (wfile == NULL) {
		puts("mixlr fail");
		return NG;
	}
	
	/* load from mem */
	{
		ClientPktHeader pkt_hdr;
		len = sizeof(WAVFILE) + wfile->bytes;
		
		v[0] = 0;
		pkt_hdr.version = XSYS35_PROTOCOL_VERSION;
		pkt_hdr.command = MUS_PCM_LOAD_MEM;
		pkt_hdr.data_length = sizeof(int) + len;
		
		write(fd, &pkt_hdr, sizeof(ClientPktHeader));
		write(fd, v, sizeof(int));
		write(fd, wfile, sizeof(WAVFILE));
		write(fd, wfile->data, wfile->bytes);
	}
	cl_read_ack(fd);
	close(fd);
	
	if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
		return NG;
	}
	/* play start */
	v[0] = 0;
	v[1] = loop;
	cl_send_packet(fd, MUS_PCM_START, v, 2 * sizeof(int));
	cl_read_ack(fd);
	
	close(fd);
	
	pcmlib_free(wfile);
	
	return OK;
}

/*
 * WAV の演奏停止 (command S?)
 *   msec: 止まるまでの時間(msec), 0の場合はすぐに止まる
 */
int mus_pcm_stop(int msec) {
	int v[2];
	int fd;
	
	if (!audio_available) return NG;

	if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
		return NG;
	}
	
	v[0] = 0;
	v[1] = msec;
	cl_send_packet(fd, MUS_PCM_STOP, v, 2 * sizeof(int));
	cl_read_ack(fd);
	
	close(fd);
	
	return OK;
}

/*
 * WAV ファイルをメモリ上に載せる
 *   no  : ファイル番号( no >= 1)
 */
int mus_pcm_load(int no) {
	int v[2];
	int fd;
	
	if (!audio_available) return NG;

	if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
		return NG;
	}
	
	/* load file */
	v[0] = 0;
	v[1] = no;
	cl_send_packet(fd, MUS_PCM_LOAD_NO, v, 2 * sizeof(int));
	cl_read_ack(fd);

	close(fd);
	
	return OK;
}

/*
 * WAV の演奏状態の取得
 *   pos: 演奏時間を格納する場所(msec)
 *        停止している場合は 0 が入る
 *        loopしている場合は合計時間
 */
int mus_pcm_get_playposition(int *pos) {
	ServerPktHeader pkt_hdr;
	int v[2];
	int fd;
	void *data;
	
	if (!audio_available) return NG;

	if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
		return 0;
	}
	
	v[0] = 0;
	cl_send_packet(fd, MUS_PCM_GETPOSITION, v, sizeof(int));
	
	data = cl_read_packet(fd, &pkt_hdr);
	if (data) {
		*pos = *(int *)(data);
		g_free(data);
	}
	cl_read_ack(fd);
	close(fd);
	
	return OK;
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
	int v[5];
	int fd;
	
	if (!audio_available) return NG;

	if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
		return NG;
	}
	
	v[0] = device;
	v[1] = 0;
	v[2] = time;
	v[3] = volume;
	v[4] = stop;
	cl_send_packet(fd, MUS_FADE_START, v, 5 * sizeof(int));
	cl_read_ack(fd);
	
	close(fd);
	
	return OK;
}

/*
 * 指定のデバイスが現在フェード中かどうかを調べる
 *   device: 指定デバイス
 *
 *   return: TRUE  -> フェード中
 *           FALSE -> フェード中でない
 */
boolean mus_mixer_fadeout_get_state(int device) {
	ServerPktHeader pkt_hdr;
	int v[2];
	int fd;
	boolean bool = FALSE;
	void *data;
	
	if (!audio_available) return FALSE;
	
	if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
		return FALSE;
	}
	
	v[0] = device;
	v[1] = 0;
	cl_send_packet(fd, MUS_FADE_GETSTATE, v, 2 * sizeof(int));
	data = cl_read_packet(fd, &pkt_hdr);
	if (data) {
		bool = *((boolean *)data);
		g_free(data);
	}
	cl_read_ack(fd);
	close(fd);
	
	return bool;
}

/*
 * 指定のデバイスのフェードを途中で止める
 *   device: 指定デバイス
 */
int mus_mixer_fadeout_stop(int device) {
	int v[2];
	int fd;
	
	if (!audio_available) return NG;

	if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
		return NG;
	}
	
	v[0] = device;
	v[1] = 0;
	cl_send_packet(fd, MUS_FADE_STOP, v, 2 * sizeof(int));
	cl_read_ack(fd);
	
	close(fd);
	
	return OK;
}

/*
 * 指定のデバイスのミキサーレベルを取得する
 *   device: 指定デバイス
 *
 *   return: ミキサーレベル(0 - 100) (ゲーム内で設定された値)
 */
int mus_mixer_get_level(int device) {
	ServerPktHeader pkt_hdr;
	int v, lv = 0;
	void *data;
	int fd;
	
	if (!audio_available) return 0;

	if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
		return 0;
	}
	
	v = device;
	cl_send_packet(fd, MUS_MIXER_GETLEVEL, &v, sizeof(int));
	
	data = cl_read_packet(fd, &pkt_hdr);
	if (data) {
		lv = *((int *)data);
		g_free(data);
	}
	cl_read_ack(fd);
	close(fd);
	return lv;
}

/*
 * 指定のフォーマットで再生可能かどうか調べる
 *   bit : 8 or 16 bit
 *   rate: frequency
 *   ch  : Mono or Stereo
 *   able: 可能かどうかの状態を受け取る場所
 */
int mus_pcm_check_ability(int bit, int rate, int ch, boolean *able) {
	if (!audio_available) {
		printf("not available\n");
		*able = FALSE;
		return NG;
	}
	
	*able = TRUE;
	return OK;
}

/*
 * 指定のチャンネルに wave file をロード
 *   ch : channel (0-127)
 *   num: ファイル番号 (1-65535)
 */
int mus_wav_load(int ch, int num) {
	int v[2];
	int fd;
	
	if (!audio_available) return NG;
	
	if (ch < 0 || ch > 128) return NG;
	
	if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
		return NG;
	}
	
	/* load file */
	v[0] = ch + 1;
	v[1] = num;
	cl_send_packet(fd, MUS_PCM_LOAD_NO, v, 2 * sizeof(int));
	cl_read_ack(fd);
	close(fd);
	return OK;
}

/*
 * 指定のチャンネルから wave file を破棄
 *   ch : channel
 */
int mus_wav_unload(int ch) {
	if (!audio_available) return NG;
	
	if (ch < 0 || ch > 128) return NG;
	
	cl_send_guint32(MUS_PCM_UNLOAD, (guint32)(ch + 1));
	return OK;
}

/*
 * WAV の演奏開始 (wavXXXX)
 *   ch  : 再生するチャンネル (0-127)
           (あらかじめ mus_wav_loadでloadしておく)
 *   loop: 繰り返し回数       (0の場合は無限, それ以外は１回のみ)
 */
int mus_wav_play(int ch, int loop) {
	int v[2];
	int fd;
	
	if (!audio_available) return NG;
	
	if (ch < 0 || ch > 128) return NG;
	
	if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
		return NG;
	}
	
	v[0] = ch + 1;
	v[1] = loop;
	cl_send_packet(fd, MUS_PCM_START, v, 2 * sizeof(int));
	cl_read_ack(fd);
	close(fd);
	return OK;
}

/*
 * 指定のチャンネルのWAVの演奏停止 (wavXXX)
 *   ch: channel
 */
int mus_wav_stop(int ch) {
	if (!audio_available) return NG;
	
	if (ch < 0 || ch > 128) return NG;
	
	cl_send_guint32(MUS_PCM_STOP, (guint32)(ch +1));
	return OK;
}

/*
 * 指定のチャンネルの演奏状態の取得
 *   ch: channel (0-127)
 *   
 *   return: 演奏時間(msec) 65535ms で飽和
 */
int mus_wav_get_playposition(int ch) {
	ServerPktHeader pkt_hdr;
	int v[2];
	int fd;
	void *data;
	int ret = 0;
	
	if (!audio_available) return NG;
	
	if (ch < 0 || ch > 128) return NG;
	
	if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
		return 0;
	}
	
	v[0] = ch + 1;
	cl_send_packet(fd, MUS_PCM_GETPOSITION, v, sizeof(int));
	
	data = cl_read_packet(fd, &pkt_hdr);
	if (data) {
		ret = *(int *)(data);
		g_free(data);
	}
	cl_read_ack(fd);
	close(fd);
	
	if (ret > 65565) ret = 65535;
	
	return ret;
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
	int v[5];
	int fd;
	
	if (!audio_available) return NG;
	
	if (ch < 0 || ch > 128) return NG;
	
	if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
		return NG;
	}
	
	v[0] = MIX_PCM;
	v[1] = ch + 1;
	v[2] = time;
	v[3] = volume;
	v[4] = stop;
	cl_send_packet(fd, MUS_FADE_START, v, 5 * sizeof(int));
	cl_read_ack(fd);
	
	close(fd);
	
	return OK;
}

/*
 * 指定のチャンネルのフェードを途中で止める
 *   ch: channel (0-127)
 */
int mus_wav_fadeout_stop(int ch) {
	int v[2];
	int fd;
	
	if (!audio_available) return NG;
	
	if (ch < 0 || ch > 128) return NG;
	
	if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
		return NG;
	}
	
	v[0] = MIX_PCM;
	v[1] = ch + 1;
	cl_send_packet(fd, MUS_FADE_STOP, v, 2 * sizeof(int));
	cl_read_ack(fd);
	
	close(fd);
	
	return OK;
}

/*
 * 指定のチャンネルが現在フェード中かどうかを調べる
 *   ch: channel
 *
 *   return: TRUE  -> フェード中
 *           FALSE -> フェード中でない
 */
boolean mus_wav_fadeout_get_state(int ch) {
	ServerPktHeader pkt_hdr;
	int v[2];
	int fd;
	boolean bool = FALSE;
	void *data;
	
	if (!audio_available) return FALSE;
	
	if (ch < 0 || ch > 128) return FALSE;
	
	if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
		return FALSE;
	}
	
	v[0] = MIX_PCM;
	v[1] = ch + 1;
	cl_send_packet(fd, MUS_FADE_GETSTATE, v, 2 * sizeof(int));
	data = cl_read_packet(fd, &pkt_hdr);
	
	if (data) {
		bool = *((boolean *)data);
		g_free(data);
	}
	cl_read_ack(fd);
	close(fd);
	
	return bool;
}


/*
 * 指定のチャンネルの再生が終了するまで待つ
 *   ch: channel (0-127)
 */
int mus_wav_waitend(int ch) {
	int v[2];
	int fd;
	
	if (!audio_available) return NG;
	
	if (ch < 0 || ch > 128) return NG;
	
	if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
		return NG;
	}

	v[0] = ch + 1;
	cl_send_packet(fd, MUS_PCM_WAITEND, v, sizeof(int));
	cl_read_ack(fd);
	close(fd);
	
	return OK;
}

/*
 * 指定のチャンネルで時間待ち
 *     再生していないならすぐに戻る。コマンドが発行された瞬間に演奏中で
 *     あれば、演奏が終っても指定時間経過するまで待つ。
 *   ch  : channel (0-127)
 *   time: 待ち時間(msec)
 */
int mus_wav_waittime(int ch, int time) {
	ServerPktHeader pkt_hdr;
	int v[1];
	int fd;
	void *data;
	int cnt, ret = 0;
	
	if (!audio_available) return NG;

	if (ch < 0 || ch > 128) return NG;
	
	cnt = get_high_counter(SYSTEMCOUNTER_MSEC);
	
	if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
		return NG;
	}
	
	v[0] = ch + 1;
	cl_send_packet(fd, MUS_PCM_GETPOSITION, v, sizeof(int));
	
	data = cl_read_packet(fd, &pkt_hdr);
	if (data) {
		ret = *(int *)(data);
		g_free(data);
	}
	cl_read_ack(fd);
	close(fd);
	
	if (ret != 0) {
		int cntn = get_high_counter(SYSTEMCOUNTER_MSEC);
		int sleeptime = time - (cntn - cnt);
		if (sleeptime > 0) {
			usleep(sleeptime * 1000);
		}
	}
	return OK;
}

/*
 * PCM の有効/無効 フラグの取得
 *   return: FALASE -> 無効
 *           TRUE   -> 有効
 */
boolean mus_pcm_get_state() {
	return audio_available;
}

/*
 * 指定のチャンネルのWAVデータの演奏時間の取得
 *   ch: channel
 *   
 *   return: 時間(msec) 65535ms で飽和
 */
int mus_wav_wavtime(int ch) {
	ServerPktHeader pkt_hdr;
	int v[2];
	int fd;
	int ret = 0;
	void *data;
	
	if (!audio_available) return 0;
	
	if (ch < 0 || ch > 128) return NG;
	
	if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
		return FALSE;
	}
	
	v[0] = ch + 1;
	cl_send_packet(fd, MUS_PCM_GETWAVETIME, v, sizeof(int));
	data = cl_read_packet(fd, &pkt_hdr);
	
	if (data) {
		ret = *((boolean *)data);
		g_free(data);
	}
	cl_read_ack(fd);
	close(fd);
	
	if (ret > 65565) ret = 65535;
	return ret;

}

/*
 * 指定の channel に WAVFILE をセット
 *   ch:    channel
 *   wfile: WAVFILE
 */
int mus_wav_sendfile(int ch, WAVFILE *wfile) {
	int v[2];
	int fd, len;
	ClientPktHeader pkt_hdr;
	
	if (!audio_available) return NG;
	
	if (ch < 0 || ch > 128) return NG;
	
	if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
		return NG;
	}
	
	len = sizeof(WAVFILE) + wfile->bytes;
	
	v[0] = ch + 1;
	pkt_hdr.version = XSYS35_PROTOCOL_VERSION;
	pkt_hdr.command = MUS_PCM_LOAD_MEM;
	pkt_hdr.data_length = sizeof(int) + len;
	
	write(fd, &pkt_hdr, sizeof(ClientPktHeader));
	write(fd, v, sizeof(int));
	write(fd, wfile, sizeof(WAVFILE));
	write(fd, wfile->data, wfile->bytes);
	
	cl_read_ack(fd);
	close(fd);
	return OK;
}

/*
 * 指定のチャンネルに wave file をLR反転してロード
 *   ch : channel (0-127)
 *   num: ファイル番号 (1-65535)
 */
int mus_wav_load_lrsw(int ch, int num) {
	int v[2];
	int fd;
	
	if (!audio_available) return NG;
	
	if (ch < 0 || ch > 128) return NG;
	
	if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
		return NG;
	}
	
	/* load file */
	v[0] = ch + 1;
	v[1] = num;
	cl_send_packet(fd, MUS_PCM_LOAD_LRSW, v, 2 * sizeof(int));
	cl_read_ack(fd);
	close(fd);
	return OK;
}

int mus_bgm_play(int no, int time, int vol) {
	int v[3];
	int fd;
	
	if (!audio_available) return NG;
	
	if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
		return NG;
	}
	
	v[0] = no;
	v[1] = time;
	v[2] = vol;
	cl_send_packet(fd, MUS_BGM_PLAY, v, 3 * sizeof(int));
	cl_read_ack(fd);
	close(fd);
	return OK;
}

int mus_bgm_stop(int no, int time) {
	int v[2];
	int fd;
	
	if (!audio_available) return NG;
	
	if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
		return NG;
	}
	
	v[0] = no;
	v[1] = time * 10;
	cl_send_packet(fd, MUS_BGM_STOP, v, 2 * sizeof(int));
	cl_read_ack(fd);
	close(fd);
	return OK;
}

int mus_bgm_fade(int no, int time, int vol) {
	int v[3];
	int fd;
	
	if (!audio_available) return NG;
	
	if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
		return NG;
	}
	
	v[0] = no;
	v[1] = time * 10;
	v[2] = vol;
	cl_send_packet(fd, MUS_BGM_FADE, v, 3 * sizeof(int));
	cl_read_ack(fd);
	close(fd);
	return OK;
}

int mus_bgm_getpos(int no) {
	ServerPktHeader pkt_hdr;
	int v[2];
	int fd;
	void *data;
	int ret = 0;
	
	if (!audio_available) return NG;
	
	if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
		return 0;
	}
	
	v[0] = no;
	cl_send_packet(fd, MUS_BGM_GETPOS, v, sizeof(int));
	
	data = cl_read_packet(fd, &pkt_hdr);
	if (data) {
		ret = *(int *)(data);
		g_free(data);
	}
	cl_read_ack(fd);
	close(fd);
	
	if (ret > 65565) ret = 65535;
	
	return ret;
}

int mus_bgm_wait(int no, int timeout) {
	ServerPktHeader pkt_hdr;
	int v[2], ret = 0;
	int fd, curtime, maxtime;
	void *data;

	if (!audio_available) return NG;
	
	curtime = get_high_counter(SYSTEMCOUNTER_MSEC);
	maxtime = curtime + timeout * 10;
	
	while (curtime < maxtime) {
		if (-1 == (fd = connect_to_server())) {
			puts("fail to connect");
			break;
		}
		v[0] = no;
		cl_send_packet(fd, MUS_BGM_GETPOS, v, sizeof(int));
		data = cl_read_packet(fd, &pkt_hdr);
		if (data) {
			ret = *(int *)(data);
			g_free(data);
		}
		cl_read_ack(fd);
		close(fd);
		if (ret == 0) break;
		usleep(10*1000);
		curtime = get_high_counter(SYSTEMCOUNTER_MSEC);
	}
	
	return OK;
}

int mus_bgm_waitpos(int no, int index) {
	int v[2];
	int fd;
	
	if (!audio_available) return NG;
	
	if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
		return NG;
	}

	v[0] = no;
	v[1] = index;
	cl_send_packet(fd, MUS_BGM_WAITPOS, v, 2 * sizeof(int));
	cl_read_ack(fd);
	close(fd);
	
	return OK;
}

int mus_bgm_stopall(int time) {
	if (!audio_available) return NG;
	
	cl_send_guint32(MUS_BGM_STOPALL, (guint32)time * 10);
	return OK;
}

int mus_bgm_getlength(int no) {
	ServerPktHeader pkt_hdr;
	int v[2];
	int fd;
	void *data;
	int ret = 0;
	
	if (!audio_available) return NG;
	
	if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
		return 0;
	}
	
	v[0] = no;
	cl_send_packet(fd, MUS_BGM_GETLEN, v, sizeof(int));
	
	data = cl_read_packet(fd, &pkt_hdr);
	if (data) {
		ret = *(int *)(data);
		g_free(data);
	}
	cl_read_ack(fd);
	close(fd);
	
	if (ret > 65565) ret = 65535;
	
	return ret;
}

int mus_vol_set_valance(int *vols, int num) {
	int fd;
	
	if (!audio_available) return NG;
	
	if (-1 == (fd = connect_to_server())) {
		puts("fail to connect");
		return NG;
	}
	
	cl_send_packet(fd, MUS_MIXER_SETVOLVAL, vols, num * sizeof(int));
	cl_read_ack(fd);
	close(fd);
	return OK;
}
