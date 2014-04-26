/*
 * midi.extplayer.c  midi play with external player
 *
 * Copyright (C) 1999-   Fumihiko Murata <fmurata@p1.tcnet.ne.jp>
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
/* $Id: midi.extplayer.c,v 1.22 2003/11/09 15:06:13 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <glib.h>

#include "portab.h"
#include "system.h"
#include "counter.h"
#include "midi.h"
#include "music_server.h"
#include "music_pcm.h"
#include "nact.h"


/* for debugging */
#if 0
# define KILL(pid, sig) ( \
	fprintf(stderr, "%s:%d:kill %d %d\n", \
		__FILE__, __LINE__, (pid), (sig)), \
	fflush(stderr), \
	kill((pid), (sig))\
	)

# define KILLPG(pid, sig) ( \
	fprintf(stderr, "%s:%d:killpg %d %d\n", \
		__FILE__, __LINE__, (pid), (sig)), \
	fflush(stderr), \
	killpg((pid), (sig))\
	)
#else
# define KILL(pid, sig) kill((pid), (sig))
# define KILLPG(pid, sig) killpg((pid), (sig))
#endif

extern void sys_set_signalhandler(int SIG, void (*handler)(int));

static int midi_initilize(char *pname, int subdev);
static int midi_exit();
static int midi_start(int no, char *data, int datalen);
static int midi_stop();
static int midi_pause(void);
static int midi_unpause(void);
static int midi_get_playing_info(midiplaystate *st);
static int midi_getflag(int mode, int index);
static int midi_setflag(int mode, int index, int val);
static int midi_setVolumePipe(int vol);
static int midi_getVolumePipe();


#define midi midi_extplayer
mididevice_t midi = {
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
	NULL
};

static boolean enabled = FALSE;
static char    midi_player[256];
static int     argc;
static char    **argv;
static int     midino;    // 現在演奏中の番号
static pid_t   midipid;   // 外部プレーヤーの pid
static int     counter;   // 演奏時間
static boolean pipedplay; // pipe play モードかどうか

static void player_set(char *buf) {
	char *b, *bb;
	int i, j;
	
	if (buf[0] == '-') {
		pipedplay = TRUE;
		buf++;
	} else {
		pipedplay = FALSE;
	}
	
	strncpy(midi_player, buf, sizeof(midi_player));
	b = midi_player;
	
	if (!pipedplay) {
		/* count arguments */
		i = j = 0;		
		while (*b != 0) {
			if (*(b++) == ' ' && j > 0) {
				i++; j = 0;
				while (*b == ' ') b++;
			} else {
				j++;
			}
			if (*b == '\n') *b = 0;
		}
		if (j == 0 && i > 0) i--;
		if (NULL == (argv = (char **)malloc(sizeof(char *) * (i +3)))) {
			return;
		}
		argc = i +1;
		
		/* devide argument */
		b = midi_player;
		j = 0;
		while (j <= i) {
			argv[j] = b;
			while (*b != ' ' && *b != 0) b++;
			*(b++)  = 0;
			while (*b == ' ' || *b == 0) b++;
			j++;
		}
		argv[j +1] = NULL;
		
		/* cut down argv[0] */
		bb = b = argv[0];
		while (*b != 0) {
			if ( *b == '/') bb = b +1;
			b++;
		}
		argv[0] = bb;
	}
}

static int midi_initilize(char *pname, int sub) {
	if (pname == NULL) return -1;
	player_set(pname);
	
	reset_counter_high(SYSTEMCOUNTER_MIDI, 10, 0);
	enabled = TRUE;
	
	if (pipedplay) {
		midi.setvol = midi_setVolumePipe;
		midi.getvol = midi_getVolumePipe;
		NOTICE("midi piped play mode\n");
	} else {
		NOTICE("midi external player mode\n");
	}
	
	return 0;
}

static int midi_exit() {
	char tmpfilename[256];
	int i;
	
	if (enabled) {
		midi_stop();
	}
	
	for (i = 0; i < 256; i++) {
		g_snprintf(tmpfilename, sizeof(tmpfilename)-1, "%s/xsys35-midi_%03d.mid", nact->tmpdir, i);
		unlink(tmpfilename);
	}
	
	return OK;
}


