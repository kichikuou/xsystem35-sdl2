/*
 * cdrom.Linux.c  CD-ROMアクセス
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
/* $Id: cdrom.Linux.c,v 1.18 2002/08/18 09:35:29 chikama Exp $ */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#if defined(sun)
#include <sys/cdio.h>
struct cdrom_msf0 {
	unsigned char   minute;     /* minute */
	unsigned char   second;     /* second */
	unsigned char   frame;      /* frame  */
};
#else
#include <linux/cdrom.h>
#endif /* sun */

#include "portab.h"
#include "cdrom.h"
#include "music_private.h"

static int  cdrom_init(char *);
static int  cdrom_exit();
static int  cdrom_start(int, int);
static int  cdrom_stop();
static int  cdrom_getPlayingInfo(cd_time *);

#define cdrom cdrom_linux
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
static struct  cdrom_msf0 tocmsf[100];
static boolean msfmode = TRUE;        /* default は MSFPLAY mode */
static int     lastindex;             /* 最終トラック */


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
	struct cdrom_tochdr   tochdr;
	struct cdrom_tocentry toc;
	struct cdrom_msf      msf;
	
	/* 最終トラック番号を得る */
	if (do_ioctl(CDROMREADTOCHDR, &tochdr) < 0) {
		perror("CDROMREADTOCHDR");
		return NG;
	}
	lastindex = endtrk = tochdr.cdth_trk1;
	if (endtrk <= 1) { /* ２トラック以上ないとダメ */
		fprintf(stderr, "No CD-AUDIO in CD-ROM\n");
		return NG; 
	}
	
	prv.cd_maxtrk = lastindex;
	
	/* すべてのトラックの目次を読みだしてキャッシュしておく */
	toc.cdte_format = CDROM_MSF;
	for (i = 1; i <= endtrk; i++) {
		toc.cdte_track = i;
		if (do_ioctl(CDROMREADTOCENTRY, &toc) < 0) {
			perror("CDROMREADTOCENTRY");
			return NG;
		}
		tocmsf[i - 1].minute = toc.cdte_addr.msf.minute;
		tocmsf[i - 1].second = toc.cdte_addr.msf.second;
		tocmsf[i - 1].frame  = toc.cdte_addr.msf.frame;
	}
	/* リードアウトを読み出す */
	toc.cdte_track = CDROM_LEADOUT;
	if (do_ioctl(CDROMREADTOCENTRY, &toc) < 0) {
		perror("CDROMREADTOCENTRY");
		return NG;
	}
	
#ifdef PPC
	/* Linux PPC 対策  ;last track -1 frame */
	FRAMES_TO_MSF(MSF_TO_FRAMES(toc.cdte_addr.msf.minute,
				    toc.cdte_addr.msf.second,
				    toc.cdte_addr.msf.frame) - 1,
		      &tocmsf[endtrk].minute,
		      &tocmsf[endtrk].second,
		      &tocmsf[endtrk].frame);
#else
	tocmsf[endtrk].minute = toc.cdte_addr.msf.minute;
	tocmsf[endtrk].second = toc.cdte_addr.msf.second;
	tocmsf[endtrk].frame  = toc.cdte_addr.msf.frame;
#endif
	
#ifdef CDROM_USE_TRKIND_ONLY
	msfmode = FALSE;
	return OK;
#endif

	/* CDROMPLAYMSF が有効かチェック */
	msf.cdmsf_min0   = tocmsf[1].minute;
	msf.cdmsf_sec0   = tocmsf[1].second;
	msf.cdmsf_frame0 = tocmsf[1].frame;
	msf.cdmsf_min1   = tocmsf[2].minute;
	msf.cdmsf_sec1   = tocmsf[2].second;
	msf.cdmsf_frame1 = tocmsf[2].frame;
	if (do_ioctl(CDROMPLAYMSF, &msf) < 0) {
		perror("CDROMPLAYMSF");
		fprintf(stderr, "CD-ROM: change TRKIND mode\n");
		msfmode = FALSE;
	}
	/* stop */
	if (do_ioctl(CDROMSTOP, NULL) < 0) {
		perror("CDROMSTOP");
		return NG;
	}
	return OK;
}

/* デバイスの初期化 */
static int cdrom_init(char *dev_cd) {
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
static int cdrom_exit() {
	if (enabled) {
		cdrom_stop();
		close(cd_fd);
	}
	return OK;
}

/* トラック番号 trk の演奏 trk = 1~ */
static int cdrom_start(int trk, int loop) {
	struct cdrom_msf msf;
	struct cdrom_ti  ti;

	if (!enabled) return NG;

	/* 曲数よりも多い指定は不可*/
	if (trk > lastindex) {
		return NG;
	}
	/* drive spin up */
        if (do_ioctl(CDROMSTART, NULL) < 0) {
		perror("CDROMSTART");
                return NG;
	}
	
	if (msfmode) {
		msf.cdmsf_min0   = tocmsf[trk - 1].minute;
		msf.cdmsf_sec0   = tocmsf[trk - 1].second;
		msf.cdmsf_frame0 = tocmsf[trk - 1].frame;
		msf.cdmsf_min1   = tocmsf[trk].minute;
		msf.cdmsf_sec1   = tocmsf[trk].second;
		msf.cdmsf_frame1 = tocmsf[trk].frame;
		if (do_ioctl(CDROMPLAYMSF, &msf) < 0) {
			perror("CDROMPLAYMSF");
			return NG;
		}
		return OK;
	} else {
		ti.cdti_trk0 = ti.cdti_trk1 = trk;
		ti.cdti_ind0 = ti.cdti_ind1 = 0;
		if (do_ioctl(CDROMPLAYTRKIND, &ti) < 0) {
			perror("CDROMPLAYTRKIND");
			return NG;
		}
		return OK;
	}
}

/* 演奏停止 */
static int cdrom_stop() {
	if (enabled) {
		/* if (do_ioctl(CDROMSTOP, NULL) < 0) { */
		if (do_ioctl(CDROMPAUSE, NULL) < 0) {
			/* perror("CDROMSTOP"); */
			perror("CDROMPAUSE");
			return NG;
		}
		return OK;
	}
	return NG;
}

/* 現在演奏中のトラック情報の取得 */
static int cdrom_getPlayingInfo (cd_time *info) {
	struct cdrom_subchnl sc;
	
	if (!enabled)
		return NG;
	
	sc.cdsc_format = CDROM_MSF;
	if (do_ioctl(CDROMSUBCHNL, &sc) < 0) {
		perror("CDROMSUBCHNL");
		return NG;
	}
	if (sc.cdsc_audiostatus != CDROM_AUDIO_PLAY) {
		return NG;
	}
	info->t = sc.cdsc_trk;
	info->m = sc.cdsc_reladdr.msf.minute;
	info->s = sc.cdsc_reladdr.msf.second;
	info->f = sc.cdsc_reladdr.msf.frame;
	return OK;
}
