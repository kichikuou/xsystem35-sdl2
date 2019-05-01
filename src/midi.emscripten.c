/*
 * midi.emscripten.c  midi play for emscripten
 *
 * Copyright (C) 2019 <KichikuouChrome@gmail.com>
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

#include <emscripten.h>

#include "portab.h"
#include "midi.h"

static int midi_initilize(char *pname, int subdev);
static int midi_exit();
static int midi_start(int no, int loop, char *data, int datalen);
static int midi_stop();
static int midi_pause(void);
static int midi_unpause(void);
static int midi_get_playing_info(midiplaystate *st);
static int midi_getflag(int mode, int index);
static int midi_setflag(int mode, int index, int val);
static int midi_setvol(int vol);
static int midi_getvol();
static int midi_fadestart(int time, int volume, int stop);
static boolean midi_fading();

#define midi midi_emscripten
mididevice_t midi = {
	midi_initilize,
	midi_exit,
	midi_start,
	midi_stop,
	midi_pause,
	midi_unpause,
	midi_get_playing_info,
	midi_getflag,
	midi_setflag,
	midi_setvol,
	midi_getvol,
	midi_fadestart,
	midi_fading
};

static int midino;

static int midi_initilize(char *pname, int subdev) {
	return OK;
}

static int midi_exit() {
	midi_stop();
	return OK;
}

static int midi_start(int no, int loop, char *data, int datalen) {
	if (midino == no)
		return OK;

	EM_ASM_ARGS({ xsystem35.midiPlayer.play($0, $1, $2); }, loop, data, datalen);

	midino = no;
	return OK;
}

static int midi_stop() {
	EM_ASM( xsystem35.midiPlayer.stop(); );

	midino = 0;
	return OK;
}

static int midi_pause(void) {
	EM_ASM( xsystem35.midiPlayer.pause(); );
	return OK;
}

static int midi_unpause(void) {
	EM_ASM( xsystem35.midiPlayer.resume(); );
	return OK;
}

static int midi_get_playing_info(midiplaystate *st) {
	if (midino != 0) {
		int pos = EM_ASM_INT_V( return xsystem35.midiPlayer.getPosition(); );
		if (pos >= 0) {
			st->in_play = TRUE;
			st->play_no = midino;
			st->loc_ms = pos;
			return OK;
		}
	}
	st->in_play = FALSE;
	st->play_no = 0;
	st->loc_ms  = 0;
	return OK;
}

static int midi_getflag(int mode, int index) {
	return 0;
}

static int midi_setflag(int mode, int index, int val) {
	return NG;
}

static int midi_setvol(int vol) {
	EM_ASM_ARGS({ xsystem35.midiPlayer.setVolume($0); }, vol);
	return OK;
}

static int midi_getvol() {
	return EM_ASM_INT_V( return xsystem35.midiPlayer.getVolume(); );
}

static int midi_fadestart(int time, int volume, int stop) {
	return EM_ASM_ARGS({ xsystem35.midiPlayer.fadeStart($0, $1, $2); },
					   time, volume, stop);
}

static boolean midi_fading() {
	return EM_ASM_INT_V( return xsystem35.midiPlayer.isFading(); );
}
