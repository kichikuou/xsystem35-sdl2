/*
 * Copyright (C) 1997-1998 Masaki Chikama (Wren) <chikama@kasumi.ipl.mech.nagoya-u.ac.jp>
 *               1998-                           <masaki-c@is.aist-nara.ac.jp>
 *               2021 bsdf <EMAILBEN145@gmail.com>
 *               2024 kichikuou <KichikuouChrome@gmail.com>
 *
 * Based on midiplay+ by Daisuke NAGANO  <breeze.nagano@nifty.ne.jp>
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

#include "config.h"

#include <assert.h>
#include <portmidi.h>
#include <SDL_thread.h>
#include <SDL_timer.h>

#include "portab.h"
#include "system.h"
#include "midi.h"
#include "midifile.h"
#include "msgqueue.h"

#define MIDI_LATENCY 20

static struct {
	char midi_variable[128];
	char midi_flag[128];
} flags;

static int midi_initialize(int subdev);
static int midi_exit(void);
static int midi_reset(void);
static int midi_start(int no, int loop, char *data, int datalen);
static int midi_stop(void);
static int midi_pause(void);
static int midi_unpause(void);
static int midi_getpos(midiplaystate *st);
static int midi_getflag(int mode, int index);
static int midi_setflag(int mode, int index, int val);

mididevice_t midi_portmidi = {
	midi_initialize,
	midi_exit,
	midi_reset,
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

static boolean enabled = FALSE;
static PortMidiStream *stream;
static PmDeviceID device_id;
static SDL_Thread *thread;
static struct msgq *queue;
static int start_time;

static char cmd_pause[] = "pause";
static char cmd_unpause[] = "unpause";
static char cmd_stop[] = "stop";

static void allnotesoff() {
	for (int i = 0; i < 16; i++) {
		Pm_WriteShort(stream, 0, Pm_Message(0xb0 + i, 0x7b, 0x00));
	}
}

static void send_reset() {
	for (int i = 0; i < 16; i++) {
		Pm_WriteShort(stream, 0, Pm_Message(0xb0 + i, 0x78, 0x00));
		Pm_WriteShort(stream, 0, Pm_Message(0xb0 + i, 0x79, 0x00));
	}
	return;
}

static void midi_write(struct midievent *event, uint32_t timestamp) {
	Pm_WriteShort(stream, timestamp, event->data);
}

static PmTimestamp midi_time_proc(void *time_info) {
	return SDL_GetTicks();
}

static uint32_t ticks2ms(int ticks, int division, unsigned int tempo) {
	return ticks * tempo / (division * 1000);
}

static void *midi_mainloop(struct midiinfo *midi) {
	int i = 0;
	int ctick = 0;
	int tempo = 500000;
	uint32_t last_time = SDL_GetTicks();

	void *cmd;
	while (i < midi->nr_events) {
		if (ctick < midi->event[i].ctime) {
			int delta = midi->event[i].ctime - ctick;
			uint32_t target_time = last_time + ticks2ms(delta, midi->division, tempo);
			uint32_t current_time = SDL_GetTicks();
			while (current_time < target_time) {
				cmd = msgq_dequeue_timeout(queue, target_time - current_time);
				if (cmd == cmd_stop)
					return cmd;

				if (cmd == cmd_pause) {
					while (TRUE) {
						cmd = msgq_dequeue(queue);
						if (cmd == cmd_stop)
							return cmd;
						if (cmd == cmd_unpause)
							break;
					}
					target_time += SDL_GetTicks() - current_time;
				}
				current_time = SDL_GetTicks();
			}
			last_time = target_time;
			ctick = midi->event[i].ctime;
		}

		if (midi->event[i].type == MIDI_EVENT_NORMAL) {
			midi_write(&midi->event[i], last_time);
			i++;
		} else if (midi->event[i].type == MIDI_EVENT_TEMPO) {
			tempo = midi->event[i].data;
			i++;
		} else if (midi->event[i].type == MIDI_EVENT_SYS35) {
			int vn1 = Pm_MessageData2(midi->event[i+1].data);
			int vn2 = Pm_MessageData2(midi->event[i+2].data);
			int vn3 = Pm_MessageData2(midi->event[i+3].data);

			switch (vn1) {
			case MIDI_SYS35_LABEL_DEFINITION:
				i += MIDI_SYS35_EVENT_SIZE;
				break;
			case MIDI_SYS35_LABEL_JUMP:
				allnotesoff();
				i = midi->sys35_label[vn2];
				ctick = midi->event[i].ctime;
				break;
			case MIDI_SYS35_FLAG_SET:
				flags.midi_flag[vn2] = vn3;
				i += MIDI_SYS35_EVENT_SIZE;
				break;
			case MIDI_SYS35_FLAG_JUMP:
				if (flags.midi_flag[vn2] == 1) {
					i = midi->sys35_label[vn3];
					ctick = midi->event[i].ctime;
					allnotesoff();
				} else {
					i += MIDI_SYS35_EVENT_SIZE;
				}
				break;
			case MIDI_SYS35_VARIABLE_SET:
				flags.midi_variable[vn2] = vn3;
				i += MIDI_SYS35_EVENT_SIZE;
				break;
			case MIDI_SYS35_VARIABLE_JUMP:
				if (--(flags.midi_variable[vn2]) == 0) {
					i = midi->sys35_label[vn3];
					ctick = midi->event[i].ctime;
					allnotesoff();
				} else {
					i += MIDI_SYS35_EVENT_SIZE;
				}
				break;
			}
		} else {
			WARNING("Unknown type of event %x (NEVER!).", midi->event[i].type);
		}
	}
	return NULL;
}

static int midi_initialize(int subdev) {
	enabled = FALSE;

	PmError err = Pm_Initialize();
	if (err != pmNoError) {
		WARNING("%s", Pm_GetErrorText(err));
		return NG;
	}

	int ndevices = Pm_CountDevices();
	for (int i = 0; i < ndevices; i++) {
		const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
		NOTICE("MIDI #%d: interf=\"%s\", name=\"%s\", input=%d, output=%d",
		       i, info->interf, info->name, info->input, info->output);
	}
	if (subdev < 0 || subdev >= ndevices) {
		WARNING("invalid midi device number");
		return NG;
	}

	device_id = subdev;
	enabled = TRUE;
	return OK;
}

static int midi_exit(void) {
	if (enabled) {
		midi_stop();
	}
	Pm_Terminate();
	return OK;
}

static int midi_reset(void) {
	if (enabled) {
		midi_stop();
	}
	return OK;
}

static int midi_thread(void *data) {
	send_reset();

	void *cmd = midi_mainloop(data);

	send_reset();
	SDL_Delay(MIDI_LATENCY);

	mf_remove_midifile(data);
	Pm_Close(stream);
	stream = NULL;

	while (cmd != cmd_stop)
		cmd = msgq_dequeue(queue);
	return 0;
}

/* no = 0~ */
static int midi_start(int no, int loop, char *data, int datalen) {
	if (queue)
		midi_stop();

	struct midiinfo *midi = mf_read_midifile(data, datalen);
	if (!midi) {
		WARNING("error reading midi file");
		return NG;
	}

	PmError err = Pm_OpenOutput(&stream, device_id, NULL, 0, midi_time_proc, NULL, MIDI_LATENCY);
	if (err != pmNoError) {
		WARNING("Pm_OpenOutput failed: %s", Pm_GetErrorText(err));
		mf_remove_midifile(midi);
		return NG;
	}

	queue = msgq_new();
	thread = SDL_CreateThread(midi_thread, "MIDI", midi);

	start_time = SDL_GetTicks();

	return OK;
}

