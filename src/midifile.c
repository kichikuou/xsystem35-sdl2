/*
 * midifile.h  standard midifile parser
 * 
 * Copyright (C) 1997-1998 Masaki Chikama (Wren) <chikama@kasumi.ipl.mech.nagoya-u.ac.jp>
 *               1998-                           <masaki-c@is.aist-nara.ac.jp>
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

#include "portab.h"
#include "system.h"
#include "midifile.h"

static long to32bit(int c1, int c2, int c3, int c4);
static void midi_inc();
static void make_chunk(struct midievent *ev, int n);
static void midi_noteoff(int chan, int note, int vol);
static void midi_noteon(int chan, int note, int vol);
static void midi_pressure(int chan, int note, int press);
static void midi_controlparameter(int chan, int cnt, int val);
static void midi_pitchbend(int chan, int lsb, int msg);
static void midi_program(int chan, int prog);
static void midi_chanpressure(int chan, int press);
static void midi_tempo(int tempo);
static void chanmessage(int status, int c1, int c2);
static void metaevent(int type);
static void msginit();
static void msgadd(int c);
// static int  msglen();
static char *msg();
static int  midigetc();
static long readvarinum();
static void read_playevent();
static int  read_header(BYTE *stream, off_t len);

static struct midiinfo *midi;

#define Read16() \
(*stream << 8) | (*(stream + 1)), stream+=2

#define Read32() \
(*stream << 24) | (*(stream + 1) << 16) | (*(stream + 2) << 8) | (*(stream + 3)), stream+=4

static long to32bit(int c1, int c2, int c3, int c4) {
	long value = 0L;
	
	value = (c1 & 0xff);
	value = (value << 8) + (c2 & 0xff);
	value = (value << 8) + (c3 & 0xff);
	value = (value << 8) + (c4 & 0xff);
	
	return value;
}

static void midi_inc() {
	midi->ceptr++;
	if (midi->ceptr > MAXMIDIEVENT) {
		NOTICE("too much event\n");
		sys_exit(0);
	}
}

static void make_chunk(struct midievent *ev, int n) {
	ev->type  = 0;
	ev->ctime = midi->curtime;
	ev->n     = n;
	ev->port  = 0;
	
	if (n != 0) {
		ev->data = malloc(sizeof(unsigned char) * (n + 1));
	}
}

static void midi_noteoff(int chan, int note, int vol) {
	make_chunk(&midi->event[midi->ceptr], 3);
	
	midi->event[midi->ceptr].data[0] = MIDI_NOTEOFF + chan;
	midi->event[midi->ceptr].data[1] = note;
	midi->event[midi->ceptr].data[2] = vol;
	
	midi_inc();
}

static void midi_noteon(int chan, int note, int vol) {
	make_chunk(&midi->event[midi->ceptr], 3);
	
	midi->event[midi->ceptr].data[0] = MIDI_NOTEON + chan;
	midi->event[midi->ceptr].data[1] = note;
	midi->event[midi->ceptr].data[2] = vol;
	
	midi_inc();
}

static void midi_pressure(int chan, int note, int press) {
	make_chunk(&midi->event[midi->ceptr], 3);
	
	midi->event[midi->ceptr].data[0] = MIDI_PRESSURE + chan;
	midi->event[midi->ceptr].data[1] = note;
	midi->event[midi->ceptr].data[2] = press;
	
	midi_inc();
}

static int check_sys35mark(int *fnc, int *val1, int *val2) {
	midi->marker++;
	
	switch(midi->marker) {
	case 1:
		if (midi->event[midi->ceptr].data[1] != 101 ||
		    midi->event[midi->ceptr].data[2] != 100) {
			midi->marker = 0;
		}
		break;
	case 2:
		if (midi->event[midi->ceptr].data[1] != 100) {
			midi->marker = 0;
		}
		break;
	case 3:
		if (midi->event[midi->ceptr].data[1] != 6) {
			midi->marker = 0;
		}
		break;
	case 4:
		if (midi->event[midi->ceptr].data[1] != 38) {
			midi->marker = 0;
		}
		break;
	case 5:
		if (midi->event[midi->ceptr].data[1] != 101 ||
		    midi->event[midi->ceptr].data[2] != 127) {
			midi->marker = 0;
		}
		break;
	case 6:
		if (midi->event[midi->ceptr].data[1] != 100 ||
		    midi->event[midi->ceptr].data[2] != 127) {
			midi->marker = 0;
		} else {
			midi->marker = 0;
			*fnc  = midi->event[midi->ceptr-4].data[2];
			*val1 = midi->event[midi->ceptr-3].data[2];
			*val2 = midi->event[midi->ceptr-2].data[2];
			return 1;
		}
		break;
	}
	return 0;
}

static void midi_controlparameter(int chan, int cnt, int val) {
	int fnc, val1, val2;
	
	make_chunk(&midi->event[midi->ceptr], 3);
	
	midi->event[midi->ceptr].data[0] = MIDI_CONTROL + chan;
	midi->event[midi->ceptr].data[1] = cnt;
	midi->event[midi->ceptr].data[2] = val;
	
	if (check_sys35mark(&fnc, &val1, &val2) > 0) {
		midi->event[midi->ceptr - 5].type = 3;
		/* ラベル登録 */
		if (fnc == 0) {
			midi->sys35_label[val1] = midi->ceptr -5;
		}
	}
	
	midi_inc();
}

