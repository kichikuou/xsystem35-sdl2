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
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>

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

#include "portab.h"
#include "system.h"
#include "midi.h"
#include "music_private.h"
#include "midifile.h"
#include "counter.h"

struct _midiflag {
	char midi_variable[128];
	char midi_flag[128];
};
typedef struct _midiflag midiflag_t;


static void signal_pause(int sig_num);
static void send_reset();
static unsigned long ticks2usec(unsigned long ticks, int division, unsigned int tempo);
static void parse_event();
static int midi_initilize(char *devnm, int subdev);
static int midi_exit();
static int midi_start(int no, int loop, char *data, int datalen);
static int midi_stop();
static int midi_pause(void);
static int midi_unpause(void);
static int midi_get_playing_info(midiplaystate *st);
static int midi_getflag(int mode, int index);
static int midi_setflag(int mode, int index, int val);

static long(* myflush)(void);
static void(* mywrite)(unsigned char *d, int n, int p);

#ifdef ENABLE_MIDI_RAWMIDI
static long raw_myflush();
static void raw_mywrite(unsigned char *d, int n, int p);
#endif
#ifdef ENABLE_MIDI_SEQMIDI
static long seq_myflush();
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
static struct midiinfo *midi;
static int midifd;
static char *mididevname;
static pid_t midipid;
static int counter;

static int shmkey;
static midiflag_t *flags;


static boolean cmd_pause;
static boolean cmd_stop;

static long extratime;

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


extern void sys_set_signalhandler(int SIG, void (*handler)(int));

static void signal_pause(int sig_num) {
	switch(sig_num) {
	case SIGTERM:
		cmd_stop = TRUE;
		break;
	case SIGUSR1:
		/* pause */
		if (!cmd_pause) {
			cmd_pause = TRUE;
		}
		break;
		
	case SIGUSR2:
		/* unpause */
		if (cmd_pause) {
			cmd_pause = FALSE;
		}
		break;
	}
}

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

static long raw_myflush() {
	struct timeval st, et;
	static long ret;
	static long t1, t2;
	int i;
	
	gettimeofday(&st, NULL);
	
	for (i = 0 ;i < nwr ; i++) {
		while(write(midifd, wrbuff+i, 1) != 1);
	}
	
	nwr = 0;
	gettimeofday(&et, NULL);
	
	t1 = et.tv_usec - st.tv_usec;
	t2 = et.tv_sec  - st.tv_sec;
	while (t1 < 0) {
		t2--;
		t1 += 1000000;
	}
	
	ret = t2 * 1000000 + t1 + extratime;
	
	extratime = 0;
	return ret;
}

/*
 * write the code to the buffer
 */