static int midi_stop(void) {
	if (!enabled || !queue) {
		return OK;
	}

	msgq_enqueue(queue, cmd_stop);
	SDL_WaitThread(thread, NULL);
	thread = NULL;
	msgq_free(queue);
	queue = NULL;

	return OK;
}

static int midi_pause(void) {
	if (!enabled || !queue) return OK;

	msgq_enqueue(queue, cmd_pause);
	return OK;
}

static int midi_unpause(void) {
	if (!enabled || !queue) return OK;

	msgq_enqueue(queue, cmd_unpause);
	return OK;
}

static int midi_getpos(midiplaystate *st) {
	if (!enabled || !queue) {
		st->in_play = FALSE;
		st->loc_ms  = 0;
		return OK;
	}

	if (!stream) {
		midi_stop();
		st->in_play = FALSE;
		st->loc_ms  = 0;
		return OK;
	}

	st->in_play = TRUE;
	st->loc_ms = SDL_GetTicks() - start_time;

	return OK;
}

static int midi_getflag(int mode, int index) {
	if (mode == 0) {
		/* flag */
		return flags.midi_flag[index];
	} else {
		/* variable */
		return flags.midi_variable[index];
	}
}

static int midi_setflag(int mode, int index, int val) {
	if (mode == 0) {
		/* flag */
		flags.midi_flag[index] = val;
	} else {
		/* variable */
		flags.midi_variable[index] = val;
	}
	return OK;
}
