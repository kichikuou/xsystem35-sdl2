/*
 * midifile.h  standard midifile parser
 * 
 * Copyright (C) 1997-1998 Masaki Chikama (Wren) <chikama@kasumi.ipl.mech.nagoya-u.ac.jp>
 *               1998-                           <masaki-c@is.aist-nara.ac.jp>
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
/* $Id: midifile.c,v 1.4 2002/12/31 04:11:19 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <portmidi.h>

#include "portab.h"
#include "system.h"
#include "midifile.h"

// magic bytes
#define RIFF 0x52494646
#define MThd 0x4d546864
#define MTrk 0x4d54726b

// MIDI commands
#define MIDI_CONTROL 0xb0

#define META_END_OF_TRACK 0x2f
#define META_SET_TEMPO 0x51

struct stream {
	const uint8_t *p;
	const uint8_t *end;
};

static inline uint8_t read8(struct stream *input) {
	return *input->p++;
}

static inline uint16_t read16_be(struct stream *s) {
	uint16_t val = s->p[0] << 8 | s->p[1];
	s->p += 2;
	return val;
}

static inline uint32_t read32_be(struct stream *s) {
	uint32_t val = s->p[0] << 24 | s->p[1] << 16 | s->p[2] << 8 | s->p[3];
	s->p += 4;
	return val;
}

static int read_vlq(struct stream *input) {
	int c, value = 0;
	do {
		c = read8(input);
		value = (value << 7) | (c & 0x7f);
	} while (c & 0x80);
	return value;
}

/*
 * System 3.x defines custom (non-standard) MIDI commands using Registered
 * Parameter Numbers (RPN). A custom command is represented by the following
 * sequence of MIDI events:
 *
 *   b0 65 64  ; set RPN MSB to 100
 *   b0 64 XX  ; set RPN LSB to XX
 *   b0 06 YY  ; set data MSB to YY
 *   b0 26 ZZ  ; set data LSB to ZZ
 *   b0 65 7f  ; unset RPN MSB
 *   b0 64 7f  ; unset RPN LSB
 *
 * XX represents the command number, and YY and ZZ are data values. The
 * following commands are defined:
 *
 * 00: Label definition. Defines a label #YY at the location of this event.
 * 01: Label jump. Jumps to the label #YY.
 * 02: Flag set. Sets the flag #YY to the value ZZ.
 * 03: Flag jump. If the flag #YY is set to 1, jumps to the label #YY.
 * 04: Variable set. Sets the variable #YY to the value ZZ.
 * 05: Variable jump. Decrements the variable #YY by 1. If it reaches 0, jumps
 *     to the label #ZZ.
 *
 * Flags and variables can be retrieved or set from scenario scripts, using the
 * SG command.
 */

#define CC_DATA_ENTRY_MSB 0x06
#define CC_DATA_ENTRY_LSB 0x26
#define CC_RPN_LSB 0x64
#define CC_RPN_MSB 0x65
#define SYS35_RPN_MSB 0x64
#define RPN_NULL 0x7f

static bool check_sys35cmd(int32_t msg, int *state) {
	if ((Pm_MessageStatus(msg) & 0xf0) != MIDI_CONTROL) {
		*state = 0;
		return false;
	}

	int data1 = Pm_MessageData1(msg);
	int data2 = Pm_MessageData2(msg);
	(*state)++;

	switch (*state) {
	case 1:
		if (data1 == CC_RPN_MSB && data2 == SYS35_RPN_MSB)
			return false;
		break;
	case 2:
		if (data1 == CC_RPN_LSB)
			return false;
		break;
	case 3:
		if (data1 == CC_DATA_ENTRY_MSB)
			return false;
		break;
	case 4:
		if (data1 == CC_DATA_ENTRY_LSB)
			return false;
		break;
	case 5:
		if (data1 == CC_RPN_MSB && data2 == RPN_NULL)
			return false;
		break;
	case 6:
		if (data1 == CC_RPN_LSB && data2 == RPN_NULL) {
			*state = 0;
			return true;
		}
		break;
	}
	*state = 0;
	return false;
}

