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
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#else  /* for FreeBSD 3.x ? */
#include "xpg_basename.c"
extern char *__xpg_basename __P ((char *__path));
#define basename(path)  __xpg_basename (path)
#endif

#include "portab.h"
#include "system.h"
#include "counter.h"
#include "cdrom.h"
#include "music_server.h"
#include "music_pcm.h"

extern void sys_set_signalhandler(int SIG, void (*handler)(int));

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

       １行目の最初の文字が - で始まっている場合 xsystem35 の audio device に
       流し込む (piped play mode) (ex. -mpg123 -quite)
       player の再生フォーマットは 44kHz,16bit,Stereoのみ
       

   3 configure で --enable-cdrom=mp3 を追加する

   4 実行時オプションに -devcd ~/game/kichiku.playlist のように上で作成したファイルを指定

*/

static int cdrom_init(char *);
static int cdrom_exit();
static int cdrom_start(int);
static int cdrom_stop();
static int cdrom_getPlayingInfo(cd_time *);
static int cdrom_setVolumePipe(int vol);
static int cdrom_getVolumePipe();

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
#define WHITE " \t\n"
#define MAX_ARGS 512

static boolean      enabled = FALSE;
static char         mp3_player[256];
static int          argc;
static char         **argv;
static char         *playlist[PLAYLIST_MAX];
static int          lastindex; // 最終トラック番号
static pid_t        cdpid;   // 外部プレーヤーの pid
static int          trackno; // 現在演奏中のトラック
static int          counter; // 演奏時間測定用カウンタ
static boolean      pipedplay; // pipe play モードかどうか

/*
  外部プレーヤーの行の解析
  1. 最初の文字が - であった場合、piped play mode
  2. プログラムと引数の分離 (!pipe)
  3. プログラムからプログラム名(argv[0])を分離 (!piped)
*/

static void player_set(char *buf) {
	char *b;
	int i, j;
	
	if (buf[0] == '-') {
		pipedplay = TRUE;
		buf++;
	} else {
		pipedplay = FALSE;
	}
	
	strncpy(mp3_player, buf, sizeof(mp3_player));
	b = mp3_player;
	
	if (!pipedplay) {
		/* count arguments */
		/* devide argument */
		char *tok_buf = NULL;
		char *str_buf[MAX_ARGS]; //MAX_ARGS以上の引数があるとだめ
		
		memset(str_buf, 0, sizeof(char *) * MAX_ARGS);
		
		i = 0;
		str_buf[i] = strtok_r(b, WHITE, &tok_buf) ;
		
		if (str_buf[i] == NULL) return;
		
		while (str_buf[i] != NULL){
			i++;
			if (i >= MAX_ARGS){
				return;
			}
			str_buf[i] = strtok_r(NULL, WHITE, &tok_buf);
		}
		argv = (char **)malloc(sizeof(char *) * (i + 2));
		
		if (argv == NULL) return;
		
		argc = i;
		
		for (j = 0; j < i; j++) {
			argv[j] = str_buf[j];
		}
		argv[i + 1] = NULL;
		
		/* cut down argv[0] */
		argv[0] = basename(argv[0]);
	} else {
		/*
		  pipe 出力するためにファイル名の後に - をつける必要の
		  あるプレーやのために、ファイル名を %s で指定できるように
		  するための処理 (by Fumihiko Murata)
		*/
		int pf = FALSE;
		
		b = mp3_player;
		while (*b > 0x1f) {
			if (*b == '%' && *(b + 1) == 's') pf = TRUE;
			b++;
		}
		*b = 0;
		if (!pf) strcat(b, " \"%s\""); // space を含むパスの場合
	}
}

static int cdrom_init(char *config_file) {
	FILE *fp;
	char lbuf[256];
	int lcnt = 0;
	char *s;
	
	if (NULL == (fp = fopen(config_file, "rt"))) return NG;
	fgets(lbuf, sizeof(lbuf), fp); lcnt++;
	
	player_set(lbuf);
	
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

	if (pipedplay) {
		cdrom_mp3.setvol = cdrom_setVolumePipe;
		cdrom_mp3.getvol = cdrom_getVolumePipe;
		NOTICE("cdrom mp3 piped play mode\n");
	} else {
		NOTICE("cdrom mp3 external player mode\n");
	}
	
	return OK;
}

static int cdrom_exit() {
	if (enabled) {
		cdrom_stop();
	}
	return OK;
}

/* トラック番号 trk の演奏 trk = 1~ */
static int cdrom_start(int trk) {
	char cmd_pipe[256];
	pid_t pid;
	
	if (!enabled) return 0;
	
	/* 曲数よりも多い指定は不可*/
	if (trk > lastindex) {
		return NG;
	}
	
	if (pipedplay) {
		snprintf(cmd_pipe, sizeof(cmd_pipe) -1, mp3_player, playlist[trk -2]);
		if (-1 == muspcm_load_pipe(SLOT_CDROMPIPE, cmd_pipe)) {
			return NG;
		}
		muspcm_start(SLOT_CDROMPIPE, 1);
		pid = 1; // dummy
	} else {
		argv[argc] = playlist[trk -2];
		argv[argc +1] = NULL;
		pid = fork();
		if (pid == 0) {
			/* child process */
			pid_t mine = getpid();
			setpgid(mine, mine);
			sys_set_signalhandler(SIGTERM, SIG_DFL);
			execvp(mp3_player, argv);
			perror("execvp");
			_exit(-1);
		} else if (pid < 0) {
			WARNING("fork failed");
			return NG;
		}
	}
	
	cdpid = pid;
	trackno = trk;
	counter = get_high_counter(SYSTEMCOUNTER_MP3);
	
	return OK;
}

/* 演奏停止 */
static int cdrom_stop() {
	if (!enabled || cdpid == 0) {
		return OK;
	}
	
	if (!pipedplay) {
		int status = 0;
		kill(cdpid, SIGTERM);
		killpg(cdpid, SIGTERM);
		while (0 >= waitpid(cdpid, &status, WNOHANG));
	} else {
		muspcm_stop(SLOT_CDROMPIPE);
	}
	
	cdpid   = 0;
	trackno = 0;
	
	return OK;
}

/* 現在演奏中のトラック情報の取得 */
static int cdrom_getPlayingInfo (cd_time *inf) {
	int status, cnt, err;
	
	if (!enabled || cdpid == 0) {
		goto errout;
	}
	
	if (pipedplay) {
		cnt = muspcm_getpos(SLOT_CDROMPIPE);
		if (cnt == 0) {
			goto errout;
		}
		cnt /= 10;
	} else {
		if (cdpid == (err = waitpid(cdpid, &status, WNOHANG))) {
			cdpid = 0;
			goto errout;
		}
		cnt = get_high_counter(SYSTEMCOUNTER_MP3) - counter;
	}
	
	inf->t = trackno;
	inf->m = cnt / (60*100); cnt %= (60*100); 
	inf->s = cnt / 100;      cnt %= 100;
	inf->f = (cnt * CD_FPS) / 100;
	
	return OK;
	
 errout:
	inf->t = inf->m = inf->s = inf->f = 999;
	return NG;
}

static int cdrom_setVolumePipe(int vol) {
	if (prv.pcm[SLOT_CDROMPIPE] != NULL) {
		prv.pcm[SLOT_CDROMPIPE]->vollv = vol;
	}
	return OK;
}

static int cdrom_getVolumePipe() {
	if (prv.pcm[SLOT_CDROMPIPE] != NULL) {
		return prv.pcm[SLOT_CDROMPIPE]->vollv;
	}
	return 100;
}
