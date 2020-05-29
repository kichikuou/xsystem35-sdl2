/*
 * music_private.h  music private data
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
#ifndef __MUSIC_PRIVATE_H__
#define __MUSIC_PRIVATE_H__

#include "portab.h"
#include "bgm.h"
#include "cdrom.h"
#include "midi.h"
#include "music_cdrom.h"
#include "music_midi.h"
#include "music_pcm.h"

struct _musprvdat {
	boolean cd_valid;
	boolean midi_valid;
	boolean pcm_valid;
	
	cdromdevice_t cddev;
	mididevice_t  mididev;
	
	cdobj_t cdrom;   // cdrom object
	midiobj_t midi;  // midi object
	
	// ゲーム内での volume 設定 (%値)
	int vol_master;
	int vol_pcm;
	int vol_midi;
	int vol_cd;
	int vol_pcm_sub[128 + 1 + 2]; // volval の channel
	int volval[16]; // 各channel 毎の volume valance
	
	// 最大トラック数
	int cd_maxtrk;
};

#define prv musprv
extern struct _musprvdat musprv;

#endif /* __MUSIC_PRIVATE_H__ */
