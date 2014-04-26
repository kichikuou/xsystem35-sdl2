/*
 * cdrom.h  cdrom control
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
/* $Id: cdrom.h,v 1.14 2002/08/18 09:35:29 chikama Exp $ */

#ifndef __CDROM_H__
#define __CDROM_H__

/*
 * CD-ROM へのアクセスが不安定な場合は次の定数を増やしてみて下さい
 */
/* ioctrole retry times */
#define CDROM_IOCTL_RETRY_TIME 3
/* ioctrole retry interval (100ms unit) */
#define CDROM_IOCTL_RETRY_INTERVAL 1

typedef struct {
        int t,m,s,f;
} cd_time;

struct _cdromdevice {
	int  (* init)(char *);
	int  (* exit)(void);
	int  (* start)(int trk);
	int  (* stop)(void);
	int  (* getpos)(cd_time *);
	int  (* setvol)(int);
	int  (* getvol)(void);
};
typedef struct _cdromdevice cdromdevice_t;

extern int  cd_init(cdromdevice_t *);
extern void cd_set_devicename(char *);

#define CD_FPS 75
#define FRAMES_TO_MSF(f, M,S,F) {                                       \
        int value = f;                                                  \
        *(F) = value%CD_FPS;                                            \
        value /= CD_FPS;                                                \
        *(S) = value%60;                                                \
        value /= 60;                                                    \
        *(M) = value;                                                   \
}
#define MSF_TO_FRAMES(M, S, F)  ((M)*60*CD_FPS+(S)*CD_FPS+(F))

#endif /* __CDROM_H__ */