static void raw_mywrite(unsigned char *d, int n, int p) {
	
	if ((nwr + n + 2) >= WRBFSIZE) {
		extratime = myflush();
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

void seqbuf_dump() {
	if (_seqbufptr) {
		if (write(midifd, _seqbuf, _seqbufptr) == -1) {
			perror("write");
			exit(-1);
		}
	}
	_seqbufptr = 0;
}

static long seq_myflush() {
	struct timeval st, et;
	static long ret;
	static long t1, t2;
	
	gettimeofday(&st, NULL);
	
	seqbuf_dump();
	
	gettimeofday(&et, NULL);
	
	t1 = et.tv_usec - st.tv_usec;
	t2 = et.tv_sec  - st.tv_sec;
	while (t1 < 0) {
		t2--;
		t1 += 1000000;
	}
	
	ret = t2 * 1000000 + t1 + extratime;
	
	extratime = 0;
	return ret;
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



/*
 * tick count to micro second for (usleep)
 * 
 */
static unsigned long ticks2usec(unsigned long ticks, int division, unsigned int tempo) {
	
	return (unsigned long) ((ticks) * (tempo) / (division));
}

static void parse_event() {
	int i = 0;
	int tick = 0;
	int ctick = 0;
	int tempo = 500000;
	unsigned long ts, tu;
	struct timeval tp;
	
	gettimeofday(&tp, NULL);
	ts = tp.tv_sec;
	tu = tp.tv_usec;
	
	while(TRUE) {
		/* pause / stop check */
		if (cmd_stop) {
			break;
		}
		
		if (cmd_pause) {
			gettimeofday(&tp, NULL);
			ts = tp.tv_sec;
			tu = tp.tv_usec;
			usleep(50*1000);
			continue;
		}
		
		while(tick == midi->event[i].ctime) {
			/* delta wait */
			if (ctick != midi->event[i].ctime) {
				int delta, stime;
				
				delta = midi->event[i].ctime - ctick;
				stime = ticks2usec(delta, midi->division, tempo) - myflush();
				if (stime > 0) {
					long j;
					struct timespec ttmp, *req;
					
					req = &ttmp;
					req->tv_sec = (time_t)stime / 1000000;
					req->tv_nsec = (long)(stime % 1000000) * 1000;
					nanosleep(req, NULL);
					
					gettimeofday(&tp, NULL);
					j = (tp.tv_sec * 1000000 + tp.tv_usec) - (ts * 1000000 + tu);
					if (j != stime) {
						extratime += (j - stime);
					}
				} else {
					gettimeofday(&tp, NULL);
					extratime +=  -stime;
				}
				ts = tp.tv_sec;
				tu = tp.tv_usec;
				ctick = midi->event[i].ctime;
			}
			if (midi->event[i].type == 0) {
				/* ordinary data */
				mywrite(midi->event[i].data, midi->event[i].n, midi->event[i].port);
				i++;
			} else if (midi->event[i].type == 2) {
				/* tempo change */
				int *p;
				p = (int*)midi->event[i].data;
				tempo = (unsigned int)*p;
				i++;
			} else if (midi->event[i].type == 3) {
				/* system35 maker */
				int vn1 = midi->event[i+1].data[2];
				int vn2 = midi->event[i+2].data[2];
				int vn3 = midi->event[i+3].data[2];
				
				switch(vn1) {
				case 0:
					/* set label */
					i += 6;
					break;
				case 1:
					/* jump */
					mywrite(all_noteoff, ALL_NOTEOFF_SZ, 0);
					i = midi->sys35_label[vn2];
					ctick = tick = midi->event[i].ctime;
					break;
				case 2:
					/* set flag */
					flags->midi_flag[vn2] = vn3;
					i += 6;
					break;
				case 3:
					/* flag jump */
					if (flags->midi_flag[vn2] == 1) {
						i = midi->sys35_label[vn3];
						ctick = 
						tick = midi->event[i].ctime;
						mywrite(all_noteoff, ALL_NOTEOFF_SZ, 0);
					} else {
						i += 6;
					}
					break;
				case 4:
					/* set variable */
					flags->midi_variable[vn2] = vn3;
					i += 6;
					break;
				case 5:
					/* variable jump */
					if (--(flags->midi_variable[vn2]) == 0) {
						i = midi->sys35_label[vn3];
						ctick =
						tick  = midi->event[i].ctime;
						mywrite(all_noteoff, ALL_NOTEOFF_SZ, 0);
					} else {
						i += 6;
					}
					break;
				}
			} else {
				WARNING("Unknown type of event %x (NEVER!).\n", midi->event[i].type);
			}
			if (i >= midi->eventsize) return;
		}
		tick++;
	}
}

static int midi_initilize(char *devnm, int subdev) {
	enabled = FALSE;
	
	reset_counter_high(SYSTEMCOUNTER_MIDI, 10, 0);
	
	if (devnm == NULL) {
		if (subdev == -1) {
			mididevname = MIDI_DEVICE;
		} else {
			mididevname = SEQ_DEVICE;
		}
	} else {
		mididevname = devnm;
	}

	if (0 > (shmkey = shmget(IPC_PRIVATE, sizeof(midiflag_t),  IPC_CREAT | 0600))) {
		perror("shmget");
		return NG;
	}
	if ((void *)-1 == (flags = (midiflag_t *)shmat(shmkey, 0, 0))) {
		perror("shmat");
		return NG;
	}
	memset(flags, 0, sizeof(midiflag_t));
	
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
		if (0 > shmdt((char *)flags)) {
			perror("shmdt");
		}
		if (0 > shmctl(shmkey, IPC_RMID, 0)) {
			perror("shmctl");
		}
	}
	return OK;
}


/* no = 0~ */
static int midi_start(int no, int loop, char *data, int datalen) {
	pid_t pid;
	
	pid = fork();
	if (pid == 0) {
#ifdef QUIET_MIDI
		close(1);
#endif
		sys_set_signalhandler(SIGTERM, SIG_DFL);
		
		if (NULL == (midi = mf_read_midifile(data, datalen))) {
			_exit(-1);
		}
		
		
		if (0 > (midifd = open(mididevname, O_RDWR))) {
			perror("open");
			_exit(-1);
		}
#ifdef ENABLE_SEQMIDI
		{ int nrsynths; 
		if (-1 == ioctl(mididev,  SNDCTL_SEQ_NRSYNTHS, &nrsynths)) {
			perror("SNDCTL_SEQ_NRSYNTHS");
			_exit(-1);
		}}
#endif

		send_reset();
		
		sys_set_signalhandler(SIGUSR1, signal_pause); /* pause   */
		sys_set_signalhandler(SIGUSR2, signal_pause); /* unpause */
		sys_set_signalhandler(SIGTERM, signal_pause); /* stop    */
		
		/* parse */
		parse_event();
		
		send_reset();
		mf_remove_midifile(midi);
		_exit(0);
	}
	
	midipid = pid;
	counter = get_high_counter(SYSTEMCOUNTER_MIDI);
	
	return OK;
}

static int midi_stop() {
	int status=0;
	
	if (!enabled || midipid == 0) {
		return OK;
	}
	
	kill(midipid, SIGTERM);
	while(0 >= waitpid(midipid, &status, WNOHANG));
	
	midipid = 0;
	
	return OK;
}

static int midi_pause(void) {
	if (!enabled) return OK;
	
	kill(midipid, SIGUSR1);
	return OK;
}

static int midi_unpause(void) {
	if (!enabled) return OK;
	
	kill(midipid, SIGUSR2);
	return OK;
}

static int midi_get_playing_info(midiplaystate *st) {
	int status, cnt, err;
	
	if (!enabled || midipid == 0) {
		st->in_play = FALSE;
		st->loc_ms  = 0;
		return OK;
	}
	
	if (midipid == (err = waitpid(midipid, &status, WNOHANG))) {
		st->in_play = FALSE;
		st->loc_ms  = 0;
		midipid = 0;
		return OK;
	}
	
	cnt = get_high_counter(SYSTEMCOUNTER_MIDI) - counter;
	
	st->in_play = TRUE;
	st->loc_ms = cnt * 10;
	
	return OK;
}

static int midi_getflag(int mode, int index) {
	if (mode == 0) {
		/* flag */
		return flags->midi_flag[index];
	} else {
		/* variable */
		return flags->midi_variable[index];
	}
}

static int midi_setflag(int mode, int index, int val) {
	if (mode == 0) {
		/* flag */
		flags->midi_flag[index] = val;
	} else {
		/* variable */
		flags->midi_variable[index] = val;
	}
	return OK;
}
