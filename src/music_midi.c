/*
 * music_midi.c  music server MIDI part
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
/* $Id: music_midi.c,v 1.2 2002/08/29 12:41:39 chikama Exp $ */

#include <stdio.h>

#include "portab.h"
#include "music_server.h"
#include "music_midi.h"
#include "midi.h"
#include "ald_manager.h"

enum {
	MIDI_START,
	MIDI_STOPCHECK,
	MIDI_LOOPCHECK,
	MIDI_STOP,
	MIDI_PAUSE,
	MIDI_UNPAUSE,
	MIDI_NOP
};

int musmidi_init() {
	int st = midi_init(&prv.mididev);

	if (st == -1) {
		prv.midi_valid = FALSE;
		return NG;
	} else {
		prv.midi_valid = TRUE;
		prv.midi.dev = &prv.mididev;
		prv.midi.st = MIDI_NOP;
		return OK;
	}
}

int musmidi_exit() {
	if (prv.midi_valid) {
		prv.mididev.exit();
	}
	return OK;
}

int musmidi_start(int no, int loop) {
	dridata *dfile;
	
	dfile = ald_getdata(DRIFILE_MIDI, no -1);
	if (dfile == NULL) {
		prv.midi.dfile = NULL;
		return NG;
	}
	
	prv.midi.no = no;
	prv.midi.loop = loop;
	prv.midi.cnt = 0;
	
	prv.midi.st = MIDI_START;
	prv.midi.in_play = FALSE;
	
	prv.midi.dfile = dfile;
	
	return OK;
}

int musmidi_stop() {
	prv.midi.st = MIDI_STOP;
	return OK;
}

int musmidi_pause() {
	prv.midi.st = MIDI_PAUSE;
	return OK;
}

int musmidi_unpause() {
	prv.midi.st = MIDI_UNPAUSE;
	return OK;
}

midiplaystate musmidi_getpos() {
	midiplaystate st;
	prv.midi.dev->getpos(&st);
	return st;
}

int musmidi_setflag(int mode, int index, int val) {
	prv.midi.dev->setflag(mode, index, val);
	return OK;
}

int musmidi_getflag(int mode, int index) {
	return prv.midi.dev->getflag(mode, index);
}

int musmidi_cb() {
	midiobj_t *obj = &prv.midi;
	
	switch(obj->st) {
	case MIDI_START:
		obj->in_play = TRUE;
		obj->dev->start(obj->no, obj->dfile->data, obj->dfile->size);
		obj->st = MIDI_STOPCHECK;
		break;
		
	case MIDI_STOPCHECK: {
		midiplaystate st;
		if (NG == obj->dev->getpos(&st)) {
			obj->st = MIDI_LOOPCHECK;
		}
		break;
	}
	case MIDI_LOOPCHECK:
		obj->cnt++;
		if (obj->loop == 0) {
			// infinity loop
			obj->st = MIDI_START;
			break;
		}
		if (--obj->loop == 0) {
			obj->st = MIDI_STOP;
		} else {
			obj->st = MIDI_START;
		}
		break;
		
	case MIDI_STOP:
		obj->dev->stop();
		obj->in_play = FALSE;
		obj->st = MIDI_NOP;
		if (obj->dfile) {
			ald_freedata(obj->dfile);
			obj->dfile = NULL;
		}
		break;

	case MIDI_PAUSE:
		obj->dev->pause();
		obj->st = MIDI_NOP;
		break;
		
	case MIDI_UNPAUSE:
		obj->dev->unpause();
		obj->st = MIDI_STOPCHECK;
		break;
		
	case MIDI_NOP:
		break;
	}

	return OK;
}
