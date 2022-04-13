/*
 * cdrom.FreeBSD.c  CD-ROMアクセス
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
/* $Id: cdrom.FreeBSD.c,v 1.17 2002/08/18 09:35:29 chikama Exp $ */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/cdio.h>

#include "portab.h"
#include "cdrom.h"
#include "music_private.h"

static int  cdrom_init(char *);
static int  cdrom_exit();
static int  cdrom_start(int, int);
static int  cdrom_stop();
static int  cdrom_getPlayingInfo(cd_time *);

#define cdrom cdrom_bsd
cdromdevice_t cdrom = {
	cdrom_init,
	cdrom_exit,
	cdrom_start,
	cdrom_stop,
	cdrom_getPlayingInfo,
	NULL,
	NULL
};

static int     cd_fd;
static boolean enabled = FALSE;
static struct  cd_toc_entry toc_buffer[100];
static boolean msfmode = TRUE;               /* default は MSFPLAY mode */
static int     lastindex;                    /* 最終トラック */

/* ioctlがエラーで帰って来る場合 IOCTL_RETRY_TIME 回リトライする */
static int do_ioctl(int cmd, void *data) {
	int i;
	for (i = 0; i < CDROM_IOCTL_RETRY_TIME; i++) {
		if (0 <= ioctl(cd_fd, cmd, data)) {
			return 0;
		}
		usleep(CDROM_IOCTL_RETRY_INTERVAL * 100 * 1000);
	}
	return -1;
}

/* CD-ROM の目次を読み出しておく */
static int get_cd_entry() {
	int    endtrk, i;
	struct ioc_toc_header     tochdr;
	struct ioc_read_toc_entry toc;
	struct ioc_play_msf       msf;

	/* 最終トラック番号を得る */
	if (do_ioctl(CDIOREADTOCHEADER, &tochdr) < 0) {
		perror("CDIOREADTOCHEADER");
		return NG;
	}
	
	lastindex = endtrk = tochdr.ending_track;
	i = tochdr.ending_track - tochdr.starting_track + 1;
	if (endtrk <= 1) {  /* ２トラック以上ないとダメ */
		fprintf(stderr, "No CD-AUDIO in CD-ROM\n");
		return NG;
	}
	
	prv.cd_maxtrk = lastindex;
	
	/* すべてのトラックの目次を読みだしてキャッシュしておく */
	toc.address_format = CD_MSF_FORMAT;
	toc.starting_track = 0;
	toc.data_len = (i + 1) * sizeof(struct cd_toc_entry);
	toc.data = toc_buffer;
	if (do_ioctl(CDIOREADTOCENTRYS, &toc) < 0) {
		perror("CDIOREADTOCENTRYS");
		return NG;
	}

#ifdef CDROM_USE_TRKIND_ONLY
	msfmode = FALSE;
	return OK;
#endif

	/* CDROMPLAYMSF が有効かチェック */
	msf.start_m = toc_buffer[1].addr.msf.minute;
	msf.start_s = toc_buffer[1].addr.msf.second;
	msf.start_f = toc_buffer[1].addr.msf.frame;
	msf.end_m   = toc_buffer[2].addr.msf.minute;
	msf.end_s   = toc_buffer[2].addr.msf.second;
	msf.end_f   = toc_buffer[2].addr.msf.frame;
	if (do_ioctl(CDIOCPLAYMSF, &msf) < 0) {
		perror("CDIOPLAYMSF");
		fprintf(stderr, "CD-ROM: change TRKMODE\n");
		msfmode = FALSE;
	}
	/* stop */
	if (do_ioctl(CDIOCSTOP, NULL) < 0) {
		perror("CDIOCSTOP");
		return NG;
	}
	return OK;
}

/* デバイスの初期化 */
int cdrom_init(char *dev_cd) {
	if (dev_cd == NULL) return NG;

	if ((cd_fd = open(dev_cd, O_RDONLY, 0)) < 0) {
		perror("CDROM_DEVICE OPEN");
		enabled = FALSE;
		return NG;
	}
	if (OK == get_cd_entry()) {
		enabled = TRUE;
		return OK;
	}
	enabled = FALSE;
	return NG;
}

/* デバイスの後始末 */
int cdrom_exit() {
	if (enabled) {
		cdrom_stop();
		close(cd_fd);
	}
	return OK;
}

/* トラック番号 trk の演奏 trk = 1~ */
int cdrom_start(int trk, int loop) {
	struct ioc_play_msf   msf;
	struct ioc_play_track track;
	
	if (!enabled) return NG;

	/* 曲数よりも多い指定は不可*/
	if (trk > lastindex) {
		return NG;
	}
	/* drive spin up */
        if (do_ioctl(CDIOCSTART, NULL) < 0) {
		perror("CDIOCSTART");
                return NG;
	}
	if (msfmode) {
		msf.start_m = toc_buffer[trk - 1].addr.msf.minute;
		msf.start_s = toc_buffer[trk - 1].addr.msf.second;
		msf.start_f = toc_buffer[trk - 1].addr.msf.frame;
		msf.end_m = toc_buffer[trk].addr.msf.minute;
		msf.end_s = toc_buffer[trk].addr.msf.second;
		msf.end_f = toc_buffer[trk].addr.msf.frame;
		if (do_ioctl(CDIOCPLAYMSF, &msf) < 0) {
			perror("CDIOPLAYMSF");
			return NG;
		}
	} else {
		track.start_track = track.end_track = trk;
		track.start_index = track.end_index = 0;
		if (do_ioctl(CDIOCPLAYTRACKS, &track) < 0) {
			perror("CDIOCPLAYTRACKS");
			return NG;
		}
	}
	return OK;
}

/* 演奏停止 */
int cdrom_stop() {
	if (enabled) {
		/* if (do_ioctl(CDIOCSTOP, NULL) < 0) { */
		if (do_ioctl(CDIOCPAUSE, NULL) < 0) {
			/* perror("CDIOCSTOP"); */
			perror("CDIOCPAUSE");
		}
		return OK;
	}
	return NG;
}

/* 現在演奏中のトラック情報の取得 */
int cdrom_getPlayingInfo (cd_time *info) {
	struct ioc_read_subchannel s;
	struct cd_sub_channel_info data;
	
	if (!enabled)
		return NG;
	memset(&s, 0, sizeof(s));

        s.data = &data;
        s.data_len = sizeof (data);
        s.address_format = CD_MSF_FORMAT;
        s.data_format = CD_CURRENT_POSITION;
	if (do_ioctl(CDIOCREADSUBCHANNEL, &s) < 0) {
		perror("CDIOCREADSUBCHANNEL");
		return NG;
	}
	if (s.data->header.audio_status != CD_AS_PLAY_IN_PROGRESS) {
		return NG;
	}
	info->t = s.data->what.position.track_number;
	info->m = s.data->what.position.reladdr.msf.minute;
	info->s = s.data->what.position.reladdr.msf.second;
	info->f = s.data->what.position.reladdr.msf.frame;
	return OK;
}
