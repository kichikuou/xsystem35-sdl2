/*
 * Copyright (C) 2021 bsdf <EMAILBEN145@gmail.com>
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

#ifndef __MIDI_PORTMIDI_H__
#define __MIDI_PORTMIDI_H__

#include <portmidi.h>

#include "midi.h"
#include "midifile.h"

struct _midiflag {
	char midi_variable[128];
	char midi_flag[128];
};

typedef struct _midiflag midiflag_t;
typedef struct midievent midievent_t;

static void *midi_thread(void *);

static int midi_initialize(char *devnm, int subdev);
static int midi_exit();
static int midi_start(int no, int loop, char *data, int datalen);
static int midi_stop();
static int midi_pause();
static int midi_unpause();
static int midi_getpos(midiplaystate *st);
static int midi_getflag(int mode, int idx);
static int midi_setflag(int mode, int idx, int val);

static void midi_allnotesoff();
static void midi_reset();
static void midi_settempo(midievent_t event);
static void midi_sync(midievent_t event);

mididevice_t midi_portmidi = {
	midi_initialize,
	midi_exit,
	midi_start,
	midi_stop,
	midi_pause,
	midi_unpause,
	midi_getpos,
	midi_getflag,
	midi_setflag,
	NULL,
	NULL,
	NULL,
	NULL
};

#endif /* __MIDI_PORTMIDI_H__ */
