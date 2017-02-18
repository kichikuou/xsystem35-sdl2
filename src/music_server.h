/*
 * music_server.h  music server main
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
/* $Id: music_server.h,v 1.3 2003/04/25 17:23:55 chikama Exp $ */

#ifndef __MUSIC_SERVER_H__
#define __MUSIC_SERVER_H__

#include <sys/poll.h>

#include "portab.h"
#include "list.h"
#include "cdrom.h"
#include "midi.h"
#include "audio.h"
#include "music_cdrom.h"
#include "music_midi.h"
#include "music_pcm.h"

struct _musprvdat {
	boolean cd_valid;
	boolean midi_valid;
	boolean pcm_valid;
	
	cdromdevice_t cddev;
	mididevice_t  mididev;
	audiodevice_t audiodev;
	
	chanfmt_t ofmt; /* audio output format */
	
	cdobj_t cdrom;   // cdrom object
	midiobj_t midi;  // midi object
	
	// pcm object
	/*
	  0:     S comman 用 
	  1-128: wavXXX 用
	  129:   cdrom pipe 用
	  130:   midi pipe 用
	*/
	pcmobj_t *pcm[128 + 1 + 2];
	
	// ゲーム内での volume 設定 (%値)
	int vol_master;
	int vol_pcm;
	int vol_midi;
	int vol_cd;
	int vol_pcm_sub[128 + 1 + 2]; // volval の channel
	int volval[16]; // 各channel 毎の volume valance
	
	// 最大トラック数
	int cd_maxtrk;
	
	List *pcmplist; // PCM多重再生用リスト
	List *fadelist; // Faderリスト
	
	// for polling
	int nfds;
	struct pollfd ufds[2];

};

extern int musserv_init();
extern int musserv_exit();
extern void musserv_send_ack(int);

#define prv musprv
extern struct _musprvdat musprv;

#endif /* __MUSIC_SERVER_H__ */