static void midi_pitchbend(int chan, int lsb, int msg) {
	make_chunk(&midi->event[midi->ceptr], 3);
	
	midi->event[midi->ceptr].data[0] = MIDI_PITCHB + chan;
	midi->event[midi->ceptr].data[1] = lsb;
	midi->event[midi->ceptr].data[2] = msg;
	
	midi_inc();
}

static void midi_program(int chan, int prog) {
	make_chunk(&midi->event[midi->ceptr], 2);
	
	midi->event[midi->ceptr].data[0] = MIDI_PROGRAM + chan;
	midi->event[midi->ceptr].data[1] = prog;
	
	midi_inc();
}

static void midi_chanpressure(int chan, int press) {
	make_chunk(&midi->event[midi->ceptr], 2);
	
	midi->event[midi->ceptr].data[0] = MIDI_CHANPRES + chan;
	midi->event[midi->ceptr].data[1] = press;
	
	midi_inc();
}

static void midi_tempo(int tempo) {
	unsigned char x[4];
	unsigned int *p;
	
	p = (unsigned int*)x;
	*p = (unsigned int)tempo;
	
	make_chunk(&midi->event[midi->ceptr], sizeof(unsigned long));
	midi->event[midi->ceptr].type = 2; /* tempo change (msec/midi-qnote) */
	midi->event[midi->ceptr].data[0] = x[0];
	midi->event[midi->ceptr].data[1] = x[1];
	midi->event[midi->ceptr].data[2] = x[2];
	midi->event[midi->ceptr].data[3] = x[3];
	
	midi_inc();
}

static void chanmessage(int status, int c1, int c2) {
	int chan = status & 0xf;
	
	switch (status & 0xf0) {
	case 0x80:
		midi_noteoff(chan, c1, c2);
		break;
	case 0x90:
		midi_noteon(chan, c1, c2);
		break;
	case 0xa0:
		midi_pressure(chan, c1, c2);
		break;
	case 0xb0:
		midi_controlparameter(chan, c1, c2);
		break;
	case 0xe0:
		midi_pitchbend(chan, c1, c2);
		break;
	case 0xc0:
		midi_program(chan, c1);
		break;
	case 0xd0:
		midi_chanpressure(chan, c1);
		break;
	default:
		NOTICE("Unknown Message\n");
	}
}

