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

static int midino;

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
	if (!prv.midi_valid) return NG;
	if (midino == no)
		return OK;

	dridata *dfile = ald_getdata(DRIFILE_MIDI, no -1);
	if (dfile == NULL)
		return NG;
	
	prv.midi.dev->start(no, loop, dfile->data, dfile->size);

	if (prv.midi.dfile)
		ald_freedata(prv.midi.dfile);
	prv.midi.dfile = dfile;
	midino = no;

	return OK;
}

int musmidi_stop() {
	if (!prv.midi_valid) return NG;

	prv.midi.dev->stop();

	if (prv.midi.dfile) {
		ald_freedata(prv.midi.dfile);
		prv.midi.dfile = NULL;
	}
	midino = 0;
	return OK;
}

int musmidi_pause() {
	if (!prv.midi_valid) return NG;

	prv.midi.dev->pause();
	return OK;
}

int musmidi_unpause() {
	if (!prv.midi_valid) return NG;

	prv.midi.dev->unpause();
	return OK;
}

midiplaystate musmidi_getpos() {
	midiplaystate st = {FALSE, 0, 0};
	if (!prv.midi_valid || !midino) return st;

	prv.midi.dev->getpos(&st);
	if (st.in_play)
		st.play_no = midino;
	else
		midino = 0;
	return st;
}

int musmidi_setflag(int mode, int index, int val) {
	if (!prv.midi_valid) return NG;

	prv.midi.dev->setflag(mode, index, val);
	return OK;
}

int musmidi_getflag(int mode, int index) {
	if (!prv.midi_valid) return 0;

	return prv.midi.dev->getflag(mode, index);
}

int musmidi_fadestart(int time, int volume, int stop) {
	if (!prv.midi_valid) return NG;

	if (!prv.midi.dev->fadestart)
		return NG;
	return prv.midi.dev->fadestart(time, volume, stop);
}

boolean musmidi_fading() {
	if (!prv.midi_valid) return FALSE;

	if (!prv.midi.dev->fading)
		return FALSE;
	return prv.midi.dev->fading();
}
