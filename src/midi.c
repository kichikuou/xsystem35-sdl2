/*
 * midi.c  midi access wrapper
 *
 * Copyright (C) 2000-  Masaki Chikama (Wren) <masaki-c@is.aist-nara.ac.jp>
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
/* $Id: midi.c,v 1.22 2003/01/04 17:01:02 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <string.h>

#include "portab.h"
#include "midi.h"

static char default_mode = 'e';
static int subdev = -1;

#ifdef __EMSCRIPTEN__
extern mididevice_t midi_emscripten;
#endif

#ifdef __ANDROID__
extern mididevice_t midi_android;
#endif

#ifdef ENABLE_MIDI_SDLMIXER
extern mididevice_t midi_sdlmixer;
#endif

#ifdef ENABLE_MIDI_PORTMIDI
extern mididevice_t midi_portmidi;
#endif

bool midi_init(mididevice_t *midi) {
	bool ret = false;

	switch(default_mode) {
	case 'e':
#ifdef __EMSCRIPTEN__
		ret = midi_emscripten.init(0);
		memcpy(midi, &midi_emscripten, sizeof(mididevice_t));
#endif
#ifdef __ANDROID__
		ret = midi_android.init(0);
		memcpy(midi, &midi_android, sizeof(mididevice_t));
#endif
#ifdef ENABLE_MIDI_SDLMIXER
		ret = midi_sdlmixer.init(0);
		memcpy(midi, &midi_sdlmixer, sizeof(mididevice_t));
#endif
		break;
	case 'p':
#ifdef ENABLE_MIDI_PORTMIDI
		ret = midi_portmidi.init(subdev);
		memcpy(midi, &midi_portmidi, sizeof(mididevice_t));
#endif
	case '0':
		break;
	}
	
	return ret;
}

void midi_set_output_device(int mode) {
	switch(mode & 0x7f) {
	case 'e':
		/* external player */
		default_mode = 'e';
		break;
	case 'p':
		/* portmidi midi mode */
		default_mode = 'p';
		subdev = mode >> 8;
		break;
	case '0':
		/* disable midi */
		default_mode = '0';
		break;
	default:
		break;
	}
}
