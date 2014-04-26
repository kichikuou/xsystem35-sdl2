/*
 * midi.h  midi wrapper
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
/* $Id: midi.h,v 1.6 2002/08/18 09:35:29 chikama Exp $ */

#ifndef __MIDI_H__
#define __MIDI_H__

#include "portab.h"

typedef struct {
	boolean in_play; /* now playing ? */
	int loc_ms;      /* playing posion in ms */
	int play_no;     /* current playing no */
} midiplaystate;

typedef struct mididevice mididevice_t;
struct mididevice {
	int  (* init)(char *, int);
	int  (* exit)(void);
	int  (* start)(int, char*, int);
	int  (* stop)(void);
	int  (* pause)(void);
	int  (* unpause)(void);
	int  (* getpos)(midiplaystate *);
	int  (* getflag)(int mode, int index);
	int  (* setflag)(int mode, int index, int val);
	int  (* setvol)(int);
	int  (* getvol)(int);
};

extern int  midi_init(mididevice_t *);
extern void midi_set_playername(char *);
extern void midi_set_devicename(char *);
extern void midi_set_output_device(int mode);

#endif /* __MIDI_H__ */
