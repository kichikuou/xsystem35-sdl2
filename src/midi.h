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

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	bool in_play; /* now playing ? */
	int loc_ms;      /* playing posion in ms */
	int play_no;     /* current playing no */
} midiplaystate;

typedef struct {
	bool (*init)(int);
	void (*exit)(void);
	void (*reset)(void);
	// Play through the music loop times. If loop == 0, loops forever.
	bool (*start)(int no, int loop, const uint8_t *data, int datalen);
	void (*stop)(void);
	void (*pause)(void);
	void (*unpause)(void);
	bool (*getpos)(midiplaystate *);
	int (*getflag)(int mode, int index);
	bool (*setflag)(int mode, int index, int val);
	bool (*fadestart)(int time, int volume, int stop);
	bool (*fading)(void);
} mididevice_t;

bool midi_init(mididevice_t *);
void midi_set_output_device(int mode);

#endif /* __MIDI_H__ */
