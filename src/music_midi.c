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

bool musmidi_init(void) {
	if (!midi_init(&prv.mididev)) {
		prv.midi_valid = false;
		return false;
	} else {
		prv.midi_valid = true;
		return true;
	}
}

void musmidi_exit(void) {
	if (prv.midi_valid) {
		prv.mididev.exit();
	}
}

void musmidi_reset(void) {
	if (prv.midi_valid) {
		prv.mididev.reset();
		prv.midi_current_track = 0;
	}
}

bool musmidi_start(int no, int loop) {
	if (!prv.midi_valid) return false;
	if (prv.midi_current_track == no)
		return true;

	dridata *dfile = ald_getdata(DRIFILE_MIDI, no -1);
	if (dfile == NULL)
		return false;
	
	if (!prv.mididev.start(no, loop, dfile->data, dfile->size)) {
		ald_freedata(dfile);
		return false;
	}

	if (prv.midi_dfile)
		ald_freedata(prv.midi_dfile);
	prv.midi_dfile = dfile;
	prv.midi_current_track = no;

	return true;
}

void musmidi_stop(void) {
	if (!prv.midi_valid) return;

	prv.mididev.stop();

	if (prv.midi_dfile) {
		ald_freedata(prv.midi_dfile);
		prv.midi_dfile = NULL;
	}
	prv.midi_current_track = 0;
}

void musmidi_pause(void) {
	if (!prv.midi_valid) return;
	prv.mididev.pause();
}

void musmidi_unpause(void) {
	if (!prv.midi_valid) return;
	prv.mididev.unpause();
}

midiplaystate musmidi_getpos(void) {
	midiplaystate st = {false, 0, 0};
	if (!prv.midi_valid || !prv.midi_current_track) return st;

	prv.mididev.getpos(&st);
	if (st.in_play)
		st.play_no = prv.midi_current_track;
	else
		prv.midi_current_track = 0;
	return st;
}

bool musmidi_setflag(int mode, int index, int val) {
	if (!prv.midi_valid) return false;

	return prv.mididev.setflag(mode, index, val);
}

int musmidi_getflag(int mode, int index) {
	if (!prv.midi_valid) return 0;

	return prv.mididev.getflag(mode, index);
}

bool musmidi_fadestart(int time, int volume, int stop) {
	if (!prv.midi_valid) return false;

	if (!prv.mididev.fadestart)
		return false;
	return prv.mididev.fadestart(time, volume, stop);
}

bool musmidi_fading(void) {
	if (!prv.midi_valid) return false;

	if (!prv.mididev.fading)
		return false;
	return prv.mididev.fading();
}
