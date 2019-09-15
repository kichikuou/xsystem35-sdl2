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
/* $Id: midifile.h,v 1.2 2001/03/22 11:10:13 chikama Exp $ */

#ifndef __MIDIFILE_H__
#define __MIDIFILE_H__

#include <stdio.h>
#include <sys/types.h>

#include "portab.h"

/* Non-standard MIDI file formats */
#define RIFF                    0x52494646
#define CTMF                    0x43544d46
/* Standard MIDI file format definitions */
#define MThd                    0x4d546864
#define MTrk                    0x4d54726b

/* MIDI COMMANDS */
#define MIDI_NOTEOFF    0x80    /* Note off */
#define MIDI_NOTEON     0x90    /* Note on */
#define MIDI_PRESSURE   0xa0    /* Polyphonic key pressure */
#define MIDI_CONTROL    0xb0    /* Control change */
#define MIDI_PROGRAM    0xc0    /* Program change */
#define MIDI_CHANPRES   0xd0    /* Channel pressure */
#define MIDI_PITCHB     0xe0    /* Pitch wheel change */
#define MIDI_SYSEX      0xf0    /* System exclusive data */

#define MAXMIDIEVENT 65536 /* maxium event sequence number */

struct midievent {
	int type;             /* Data type 
				 0: normal
				 1: text
				 2: tempo change
				 3: system35 message
			      */
	unsigned long ctime;  /* steptime */
	int n;                /* data length */
	int port;             /* midi port */
	unsigned char *data;  /* midi data */
};

struct midiinfo {
	int format;   /* MIDI format version ( only 0 is supported) */
	int division; /* division for delta time*/
	int ntrks;    /* number of track */
	
	BYTE *data;
	BYTE *cdata;
	int length;
	int length_left;
	
	struct midievent event[MAXMIDIEVENT]; /* event data */
	int eventsize; /* total event size */
	int ceptr; /* current event pointer */
	
	/* work info */
	long curtime;    /* current time */
	size_t msgindex; /* midi message buffer index */
	size_t msgsize;  /* size of current allocaed msg */
	unsigned char *msgbuffer; /* message buffer */
	
	/* system35 jump info */
	int  sys35_label[127];
	int marker;
};

extern struct midiinfo *mf_read_midifile(BYTE *stream, off_t len);
extern void mf_remove_midifile(struct midiinfo *m);

#endif /* __MIDIFILE_H__ */
