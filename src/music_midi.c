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
#include "music_private.h"
#include "music_midi.h"
#include "midi.h"
#include "ald_manager.h"

int musmidi_init() {
	int st = midi_init(&prv.mididev);

	if (st == -1) {
		prv.midi_valid = FALSE;
		return NG;
	} else {
		prv.midi_valid = TRUE;
		prv.midi.dev = &prv.mididev;
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
	
	prv.midi.dfile = dfile;
	prv.midi.dev->start(no, loop, prv.midi.dfile->data, prv.midi.dfile->size);

	return OK;
}

int musmidi_stop() {
	prv.midi.dev->stop();

	if (prv.midi.dfile) {
		ald_freedata(prv.midi.dfile);
		prv.midi.dfile = NULL;
	}
	return OK;
}

int musmidi_pause() {
	prv.midi.dev->pause();
	return OK;
}

int musmidi_unpause() {
	prv.midi.dev->unpause();
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
