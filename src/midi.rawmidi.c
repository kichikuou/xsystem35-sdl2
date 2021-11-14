/*
 * midi.rawmidi.c  midi play with rawmidi device
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
/* $Id: midi.rawmidi.c,v 1.7 2003/01/31 12:58:28 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#ifdef ENABLE_MIDI_SEQMIDI
#if defined(__FreeBSD__)
#include <machine/soundcard.h>
#elif defined(__OpenBSD__) || defined(__NetBSD__)
#include <soundcard.h>
#else
#include <sys/soundcard.h>
#endif
#include <sys/ioctl.h>
#endif

#include <SDL_thread.h>
#include <SDL_timer.h>
#include "portab.h"
#include "system.h"
#include "midi.h"
#include "music_private.h"
#include "midifile.h"
#include "msgqueue.h"

static struct {
	char midi_variable[128];
	char midi_flag[128];
} flags;

static void send_reset();
static int midi_initilize(char *devnm, int subdev);
static int midi_exit();
static int midi_start(int no, int loop, char *data, int datalen);
static int midi_stop();
static int midi_pause(void);
static int midi_unpause(void);
static int midi_get_playing_info(midiplaystate *st);
static int midi_getflag(int mode, int index);
static int midi_setflag(int mode, int index, int val);

static void (*myflush)(void);
static void (*mywrite)(unsigned char *d, int n, int p);

#ifdef ENABLE_MIDI_RAWMIDI
static void raw_myflush();
static void raw_mywrite(unsigned char *d, int n, int p);
#endif
#ifdef ENABLE_MIDI_SEQMIDI
static void seq_myflush();
static void seq_mywrite(unsigned char *d, int n, int p);
#endif

mididevice_t midi_rawmidi = {
	midi_initilize,
	midi_exit,
	midi_start,
	midi_stop,
	midi_pause,
	midi_unpause,
	midi_get_playing_info,
	midi_getflag,
	midi_setflag,
	NULL,
	NULL,
	NULL,
	NULL
};

#ifndef MIDI_DEVICE
#define MIDI_DEVICE "/dev/midi"
#endif

#ifndef SEQ_DEVICE
#define SEQ_DEVICE "/dev/sequencer"
#endif

static boolean enabled = FALSE;
static int midifd = -1;
static char *mididevname;
static SDL_Thread *thread;
static struct msgq *queue;
static int start_time;

static char cmd_pause[] = "pause";
static char cmd_unpause[] = "unpause";
static char cmd_stop[] = "stop";

#define ALL_NOTEOFF_SZ 48
static char all_noteoff[] = { 0xb0, 0x7b, 0x00,
			      0xb1, 0x7b, 0x00,
			      0xb2, 0x7b, 0x00,
			      0xb3, 0x7b, 0x00,
			      0xb4, 0x7b, 0x00,
			      0xb5, 0x7b, 0x00,
			      0xb6, 0x7b, 0x00,
			      0xb7, 0x7b, 0x00,
			      0xb8, 0x7b, 0x00,
			      0xb9, 0x7b, 0x00,
			      0xba, 0x7b, 0x00,
			      0xbb, 0x7b, 0x00,
			      0xbc, 0x7b, 0x00,
			      0xbd, 0x7b, 0x00,
			      0xbe, 0x7b, 0x00,
			      0xbf, 0x7b, 0x00 };

#define ALL_RESET_SZ 96
static char all_reset[] = { 0xb0, 0x78, 0x00, 0xb0, 0x79, 0x00,
			    0xb1, 0x78, 0x00, 0xb1, 0x79, 0x00,
			    0xb2, 0x78, 0x00, 0xb2, 0x79, 0x00,
			    0xb3, 0x78, 0x00, 0xb3, 0x79, 0x00,
			    0xb4, 0x78, 0x00, 0xb4, 0x79, 0x00,
			    0xb5, 0x78, 0x00, 0xb5, 0x79, 0x00,
			    0xb6, 0x78, 0x00, 0xb6, 0x79, 0x00,
			    0xb7, 0x78, 0x00, 0xb7, 0x79, 0x00,
			    0xb8, 0x78, 0x00, 0xb8, 0x79, 0x00,
			    0xb9, 0x78, 0x00, 0xb9, 0x79, 0x00,
			    0xba, 0x78, 0x00, 0xba, 0x79, 0x00,
			    0xbb, 0x78, 0x00, 0xbb, 0x79, 0x00,
			    0xbc, 0x78, 0x00, 0xbc, 0x79, 0x00,
			    0xbd, 0x78, 0x00, 0xbd, 0x79, 0x00,
			    0xbe, 0x78, 0x00, 0xbe, 0x79, 0x00,
			    0xbf, 0x78, 0x00, 0xbf, 0x79, 0x00 };


static void send_reset() {
	mywrite(all_reset, ALL_RESET_SZ, 0);
	usleep(60000);
	myflush();
	usleep(70000);
	return;
}

#ifdef ENABLE_MIDI_RAWMIDI

#define WRBFSIZE	16384
static unsigned char wrbuff[WRBFSIZE];
static int nwr = 0; /* wrbuff の index */

static void raw_myflush() {
	for (int i = 0; i < nwr; i++) {
		while (write(midifd, wrbuff+i, 1) != 1);
	}
	nwr = 0;
}

/*
 * write the code to the buffer
 */
