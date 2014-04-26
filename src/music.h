/*
 * music.h  music sever/client shared information
 *
 * Copyright (C) 1997-1998 Masaki Chikama (Wren) <chikama@kasumi.ipl.mech.nagoya-u.ac.jp>
 *               1998-                           <masaki-c@is.aist-nara.ac.jp>
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
/* $Id: music.h,v 1.25 2003/11/16 15:29:52 chikama Exp $ */

#ifndef __MUSIC_H__
#define __MUSIC_H__

#include <glib.h>

/* music command */
enum {
	MUS_CDROM_START, /* 0 */
	MUS_CDROM_STOP,
	MUS_CDROM_GETPOSITION,
	MUS_CDROM_GETMAXTRACK,
	
	MUS_PCM_START,
	MUS_PCM_STOP,
	MUS_PCM_LOAD_NO,
	MUS_PCM_LOAD_MEM,
	MUS_PCM_LOAD_LRSW,
	MUS_PCM_UNLOAD,
	MUS_PCM_GETPOSITION,
	MUS_PCM_WAITEND,
	MUS_PCM_WAITTIME,
	MUS_PCM_GETWAVETIME,
	
	MUS_FADE_START,
	MUS_FADE_STOP,
	MUS_FADE_GETSTATE,
	MUS_FADE_GETLEVEL,
	
	MUS_MIDI_START,
	MUS_MIDI_STOP,
	MUS_MIDI_PAUSE,
	MUS_MIDI_UNPAUSE,
	MUS_MIDI_GETPOSITION,
	MUS_MIDI_GETFLAG,
	MUS_MIDI_SETFLAG,

	MUS_MIXER_GETLEVEL,
	MUS_MIXER_SETVOLVAL,
	
	MUS_BGM_PLAY,
	MUS_BGM_STOP,
	MUS_BGM_STOPALL,
	MUS_BGM_FADE,
	MUS_BGM_GETPOS,
	MUS_BGM_GETLEN,
	MUS_BGM_WAIT,
	MUS_BGM_WAITPOS,
	
	MUS_GET_VALIDSUBSYSTEM,
	MUS_EXIT
};

/* socket file */
#define XSYS35MUSSOCK "/xsystem35_sock"

#define XSYS35_PROTOCOL_VERSION 1

typedef struct {
	guint16 version;
	guint16 command;
	guint32 data_length;
} ClientPktHeader;

typedef struct {
	guint16 version;
	guint32 data_length;
} ServerPktHeader;

typedef struct {
	ClientPktHeader hdr;
	gpointer data;
	gint fd;
} PacketNode;

typedef struct {
	boolean cdrom;
	boolean midi;
	boolean pcm;
} ValidSubsystem;

/* init and exit */
extern int mus_init();
extern int mus_exit();

#endif /* __MUSIC_H__ */
