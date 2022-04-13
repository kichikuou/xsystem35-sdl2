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
extern int midi_pause(void);
extern int midi_unpause(void);
static int midi_get_playing_info(midiplaystate *st);
static int midi_getflag(int mode, int index);
static int midi_setflag(int mode, int index, int val);
extern int midi_setvol(int vol);
extern int midi_getvol();
extern int midi_fadestart(int time, int volume, int stop);
extern boolean midi_fading();

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

static int midi_initilize(char *pname, int subdev) {
	return OK;
}

static int midi_exit() {
	midi_stop();
	return OK;
}

static int midi_start(int no, int loop, char *data, int datalen) {
	EM_ASM_ARGS({ xsystem35.midiPlayer.play($0, $1, $2); }, loop, data, datalen);
	return OK;
}

static int midi_stop() {
	EM_ASM( xsystem35.midiPlayer.stop(); );
	return OK;
}

EM_JS(int, midi_pause, (void), {
	xsystem35.midiPlayer.pause();
	return xsystem35.Status.OK;
});

EM_JS(int, midi_unpause, (void), {
	xsystem35.midiPlayer.resume();
	return xsystem35.Status.OK;
});

static int midi_get_playing_info(midiplaystate *st) {
	int pos = EM_ASM_INT_V( return xsystem35.midiPlayer.getPosition(); );
	if (pos >= 0) {
		st->in_play = TRUE;
		st->loc_ms = pos;
		return OK;
	}
	st->in_play = FALSE;
	st->loc_ms  = 0;
	return OK;
}

static int midi_getflag(int mode, int index) {
	return 0;
}

static int midi_setflag(int mode, int index, int val) {
	return NG;
}

EM_JS(int, midi_setvol, (int vol), {
	xsystem35.midiPlayer.setVolume(vol);
	return xsystem35.Status.OK;
});

EM_JS(int, midi_getvol,(), {
	return xsystem35.midiPlayer.getVolume();
});

EM_JS(int, midi_fadestart, (int time, int volume, int stop), {
	return xsystem35.midiPlayer.fadeStart(time, volume, stop);
});

EM_JS(boolean, midi_fading, (), {
	return xsystem35.midiPlayer.isFading();
});
