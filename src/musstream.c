/*
 * musstream.c  wrapper for file/pipe/memory 
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
/* $Id: musstream.c,v 1.5 2002/12/31 04:11:19 chikama Exp $ */

#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <errno.h>
#include <signal.h>

#include "portab.h"
#include "system.h"
#include "musstream.h"
#include "counter.h"

static int file_seek(struct _musstream *this, int off, int where);
static int file_read(struct _musstream *this, void *ptr, int size, int maxnum);
static int file_close(struct _musstream *this);
static int pipe_seek(struct _musstream *this, int off, int where);
static int pipe_read(struct _musstream *this, void *ptr, int size, int maxnum);
static int pipe_close(struct _musstream *this);
static int mem_seek(struct _musstream *this, int off, int where);
static int mem_read(struct _musstream *this, void *ptr, int size, int maxnum);
static int mem_close(struct _musstream *this);

static int file_seek(struct _musstream *this, int off, int where) {
	if (0 == fseek(this->hidden.stdio.fp, off, where)) {
		return ftell(this->hidden.stdio.fp);
	} else {
		return -1;
	}
}

static int file_read(struct _musstream *this, void *ptr, int size, int maxnum) {
	size_t nread;

	nread = fread(ptr, size, maxnum, this->hidden.stdio.fp);
	
	return nread;
}

static int file_close(struct _musstream *this) {
	if (this) {
		fclose(this->hidden.stdio.fp);
		g_free(this);
	}
	return 0;
}

static int pipe_seek(struct _musstream *this, int off, int where) {
	FILE *fp;
	char *tbuf = g_new(char, off);
	int num = 0;
	
	switch(where) {
	case SEEK_SET:
		if (this->hidden.pipe.fp) {
			pclose(this->hidden.pipe.fp);
		}
		
		fp = popen(this->hidden.pipe.cmd, "r");
		if (fp == NULL) return -1;
		
		this->hidden.pipe.fp = fp;
		if (off != 0) {
			num = pipe_read(this, tbuf, 1, off);
		}
		break;
	case SEEK_CUR:
		if (off != 0) {
			num = pipe_read(this, tbuf, 1, off);
		}
		break;
	case SEEK_END:
		// no
		num = -1;
		break;
	default:
		WARNING("Unknown value for 'whence'\n");
		g_free(tbuf);
		return -1;
	}
	
	g_free(tbuf);
	return num;
}

static int pipe_read(struct _musstream *this, void *ptr, int size, int maxnum) {
	size_t nread;
	
	nread = fread(ptr, size, maxnum, this->hidden.pipe.fp);
	if (nread == 0 && !feof(this->hidden.pipe.fp)) {
		while(nread == 0) {
			nread = fread(ptr, size, maxnum, this->hidden.pipe.fp);
		}
	}
	
	//printf("nread = %d\n", nread);
	
	return nread;
}

static int pipe_close(struct _musstream *this) {
	if (this) {
		if (this->hidden.pipe.fp) {
			pclose(this->hidden.pipe.fp);
			this->hidden.pipe.fp = NULL;
		}
		g_free(this);
	}
	return 0;
}

static int mem_seek(struct _musstream *this, int offset, int where) {
	void *newpos;
	
	switch(where) {
	case SEEK_SET:
		newpos = this->hidden.mem.base + offset;
		break;
	case SEEK_CUR:
		newpos = this->hidden.mem.cur + offset;
		break;
	case SEEK_END:
		newpos = this->hidden.mem.end + offset;
		break;
	default:
		WARNING("Unknown value for 'whence'\n");
		return -1;
	}
	
	if (newpos < this->hidden.mem.base) {
		newpos = this->hidden.mem.base;
	}
	if (newpos > this->hidden.mem.end) {
		newpos = this->hidden.mem.end;
	}
	
	this->hidden.mem.cur = newpos;
	
	return (this->hidden.mem.cur - this->hidden.mem.base); 
}

static int mem_read(struct _musstream *this, void *ptr, int size, int maxnum) {
	int num = maxnum;
	
	if ((this->hidden.mem.cur + (num*size)) > this->hidden.mem.end) {
		num = (this->hidden.mem.end - this->hidden.mem.cur) / size;
	}
	
	if (num == 0) return 0;
	
	//printf("copy len = %d, cur = %p\n", num * size, this->hidden.mem.cur);
	memcpy(ptr, this->hidden.mem.cur, num * size);
	this->hidden.mem.cur += (num * size);
	
	return num;
}

static int mem_close(struct _musstream *this) {
	if (this) {
		g_free(this);
	}
	return 0;
}

musstream_t *ms_file(char *filename) {
	FILE *fp;
	musstream_t *s;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		WARNING("file open(%s) error\n", filename);
		return NULL;
	}
	
	s = g_new0(musstream_t, 1);
	s->hidden.stdio.fp = fp;
	s->seek = file_seek;
	s->read = file_read;
	s->close = file_close;
	
	return s;
}

musstream_t *ms_pipe(char *cmd) {
	FILE *fp;
	musstream_t *s;

#if 0
	fp = popen(cmd, "r");
	if (fp == NULL) {
		WARNING("pipe open(%s) error\n", cmd);
		return NULL;
	}
#endif
	fp = NULL;
	
	s = g_new0(musstream_t, 1);
	s->hidden.pipe.fp = fp;
	s->hidden.pipe.cmd = cmd;
	
	s->seek = pipe_seek;
	s->read = pipe_read;
	s->close = pipe_close;
	
	return s;
}

musstream_t *ms_memory(void *ptr, int size) {
	musstream_t *s;
	
	s = g_new0(musstream_t, 1);
	s->hidden.mem.base = ptr;
	s->hidden.mem.cur = ptr;
	s->hidden.mem.end = ptr + size;
	
	s->seek = mem_seek;
	s->read = mem_read;
	s->close = mem_close;
	
	return s;
}

#if 0
int main(int argc, char *argv[]) {
	musstream_t *ms;
	char buf[256];
	char readbuf[4096] = {255};
	int num, i;
	
	sprintf(buf, "mpg123-esd -s -q %s", argv[1]);
	printf("buf = %s\n", buf);
	
	ms = ms_pipe(buf); sleep(2);
	if (ms == NULL) {
		puts("fail");
		exit(1);
	}
	ms->seek(ms, 0, SEEK_SET);
	for (i = 0; num != 0; i++) {
		num = ms->read(ms, readbuf, 1, sizeof(readbuf));
		printf("%d %d, %d\n", num, readbuf[100], readbuf[105]);
		usleep(100*1000);
	}
	ms->close(ms);
}
#endif
