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

#include <portmidi.h>
#include <SDL_atomic.h>
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

static void midi_stop(void);

enum midi_command {
	CMD_PLAY,
	CMD_STOP,
	CMD_QUIT,
	CMD_PAUSE,
	CMD_UNPAUSE,
};

struct midi_message {
	enum midi_command command;
	// for CMD_PLAY
	int seq;
	struct midiinfo *music;
};

static PmDeviceID device_id;
static SDL_Thread *thread;
static struct msgq *queue;
static int start_time;
static SDL_atomic_t atomic_seq;

static void enqueue(struct midi_message msg) {
	struct midi_message *buf = malloc(sizeof(struct midi_message));
	*buf = msg;
	msgq_enqueue(queue, buf);
}
#define ENQUEUE(...) enqueue((struct midi_message){__VA_ARGS__})

static void dequeue(struct midi_message *msg_out) {
	struct midi_message *msg = msgq_dequeue(queue);
	*msg_out = *msg;
	free(msg);
}

static bool dequeue_timeout(uint32_t timeout_ms, struct midi_message *msg_out) {
	struct midi_message *msg = msgq_dequeue_timeout(queue, timeout_ms);
	if (!msg)
		return false;
	*msg_out = *msg;
	free(msg);
	return true;
}

static void allnotesoff(PortMidiStream *stream) {
	for (int i = 0; i < 16; i++) {
		Pm_WriteShort(stream, 0, Pm_Message(0xb0 + i, 0x7b, 0x00));
	}
}

static void send_reset(PortMidiStream *stream) {
	for (int i = 0; i < 16; i++) {
		Pm_WriteShort(stream, 0, Pm_Message(0xb0 + i, 0x78, 0x00));
		Pm_WriteShort(stream, 0, Pm_Message(0xb0 + i, 0x79, 0x00));
	}
}

static PmTimestamp midi_time_proc(void *time_info) {
	return SDL_GetTicks();
}

static uint32_t ticks2ms(int ticks, int division, unsigned int tempo) {
	return ticks * tempo / (division * 1000);
}

static void midi_playloop(PortMidiStream *stream, struct midiinfo *midi) {
	int i = 0;
	int ctick = 0;
	int tempo = 500000;
	uint32_t last_time = SDL_GetTicks();

	while (i < midi->nr_events) {
		if (ctick < midi->event[i].ctime) {
			int delta = midi->event[i].ctime - ctick;
			uint32_t target_time = last_time + ticks2ms(delta, midi->division, tempo);
			uint32_t current_time = SDL_GetTicks();
			while (current_time < target_time) {
				struct midi_message msg;
				if (dequeue_timeout(target_time - current_time, &msg)) {
					switch (msg.command) {
					case CMD_PAUSE:
						while (msg.command != CMD_UNPAUSE) {
							dequeue(&msg);
							switch (msg.command) {
							case CMD_UNPAUSE:
							case CMD_PAUSE:
								break;
							case CMD_STOP:
								return;
							case CMD_PLAY:
							case CMD_QUIT:
								SYSERROR("Cannot happen");
							}
						}
						target_time += SDL_GetTicks() - current_time;
						break;
					case CMD_UNPAUSE:
						break;
					case CMD_STOP:
						return;
					case CMD_PLAY:
					case CMD_QUIT:
						SYSERROR("Cannot happen");
					}
				}
				current_time = SDL_GetTicks();
			}
			last_time = target_time;
			ctick = midi->event[i].ctime;
		}

		if (midi->event[i].type == MIDI_EVENT_NORMAL) {
			Pm_WriteShort(stream, last_time, midi->event[i].data);
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
				allnotesoff(stream);
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
					allnotesoff(stream);
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
					allnotesoff(stream);
				} else {
					i += MIDI_SYS35_EVENT_SIZE;
				}
				break;
			}
		} else {
			WARNING("Unknown type of event %x (NEVER!).", midi->event[i].type);
		}
	}
}

static int midi_thread(void* unused) {
	PortMidiStream *stream;
	PmError err = Pm_OpenOutput(&stream, device_id, NULL, 0, midi_time_proc, NULL, MIDI_LATENCY);
	if (err != pmNoError) {
		WARNING("Pm_OpenOutput failed: %s", Pm_GetErrorText(err));
		stream = NULL;
	}

	if (stream)
		send_reset(stream);

	for (;;) {
		struct midi_message msg;
		dequeue(&msg);
		switch (msg.command) {
		case CMD_PLAY:
			if (stream) {
				midi_playloop(stream, msg.music);
				send_reset(stream);
			}
			SDL_AtomicCAS(&atomic_seq, msg.seq, 0);
			mf_remove_midifile(msg.music);
			break;

		case CMD_STOP:
		case CMD_PAUSE:
		case CMD_UNPAUSE:
			break;

		case CMD_QUIT:
			if (stream) {
				SDL_Delay(MIDI_LATENCY);
				Pm_Close(stream);
			}
			return 0;
		}
	}
}

static bool midi_initialize(int subdev) {
	PmError err = Pm_Initialize();
	if (err != pmNoError) {
		WARNING("%s", Pm_GetErrorText(err));
		return false;
	}

	int ndevices = Pm_CountDevices();
	for (int i = 0; i < ndevices; i++) {
		const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
		NOTICE("MIDI #%d: interf=\"%s\", name=\"%s\", input=%d, output=%d",
		       i, info->interf, info->name, info->input, info->output);
	}
	if (subdev < 0 || subdev >= ndevices) {
		WARNING("invalid midi device number");
		return false;
	}

	device_id = subdev;
	return true;
}

static void midi_exit(void) {
	if (queue) {
		ENQUEUE(CMD_STOP);
		ENQUEUE(CMD_QUIT);
		SDL_WaitThread(thread, NULL);
		thread = NULL;
		msgq_free(queue);
		queue = NULL;
	}
	Pm_Terminate();
}

static void midi_reset(void) {
	midi_stop();
}

/* no = 0~ */
static bool midi_start(int no, int loop, const uint8_t *data, int datalen) {
	static int seq = 0;

	if (queue) {
		ENQUEUE(CMD_STOP);
	} else {
		queue = msgq_new();
		thread = SDL_CreateThread(midi_thread, "MIDI", NULL);
	}

	struct midiinfo *midi = mf_read_midifile(data, datalen);
	if (!midi) {
		WARNING("error reading midi file");
		return false;
	}
	SDL_AtomicSet(&atomic_seq, ++seq);
	ENQUEUE(CMD_PLAY, seq, midi);

	start_time = SDL_GetTicks();

	return true;
}

static void midi_stop(void) {
	if (queue) {
		SDL_AtomicSet(&atomic_seq, 0);
		ENQUEUE(CMD_STOP);
	}
}

static void midi_pause(void) {
	if (queue)
		ENQUEUE(CMD_PAUSE);
}

static void midi_unpause(void) {
	if (queue)
		ENQUEUE(CMD_UNPAUSE);
}

static bool midi_get_playing_info(midiplaystate *st) {
	if (SDL_AtomicGet(&atomic_seq) == 0) {
		st->in_play = false;
		st->loc_ms  = 0;
		return true;
	}

	st->in_play = true;
	st->loc_ms = SDL_GetTicks() - start_time;

	return true;
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

static bool midi_setflag(int mode, int index, int val) {
	if (mode == 0) {
		/* flag */
		flags.midi_flag[index] = val;
	} else {
		/* variable */
		flags.midi_variable[index] = val;
	}
	return true;
}

mididevice_t midi_portmidi = {
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
	.fadestart = NULL,
	.fading = NULL,
};