static void raw_mywrite(unsigned char *d, int n, int p) {
	
	if ((nwr + n + 2) >= WRBFSIZE) {
		myflush();
	}
	
	while (n--) {
		wrbuff[nwr++] = *(d++);
	}
	return;
}
#endif

#ifdef ENABLE_MIDI_SEQMIDI
SEQ_DEFINEBUF(2048);
SEQ_USE_EXTBUF();
static int midi_subdev;

void seqbuf_dump(void) {
	if (_seqbufptr) {
		if (write(midifd, _seqbuf, _seqbufptr) == -1)
			WARNING("write: %s\n", strerror(errno));
	}
	_seqbufptr = 0;
}

static void seq_myflush() {
	seqbuf_dump();
}

/*
 * write the code to the buffer
 */
static void seq_mywrite(unsigned char *d, int n, int p) {
	
	while(n--) {
		SEQ_MIDIOUT(midi_subdev, *(d++));
	}
	return;
}
#endif



static uint32_t ticks2ms(int ticks, int division, unsigned int tempo) {
	return ticks * tempo / (division * 1000);
}

static void *midi_mainloop(struct midiinfo *midi) {
	int i = 0;
	int ctick = 0;
	int tempo = 500000;
	uint32_t last_time = SDL_GetTicks();

	void *cmd;
	while (i < midi->eventsize) {
		if (ctick < midi->event[i].ctime) {
			myflush();
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

		if (midi->event[i].type == 0) {
			/* ordinary data */
			mywrite(midi->event[i].data, midi->event[i].n, midi->event[i].port);
			i++;
		} else if (midi->event[i].type == 2) {
			/* tempo change */
			int *p = (int*)midi->event[i].data;
			tempo = (unsigned int)*p;
			i++;
		} else if (midi->event[i].type == 3) {
			/* system35 maker */
			int vn1 = midi->event[i+1].data[2];
			int vn2 = midi->event[i+2].data[2];
			int vn3 = midi->event[i+3].data[2];
				
			switch (vn1) {
			case 0:
				/* set label */
				i += 6;
				break;
			case 1:
				/* jump */
				mywrite(all_noteoff, ALL_NOTEOFF_SZ, 0);
				i = midi->sys35_label[vn2];
				ctick = midi->event[i].ctime;
				break;
			case 2:
				/* set flag */
				flags.midi_flag[vn2] = vn3;
				i += 6;
				break;
			case 3:
				/* flag jump */
				if (flags.midi_flag[vn2] == 1) {
					i = midi->sys35_label[vn3];
					ctick = midi->event[i].ctime;
					mywrite(all_noteoff, ALL_NOTEOFF_SZ, 0);
				} else {
					i += 6;
				}
				break;
			case 4:
				/* set variable */
				flags.midi_variable[vn2] = vn3;
				i += 6;
				break;
			case 5:
				/* variable jump */
				if (--(flags.midi_variable[vn2]) == 0) {
					i = midi->sys35_label[vn3];
					ctick = midi->event[i].ctime;
					mywrite(all_noteoff, ALL_NOTEOFF_SZ, 0);
				} else {
					i += 6;
				}
				break;
			}
		} else {
			WARNING("Unknown type of event %x (NEVER!).\n", midi->event[i].type);
		}
	}
	return NULL;
}

static int midi_initilize(char *devnm, int subdev) {
	enabled = FALSE;
	
	if (devnm == NULL) {
		if (subdev == -1) {
			mididevname = MIDI_DEVICE;
		} else {
			mididevname = SEQ_DEVICE;
		}
	} else {
		mididevname = devnm;
	}

	if (subdev == -1) {
#ifdef ENABLE_MIDI_RAWMIDI
		myflush = raw_myflush;
		mywrite = raw_mywrite;
		NOTICE("RAWMIDI Initilize OK\n");
#endif
	} else {
#ifdef ENABLE_MIDI_SEQMIDI
		myflush = seq_myflush;
		mywrite = seq_mywrite;
		midi_subdev = subdev;
		NOTICE("SEQMIDI Initilize OK\n");
#endif
	}
	
	enabled = TRUE;
	return OK;
}

static int midi_exit() {
	if (enabled) {
		midi_stop();
	}
	return OK;
}

static int midi_thread(void *data) {
	send_reset();

	void *cmd = midi_mainloop(data);

	send_reset();
	mf_remove_midifile(data);
	close(midifd);
	midifd = -1;

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
		WARNING("error reading midi file\n");
		return NG;
	}

	if (0 > (midifd = open(mididevname, O_RDWR))) {
		WARNING("error opening %s: %s\n", mididevname, strerror(errno));
		mf_remove_midifile(midi);
		return NG;
	}

#ifdef ENABLE_SEQMIDI
	{
		int nrsynths;
		if (-1 == ioctl(mididev,  SNDCTL_SEQ_NRSYNTHS, &nrsynths)) {
			WARNING("SNDCTL_SEQ_NRSYNTHS: %s\n", strerror(errno));
			mf_remove_midifile(midi);
			close(midifd);
			midifid = -1;
			return NG;
		}
	}
#endif

	queue = msgq_new();
	thread = SDL_CreateThread(midi_thread, "MIDI", midi);

	start_time = SDL_GetTicks();
	
	return OK;
}

static int midi_stop() {
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

static int midi_get_playing_info(midiplaystate *st) {
	if (!enabled || !queue) {
		st->in_play = FALSE;
		st->loc_ms  = 0;
		return OK;
	}
	
	if (midifd < 0) {
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
