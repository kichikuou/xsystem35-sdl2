/*
 * midifile.h  standard midifile parser
 * 
 * Copyright (C) 1997-1998 Masaki Chikama (Wren) <chikama@kasumi.ipl.mech.nagoya-u.ac.jp>
 *               1998-                           <masaki-c@is.aist-nara.ac.jp>
 *
 * Based on midiplay+ by Daisuke NAGANO  <breeze.nagano@nifty.ne.jp>
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
/* $Id: midifile.h,v 1.2 2001/03/22 11:10:13 chikama Exp $ */

#ifndef __MIDIFILE_H__
#define __MIDIFILE_H__

#include "portab.h"

enum midi_event_type {
	MIDI_EVENT_NORMAL,
	MIDI_EVENT_TEMPO,  /* tempo change (msec/midi-qnote) */
	MIDI_EVENT_SYS35
};

#define MIDI_SYS35_EVENT_SIZE 6

enum midi_sys35_event_type {
	MIDI_SYS35_LABEL_DEFINITION = 0,
	MIDI_SYS35_LABEL_JUMP       = 1,
	MIDI_SYS35_FLAG_SET         = 2,
	MIDI_SYS35_FLAG_JUMP        = 3,
	MIDI_SYS35_VARIABLE_SET     = 4,
	MIDI_SYS35_VARIABLE_JUMP    = 5,
};

struct midievent {
	enum midi_event_type type;
	int ctime;  /* steptime */
	int32_t data;
};

struct midiinfo {
	int division; /* division for delta time*/

	struct midievent *event;
	int nr_events;

	int sys35_label[128]; /* system35 jump info */
};

extern struct midiinfo *mf_read_midifile(const uint8_t *stream, size_t len);
extern void mf_remove_midifile(struct midiinfo *m);

#endif /* __MIDIFILE_H__ */
