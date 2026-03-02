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

static bool midi_initialize(int subdev) {
	return true;
}

EM_JS(void, midi_stop, (void), {
	xsystem35.midiPlayer.stop();
});

static void midi_exit(void) {
	midi_stop();
}

static void midi_reset(void) {
	midi_stop();
}

static bool midi_start(int no, int loop, const uint8_t *data, int datalen) {
	EM_ASM_ARGS({ xsystem35.midiPlayer.play($0, $1, $2); }, loop, data, datalen);
	return true;
}

EM_JS(void, midi_pause, (void), {
	xsystem35.midiPlayer.pause();
});

EM_JS(void, midi_unpause, (void), {
	xsystem35.midiPlayer.resume();
});

static bool midi_get_playing_info(midiplaystate *st) {
	int pos = EM_ASM_INT_V( return xsystem35.midiPlayer.getPosition(); );
	if (pos >= 0) {
		st->in_play = true;
		st->loc_ms = pos;
		return true;
	}
	st->in_play = false;
	st->loc_ms  = 0;
	return true;
}

static int midi_getflag(int mode, int index) {
	return 0;
}

static bool midi_setflag(int mode, int index, int val) {
	return false;
}

EM_JS(bool, midi_fadestart, (int time, int volume, int stop), {
	xsystem35.midiPlayer.fadeStart(time, volume, stop);
	return 1;
});

EM_JS(bool, midi_fading, (void), {
	return xsystem35.midiPlayer.isFading();
});

mididevice_t midi_emscripten = {
	.init = midi_initialize,
	.exit = midi_exit,
	.reset = midi_reset,
	.start = midi_start,
	.stop = midi_stop,
	.pause = midi_pause,
	.unpause = midi_unpause,
	.getpos = midi_get_playing_info,
	.getflag = midi_getflag,
	.setflag = midi_setflag,
	.fadestart = midi_fadestart,
	.fading = midi_fading,
};
