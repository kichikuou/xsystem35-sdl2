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

#include <stdio.h>
#include <unistd.h>

#include <sys/time.h>
#include <pthread.h>

#include "system.h"
#include "counter.h"

#include "midi.portmidi.h"

static PortMidiStream *stream;
static struct midiinfo *midi;
static midiflag_t *flags;
static int counter;

static pthread_t thread;
static boolean thread_running = FALSE;
static boolean should_stop    = FALSE;
static boolean should_pause   = FALSE;
static boolean should_unpause = FALSE;
static boolean should_loop    = FALSE;

static int midi_initialize(char *devnm, int subdev) {
	NOTICE("midi_initialize: devnm = [%s] subdev = [%i]\n", devnm, subdev);
	reset_counter_high(SYSTEMCOUNTER_MIDI, 10, 0);
	PmError err;

	if ((err = Pm_Initialize()) != pmNoError) {
		WARNING("%s\n", Pm_GetErrorText(err));
		return NG;
	}

	int ndevices = Pm_CountDevices();
	if (ndevices == 0 || subdev >= ndevices) {
		WARNING("invalid midi device number\n");
		return NG;
	}

	if ((err = Pm_OpenOutput(&stream, subdev, NULL, 0, NULL, NULL, 10)) != pmNoError) {
		WARNING("%s\n", Pm_GetErrorText(err));
		return NG;
	}

	return OK;
}

static int midi_exit() {
	NOTICE("midi_exit\n");
	if (thread_running) {
		midi_stop();
	}
	NOTICE("midi stopped\n");

	Pm_Close(stream);
	stream = NULL;
	Pm_Terminate();
	return OK;
}

static int midi_start(int no, int loop, char *data, int datalen) {
	NOTICE("midi_start: no = %i loop = %i datalen = %i\n", no, loop, datalen);
	should_stop    = FALSE;
	should_pause   = FALSE;
	should_unpause = FALSE;
	should_loop    = loop;

	if (thread_running) {
		if (midi_stop() == NG) {
			WARNING("error stopping midi thread\n");
			return NG;
		}
	}

	midi = mf_read_midifile(data, datalen);
	if (midi == NULL) {
		WARNING("error reading midi file\n");
		return NG;
	}

	int err = pthread_create(&thread, NULL, midi_thread, NULL);
	if (err) {
		WARNING("could not create midi thread\n");
		return NG;
	}

	counter = get_high_counter(SYSTEMCOUNTER_MIDI);
	return OK;
}

static int midi_stop() {
	NOTICE("midi_stop\n");
	should_stop = TRUE;
	pthread_join(thread, NULL);
	return OK;
}

static int midi_pause() {
	NOTICE("midi_pause\n");
	should_pause = TRUE;
	return OK;
}

static int midi_unpause() {
	NOTICE("midi_unpause\n");
	should_unpause = TRUE;
	return OK;
}

static int midi_getpos(midiplaystate *st) {
	NOTICE("midi_getpos\n");
	if (!thread_running) {
		st->in_play = FALSE;
		st->loc_ms = 0;
		return OK;
	}

	int cnt = get_high_counter(SYSTEMCOUNTER_MIDI) - counter;
	st->in_play = TRUE;
	st->loc_ms = cnt * 10;

	return OK;
}

static int midi_getflag(int mode, int idx) {
	NOTICE("midi_getflag: %i = %i\n", mode, idx);
	if (mode == 0) {
		/* flag */
		return flags->midi_flag[idx];
	}
	else {
		/* variable */
		return flags->midi_variable[idx];
	}
}

static int midi_setflag(int mode, int idx, int val) {
	NOTICE("midi_setflag: %i %i = %i\n", mode, idx, val);
	if (mode == 0) {
		/* flag */
		return flags->midi_flag[idx] = val;
	}
	else {
		/* variable */
		flags->midi_variable[idx] = val;
	}

	return OK;
}

static uint64_t pc, tick, ctick, tempo, ppqn;

static void midi_write(midievent_t event) {
	uint64_t ts = (event.ctime * ppqn) / 1000;
	Pm_WriteShort(stream, ts, Pm_Message(event.data[0], event.data[1], event.data[2]));
	pc++;
}

static void midi_allnotesoff() {
	NOTICE("midi_allnotesoff\n");
	for (int i = 0; i < 16; i++) {
		Pm_WriteShort(stream, 0, Pm_Message(0xb0 + i, 0x7b, 0x00));
	}
}

static void midi_reset() {
	NOTICE("midi_reset\n");
	for (int i = 0; i < 16; i++) {
		Pm_WriteShort(stream, 0, Pm_Message(0xb0 + i, 0x78, 0x00));
		Pm_WriteShort(stream, 0, Pm_Message(0xb0 + i, 0x79, 0x00));
	}
}