static void metaevent(int type) {
	// int  len = msglen();
	char *m = msg();
	
	switch(type) {
	case 0x00:
		NOTICE("seqnum\n");
		break;
	case 0x01:
		NOTICE("text event (Text Event, Ignored)\n");
		break;
	case 0x02:
		NOTICE("copyright notice (Text Event, Ignored)\n");
		break;
	case 0x03:
		NOTICE("sequence / track name (Text Event, Ignored)\n");
		break;
	case 0x04:
		NOTICE("instrument name (Text Event, Ignored)\n");
		break;
	case 0x05:
		NOTICE("lyric (Text Event, Ignored)\n");
		break;
	case 0x06:
		NOTICE("marker (Text Event, Ignored)\n");
		break;
	case 0x07:
		NOTICE("cue point (Text Event, Ignored)\n");
		break;
	case 0x08:
		NOTICE("meta event 0x08 (Text Event, Ignored)\n");
		break;
	case 0x09:
		NOTICE("meta event 0x09 (Text Event, Ignored)\n");
		break;
	case 0x0a:
		NOTICE("meta event 0x0a (Text Event, Ignored)\n");
		break;
	case 0x0b:
		NOTICE("meta event 0x0b (Text Event, Ignored)\n");
		break;
	case 0x0c:
		NOTICE("meta event 0x0c (Text Event, Ignored)\n");
		break;
	case 0x0d:
		NOTICE("meta event 0x0d (Text Event, Ignored)\n");
		break;
	case 0x0e:
		NOTICE("meta event 0x0e (Text Event, Ignored)\n");
		break;
	case 0x0f:
		NOTICE("meta event 0x0f (Text Event, Ignored)\n");
		break;
	case 0x2f:
		/* EOF */
		midi->length_left = 0;
		midi->eventsize = midi->ceptr;
		break;
	case 0x51:
		midi_tempo(to32bit(0, m[0], m[1], m[2]));
		break;
	case 0x54:
		NOTICE("set smpte\n");
		break;
	case 0x58:
		NOTICE("time sig (Ignore)\n");
		break;
	case 0x59:
		NOTICE("keysig (Ignore)\n");
		break;
	case 0x7f:
		NOTICE("seq specific\n");
		break;
	default:
		NOTICE("meta misc\n");
		break;
	}
}

static void msginit() {
	midi->msgindex = 0;
}

static void msgadd(int c) {
	if (midi->msgindex >= midi->msgsize) {
		midi->msgsize *= 2;
		midi->msgbuffer = (unsigned char *)realloc(midi->msgbuffer, midi->msgsize);
	}
	midi->msgbuffer[midi->msgindex++] = (unsigned char)c;
}

#if 0
static int msglen() {
	return midi->msgindex;
}
#endif

static char *msg() {
	return midi->msgbuffer;
}

static int midigetc() {
	midi->length_left--;
	return *midi->cdata++;
}

/* readvarinum - read a varying-length number, and return the */
/* number of characters it took. */

static long readvarinum() {
	long value;
	int  c;
	
	value = c = midigetc();
	
	if (c & 0x80) {
		value &= 0x7f;
		do {
			c = midigetc();
			value = (value << 7) + (c & 0x7f);
		} while (c & 0x80);
	}
	return value;
}