static void add_event(struct midiinfo *midi, int *event_buffer_size, enum midi_event_type type, int curtime, int32_t data) {
	if (midi->nr_events >= *event_buffer_size) {
		*event_buffer_size = *event_buffer_size ? *event_buffer_size * 3 / 2 : 4096;
		midi->event = realloc(midi->event, *event_buffer_size * sizeof(struct midievent));
	}
	midi->event[midi->nr_events].type = type;
	midi->event[midi->nr_events].ctime = curtime;
	midi->event[midi->nr_events].data = data;
	midi->nr_events++;
}

static void read_events(struct midiinfo *midi, struct stream *input) {
	/* This array is indexed by the high half of a status byte.  It's */
	/* value is either the number of bytes needed (1 or 2) for a channel */
	/* message, or 0 (meaning it's not  a channel message). */
	const int chantype[] = {
		0, 0, 0, 0, 0, 0, 0, 0,	/* 0x00 through 0x70 */
		2, 2, 2, 2, 1, 1, 2, 0	/* 0x80 through 0xf0 */
	};

	int curtime = 0;
	int event_buffer_size = 0;

	int status = 0; /* status value (e.g. 0x90==note-on) */
	bool running = false;
	int sys35cmd_state = 0;
	while (input->p < input->end) {
		int delta = read_vlq(input);
		curtime += delta; /* delta time for midi event */

		int c = read8(input);

		if (!(c & 0x80)) {
			running = true;
		} else {
			status = c;
			running = false;
		}

		int needed = chantype[(status >> 4) & 0x0f];

		if (needed) {	/* ie. is it a channel message? */
			int c1 = running ? c : read8(input);
			int c2 = needed > 1 ? read8(input) : 0;
			int32_t msg = Pm_Message(status, c1, c2);
			add_event(midi, &event_buffer_size, MIDI_EVENT_NORMAL, curtime, msg);

			if (check_sys35cmd(msg, &sys35cmd_state)) {
				int command_top = midi->nr_events - MIDI_SYS35_EVENT_SIZE;
				midi->event[command_top].type = MIDI_EVENT_SYS35;
				int cmd = Pm_MessageData2(midi->event[command_top + 1].data);
				int val1 = Pm_MessageData2(midi->event[command_top + 2].data);
				if (cmd == MIDI_SYS35_LABEL_DEFINITION)
					midi->sys35_label[val1] = command_top;
			}
			continue;
		}

		sys35cmd_state = 0;

		int type, len;
		switch (c) {
		case 0xFF: /* meta event */
			type = read8(input);
			len = read_vlq(input);
			switch(type) {
			case META_END_OF_TRACK:
				return;
			case META_SET_TEMPO:
				add_event(midi, &event_buffer_size, MIDI_EVENT_TEMPO, curtime, input->p[0] << 16 | input->p[1] << 8 | input->p[2]);
				break;
			}
			input->p += len;
			break;

		case 0xF0: /* system exclusive */
			input->p += read_vlq(input);
			break;

		case 0xF7: /* system exclusive continuation or arbitrary stuff */
			input->p += read_vlq(input);
			break;

		default:
			WARNING("SMF: Unknown Event 0x%x", c);
		}
	}
}

static bool read_header(struct midiinfo *midi, struct stream *input) {
	int magic = read32_be(input);
	if (magic == RIFF) {
		input->p += 16;
		magic = read32_be(input);
	}
	if (magic != MThd)
		return false;

	if (read32_be(input) != 6)  // header size
		return false;
	if (read16_be(input) != 0)  // format
		return false;
	if (read16_be(input) != 1)  // number of tracks
		return false;
	midi->division = read16_be(input);

	magic = read32_be(input);
	if (magic != MTrk)
		return false;

	int tracklen = read32_be(input);
	if (input->p + tracklen < input->end)
		input->end = input->p + tracklen;

	return true;
}

struct midiinfo *mf_read_midifile(const uint8_t *data, size_t len) {
	struct stream input = { .p = data, .end = data + len };
	struct midiinfo *midi = calloc(1, sizeof(struct midiinfo));

	if (!read_header(midi, &input)) {
		free(midi);
		return NULL;
	}

	read_events(midi, &input);

	return midi;
}

void mf_remove_midifile(struct midiinfo *m) {
	free(m->event);
	free(m);
}