static void midi_settempo(midievent_t event) {
	int *p;
	p = (int*)event.data;
	tempo = (unsigned int)*p;
	NOTICE("midi_settempo(%i)\n", tempo);
	ppqn = tempo / midi->division;
	pc++;
}

// the timing here feels a little ...off. it could just be that i've
// been listening way too hard and overanalyzing but there are a few
// things that don't feel right. i've tweaked this every way i can think of
// so any new ideas welcome.
static void midi_sync(midievent_t event) {
	int delta = event.ctime - ctick;
	// (delta * ppqn) SHOULD return the clock time in microseconds.
	// the 1000 is subtracted to give portmidi some time to queue the events
	uint64_t stime = (uint64_t) (delta * ppqn) - 1000;

	if (stime > 0) {
		struct timespec req;
		// since stime should be in microseconds, we should be dividing by 10^6
		// but here we are dividing by 10^7 instead. i can't figure out why
		// the numbers are off.
		req.tv_sec  = (time_t)   stime / 10000000;
		req.tv_nsec = (uint64_t)(stime % 10000000) * 1000;

		nanosleep(&req, NULL);
	}

	ctick = event.ctime;
}

static void midi_handlesys35(midievent_t event) {
	NOTICE("midi_handlesys35: ");
	int vn1 = midi->event[pc+1].data[2];
	int vn2 = midi->event[pc+2].data[2];
	int vn3 = midi->event[pc+3].data[2];

	switch (vn1) {
	case 0:
		/* set label */
		NOTICE("set label\n");
		pc += 6;
		break;
	case 1:
		/* jump */
		midi_allnotesoff();
		pc = midi->sys35_label[vn2];
		NOTICE("jump to tick [%i]\n", pc);
		break;
	case 2:
		/* set flag */
		NOTICE("set flag %i = %i\n", vn2, vn3);
		flags->midi_flag[vn2] = vn3;
		pc += 6;
		break;
	case 3:
		/* flag jump */
		if (flags->midi_flag[vn2] == 1) {
			pc = midi->sys35_label[vn3];
			tick = ctick = event.ctime;
			midi_allnotesoff();
			NOTICE("jump to tick [%i] ctime = [%i]\n", pc, tick);
		} else {
			NOTICE("flag [%i] not set, not jumping\n", vn2);
			pc += 6;
		}
		break;
	case 4:
		/* set variable */
		NOTICE("set var %i = %i\n", vn2, vn3);
		flags->midi_variable[vn2] = vn3;
		pc += 6;
		break;
	case 5:
		/* variable jump */
		if (--(flags->midi_variable[vn2]) == 0) {
			pc = midi->sys35_label[vn3];
			tick = ctick = event.ctime;
			midi_allnotesoff();
			NOTICE("jump to tick [%i] ctime = [%i]\n", pc, tick);
		} else {
			NOTICE("var [%i] not set, not jumping\n", vn2);
			pc += 6;
		}
		break;
	default:
		NOTICE("unknown [%i]\n", vn2);
		break;
	}
}

static void *midi_thread(void *args) {
	thread_running = TRUE;
	NOTICE("midi_thread started\n");

 start:
	midi_allnotesoff();
	midi_reset();

	boolean pausing = FALSE;
	int nevents = midi->eventsize;

	pc = tick = ctick = 0;
	tempo = 500000;
	ppqn = tempo / midi->division;

	while (TRUE) {
		if (pc >= nevents) {
			NOTICE("midi_thread finished midi file\n");
			break;
		}

		if (should_stop) {
			NOTICE("midi_thread received stop command\n");
			break;
		}

		if (should_pause) {
			NOTICE("midi_thread pausing\n");
			midi_allnotesoff();
			should_pause = FALSE;
			pausing = TRUE;
		}

		if (should_unpause) {
			NOTICE("midi_thread unpausing\n");
			should_unpause = FALSE;
			pausing = FALSE;
		}

		if (pausing) {
			usleep(50*1000);
			continue;
		}

		while (tick == midi->event[pc].ctime) {
			midievent_t event = midi->event[pc];
			midi_sync(event);

			switch (midi->event[pc].type) {
			case MIDI_EVENT_NORMAL:
				midi_write(event);
				break;
			case MIDI_EVENT_TEMPO:
				midi_settempo(event);
				break;
			case MIDI_EVENT_SYS35:
				midi_handlesys35(event);
				break;
			default:
				NOTICE("unknown type of event [%x]\n", event.type);
				break;
			}
		}
		tick++;
	}

	if (should_loop && !should_stop)
		goto start;

	if (midi != NULL) {
		NOTICE("freeing midi file\n");
		mf_remove_midifile(midi);
		midi = NULL;
	}

	midi_allnotesoff();
	usleep(10000);
	midi_reset();
	usleep(10000);

	should_stop = FALSE;
	thread_running = FALSE;
	NOTICE("midi_thread done.\n");
	pthread_exit(NULL);
}