static void read_playevent() {
	int c, c1, type;
	int running = 0; /* 1 when running status used */
	int status = 0; /* status value (e.g. 0x90==note-on) */
	int delta, needed, lookfor;
	
	/* This array is indexed by the high half of a status byte.  It's */
	/* value is either the number of bytes needed (1 or 2) for a channel */
	/* message, or 0 (meaning it's not  a channel message). */
	static int      chantype[] = {
		0, 0, 0, 0, 0, 0, 0, 0,	/* 0x00 through 0x70 */
		2, 2, 2, 2, 1, 1, 2, 0	/* 0x80 through 0xf0 */
	};
	
	midi->cdata = midi->data;
	midi->length_left = midi->length;
	midi->curtime = 0;
	midi->ceptr = 0;
	
	while(midi->length_left > 0) {
		midi->curtime += (delta = readvarinum()); /* delta time for midi event */

		c = midigetc();
		
		if ((c & 0x80) == 0) {
			running = 1;
		} else {
			status = c;
			running = 0;
		}
		
		needed = chantype[(status >> 4) & 0x0f];
		
		if (needed) {	/* ie. is it a channel message? */
			
			if (running) {
				c1 = c;
			} else {
				c1 = midigetc();
			}
			chanmessage(status, c1, (needed > 1) ? midigetc() : 0);
			
			if ((status & 0xf0) != 0xb0) {
				midi->marker = 0; /* maker flag clear */
			}
			continue;;
		}

		midi->marker = 0; /* maker flag clear */
		
		switch (c) {
		case 0xFF: /* meta event */
			type = midigetc();
			lookfor = midi->length_left - readvarinum();
			msginit();
			
			while(midi->length_left > lookfor) {
				msgadd(midigetc());
			}
			
			metaevent(type);
			break;
			
		case 0xF0: /* system exclusive */
			lookfor = midi->length_left - readvarinum();
			msginit();
			msgadd(0xF0);
			while(midi->length_left > lookfor) {
				msgadd(midigetc());
			}
			NOTICE("system exculsive ( not supported )");
			break;
			
		case 0xF7: /* system exclusive continuation or arbitrary stuff */
			lookfor = midi->length_left - readvarinum();
			msginit();
			while(midi->length_left > lookfor) {
				msgadd(c = midigetc());
			}
			NOTICE("system exclusive cont. ( not supporte d)");
			break;
			
		default:
			NOTICE("Unknow Event ( not supported )");
		}
		
	}
}

static int read_header(BYTE *stream, off_t len) {
	int tracklen, i = 0;
	BYTE *stream_top = stream;
	
	while(i == 0 && stream < (stream_top + len)) {
		if (0 == strncmp(stream, "MThd", 4)) {
			stream += 4;
			i++;
		} else {
			stream++;
		}
	}
	
	if (stream != stream_top) {
		stream -= 4;
	}
	
	i = Read32();
	
	if (i == RIFF) {
		stream += 16;
		i = Read32();
	}
	
	if (i == MThd) {
		tracklen = Read32();
		midi->format = Read16();
		midi->ntrks = Read16();
		midi->division = Read16();
	} else {
		boolean found = FALSE;
		while (!found && stream < (stream_top + len - 8)) {
			if (0 == strncmp(stream, "MThd", 4)) {
				found = TRUE;
			} else {
				stream++;
			}
		}
		if (found) {
			stream += 4;
			tracklen = Read32();
			midi->format = Read16();
			midi->ntrks = Read16();
			midi->division = Read16();
		} else {
			WARNING("unknow format\n");
			return NG;
		}
	}
	
	if (midi->ntrks != 1) {
		WARNING("multiple track (format 1) is not supported");
		return NG;
	}
	
	NOTICE("tracklen = %d, format = %d, ntrks = %d, division = %d\n", 
	       tracklen, midi->format, midi->ntrks, midi->division);
	
	i = Read32();
	if (i != MTrk) {
		WARNING("Unknow format\n");
	}
	
	tracklen = Read32();
	
	if (stream + tracklen > stream_top + len) {
		tracklen = stream + len - stream_top;
	}
	
	midi->length = tracklen;
	midi->data = stream;
	
	return OK;
}

struct midiinfo *mf_read_midifile(BYTE *stream, off_t len) {
	midi = malloc(sizeof(struct midiinfo));
	
	midi->msgsize = 128; /* Initial msg buffer size */
	midi->msgbuffer = calloc(midi->msgsize, sizeof(unsigned char));
	
	if (0 > read_header(stream, len)) {
		return NULL;
	}
	
	read_playevent();
	
	return midi;
}

void mf_remove_midifile(struct midiinfo *m) {
	int i;
	
	free(m->msgbuffer);
	
	for (i = 0; i < m->eventsize; i ++) {
		free(m->event[i].data);
	}
	free(m);
}
