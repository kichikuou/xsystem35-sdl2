/*
 * cdrom.mp3.c  CD-ROMのかわりにMP3fileだ！
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
/* $Id: cdrom.mp3.c,v 1.24 2003/01/31 12:58:28 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL_mixer.h>

#include "portab.h"
#include "system.h"
#include "counter.h"
#include "cdrom.h"
#include "music_private.h"

/*
   CPUパワーがあまってあまってどうしようもない人へ :-)

  使い方

   1. とりあえず mpg123 などの プレイヤーを用意する。
      esd を使いたい場合は Ver 0.59q 以降を入れよう。
      プレーヤーにはあらかじめパスを通しておく。

   2. % cat ~/game/kichiku.playlist
       mpg123 -quite
       $(HOME)/game/kichiku/mp3/trk02.mp3
       $(HOME)/game/kichiku/mp3/trk03.mp3
       $(HOME)/game/kichiku/mp3/trk04.mp3
       $(HOME)/game/kichiku/mp3/trk05.mp3
       $(HOME)/game/kichiku/mp3/trk06.mp3

       ってなファイルを用意する。( $(HOME)は適当にかえてね )
       １行目はプレーヤーとそのオプション
       ２行目以降はトラック２から順にファイルをならべる
       

   3 configure で --enable-cdrom=mp3 を追加する

   4 実行時オプションに -devcd ~/game/kichiku.playlist のように上で作成したファイルを指定

*/

static int cdrom_init(char *);
static int cdrom_exit();
static int cdrom_start(int, boolean);
static int cdrom_stop();
static int cdrom_getPlayingInfo(cd_time *);

#define cdrom cdrom_mp3
cdromdevice_t cdrom = {
	cdrom_init,
	cdrom_exit,
	cdrom_start,
	cdrom_stop,
	cdrom_getPlayingInfo,
	NULL,
	NULL
};

#define PLAYLIST_MAX 256

static boolean      enabled = FALSE;
static char         *playlist[PLAYLIST_MAX];
static int          lastindex; // 最終トラック番号
static Mix_Music    *mix_music;
static int          trackno; // 現在演奏中のトラック
static int          counter; // 演奏時間測定用カウンタ

static int cdrom_init(char *config_file) {
	FILE *fp;
	char lbuf[256];
	int lcnt = 0;
	char *s;
	
	if (NULL == (fp = fopen(config_file, "rt"))) return NG;
	// Skip the first line
	fgets(lbuf, sizeof(lbuf), fp); lcnt++;
	
	while (TRUE) {
		if (++lcnt >= (PLAYLIST_MAX +2)) {
			break;
		}
		if (!fgets(lbuf, sizeof(lbuf) -1, fp)) {
			if (feof(fp)) {
				break;
			} else {
				perror("fgets()");
				fclose(fp);
				return NG;
			}
		}
		if (NULL == (playlist[lcnt -2] = malloc(strlen(lbuf) + 1))) {
			fclose(fp);
			return NG;
		}
		s = lbuf;
		while (*s != '\n' && *s != 0) s++;
		if (*s == '\n') *s=0;
		strcpy(playlist[lcnt - 2], lbuf);
	}
	lastindex = lcnt -1;
	fclose(fp);
	
	trackno = 0;
	prv.cd_maxtrk = lastindex;
	
	reset_counter_high(SYSTEMCOUNTER_MP3, 10, 0);
	enabled = TRUE;

	NOTICE("cdrom mp3 external player mode\n");
	
	return OK;
}

static int cdrom_exit() {
	if (enabled) {
		cdrom_stop();
	}
	return OK;
}

/* トラック番号 trk の演奏 trk = 1~ */
static int cdrom_start(int trk, boolean loop) {
	if (!enabled) return 0;
	
	/* 曲数よりも多い指定は不可*/
	if (trk > lastindex) {
		return NG;
	}
	
	if (mix_music)
		Mix_FreeMusic(mix_music);

	mix_music = Mix_LoadMUS(playlist[trk -2]);
	if (!mix_music)
		return NG;
	if (Mix_PlayMusic(mix_music, loop ? -1 : 1) != 0) {
		Mix_FreeMusic(mix_music);
		mix_music = NULL;
		return NG;
	}

	trackno = trk;
	counter = get_high_counter(SYSTEMCOUNTER_MP3);
	
	return OK;
}

/* 演奏停止 */
static int cdrom_stop() {
	if (!enabled || !mix_music) {
		return OK;
	}
	Mix_FreeMusic(mix_music);
	mix_music = NULL;
	trackno = 0;
	
	return OK;
}

/* 現在演奏中のトラック情報の取得 */
static int cdrom_getPlayingInfo (cd_time *inf) {
	int cnt;
	
	if (!enabled || !mix_music) {
		goto errout;
	}
	
	if (!Mix_PlayingMusic()) {
		Mix_FreeMusic(mix_music);
		mix_music = NULL;
		goto errout;
	}
	cnt = get_high_counter(SYSTEMCOUNTER_MP3) - counter;
	
	inf->t = trackno;
	inf->m = cnt / (60*100); cnt %= (60*100); 
	inf->s = cnt / 100;      cnt %= 100;
	inf->f = (cnt * CD_FPS) / 100;
	
	return OK;
	
 errout:
	inf->t = inf->m = inf->s = inf->f = 999;
	return NG;
}