/* no = 0~ */
static int midi_start(int no, char *data, int datalen) {
	char tmpfilename[256];
	FILE *fd;
	char cmd_pipe[256];
	pid_t pid;
	
	g_snprintf(tmpfilename, sizeof(tmpfilename) -1, "%s/xsys35-midi_%03d.mid", nact->tmpdir, no);
	if (NULL == (fd = fopen(tmpfilename, "rb"))) {	
		if (NULL == (fd = fopen(tmpfilename, "wb"))) {
			WARNING("cannot open tmporaryfile");
			return NG;
		}
		fwrite(data, 1, datalen, fd);
	}
	fclose(fd);
	
	if (pipedplay) {
		g_snprintf(cmd_pipe, sizeof(cmd_pipe) -1, "%s %s", midi_player, tmpfilename);
		if (-1 == muspcm_load_pipe(SLOT_MIDIPIPE, cmd_pipe)) {
			return NG;
		}
		muspcm_start(SLOT_MIDIPIPE, 1);
		pid = 1; // dummy
	} else {
		/* arg set */
		argv[argc] = tmpfilename;
		argv[argc +1] = NULL;
		
		pid = fork();
		if (pid == 0) {
			/* child process */
			pid_t mine = getpid();
#ifdef QUITE_MIDI
			close(1);
#endif
			setpgid(mine, mine);
			sys_set_signalhandler(SIGTERM, SIG_DFL);
			execvp(midi_player, argv);
			perror("execvp");
			_exit(-1);
		} else if (pid < 0) {
			WARNING("fork failed");
			return NG;
		}
	}
	
	midino = no;
	midipid = pid;
	counter = get_high_counter(SYSTEMCOUNTER_MIDI);
	
	return OK;
}

static int midi_stop() {
	int status = 0;
	
	if (!enabled || midipid == 0) {
		return OK;
	}
	
	if (!pipedplay) {
		KILL(midipid, SIGCONT);
		KILLPG(midipid, SIGCONT);
		KILL(midipid, SIGTERM);
		KILLPG(midipid, SIGTERM);
		while (0 >= waitpid(midipid, &status, WNOHANG));
	} else {
		muspcm_stop(SLOT_MIDIPIPE);
	}
	
	midipid = 0;
	midino = 0;
	
	return OK;
}

static int midi_pause(void) {
	if (!enabled || midipid == 0) return OK;
	
	if (pipedplay) {
		muspcm_pause(SLOT_MIDIPIPE);
	} else {
		KILLPG(midipid, SIGTSTP);
	}
	return OK;
}

static int midi_unpause(void) {
	if (!enabled || midipid == 0) return OK;
	
	if (pipedplay) {
		muspcm_unpause(SLOT_MIDIPIPE);
	} else {
		KILLPG(midipid, SIGCONT);
	}
	return OK;
}

static int midi_get_playing_info(midiplaystate *st) {
	int status, cnt, err;
	
	if (!enabled || midipid == 0) {
		goto errout;
	}
	
	if (pipedplay) {
		cnt = muspcm_getpos(SLOT_MIDIPIPE);
		if (cnt == 0) {
			goto errout;
		}
	} else {
		if (midipid == (err = waitpid(midipid, &status, WNOHANG))) {
			midipid = 0;
			goto errout;
		}
		cnt = (get_high_counter(SYSTEMCOUNTER_MIDI) - counter) * 10;
	}
	
	st->in_play = TRUE;
	st->play_no = midino;
	st->loc_ms  = cnt;
	
	return OK;

 errout:
	st->in_play = FALSE;
	st->play_no = 0;
	st->loc_ms  = 0;
	return NG;
}

static int midi_getflag(int mode, int index) {
	return 0;
}

static int midi_setflag(int mode, int index, int val) {
	return NG;
}

static int midi_setVolumePipe(int vol) {
	if (prv.pcm[SLOT_CDROMPIPE] != NULL) {
		prv.pcm[SLOT_CDROMPIPE]->vollv = vol;
	}
	return OK;
}

static int midi_getVolumePipe() {
	if (prv.pcm[SLOT_CDROMPIPE] != NULL) {
		return prv.pcm[SLOT_CDROMPIPE]->vollv;
	}
	return 100;
}
