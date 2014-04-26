/*
 * musstream_ogg.c  music strema for .ogg file
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
/* $Id: musstream_ogg.c,v 1.3 2004/10/31 04:18:06 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <vorbis/vorbisfile.h>

#include "portab.h"
#include "musstream.h"
#include "wavfile.h"

struct _ogg2pcm {
	void *top;
	void *cur;
	ogg_int64_t size;
};
typedef struct _ogg2pcm ogg2pcm_t;

static size_t ovcb_read(void *ptr, size_t size, size_t nmemb, void *ds);
static int    ovcb_seek(void *ds, ogg_int64_t offset, int whence);
static int    ovcb_close(void *ds);
static long   ovcb_tell(void *ds);

static ov_callbacks ovcb = {
	ovcb_read,
	ovcb_seek,
	ovcb_close,
	ovcb_tell
};

static size_t ovcb_read(void *ptr, size_t size, size_t nmemb, void *ds) {
	ogg2pcm_t *i = (ogg2pcm_t *)ds;
	int len = MIN(size * nmemb, i->size - (i->cur - i->top));
	// fprintf(stderr, "my read %d, %d, %ld\n", size, nmemb, len);

	if (len < 0) len = 0;

	memcpy(ptr, i->cur, len);
	i->cur += len;
	return len;
}

static int ovcb_seek(void *ds, ogg_int64_t offset, int whence) {
	ogg2pcm_t *i = (ogg2pcm_t *)ds;
	
	switch(whence) {
	case SEEK_SET:
		i->cur = i->top + offset;
		break;
	case SEEK_CUR:
		i->cur += offset;
		break;
	case SEEK_END:
		i->cur = i->top + i->size + offset;
		break;
	}
	return 0;
}

static int ovcb_close(void *ds) {
	g_free(ds);
	return 0;
}

static long ovcb_tell(void *ds) {
	long off;
	ogg2pcm_t *i = (ogg2pcm_t *)ds;

	off = i->cur - i->top;
	// fprintf(stderr, "my tell %ld\n", off);
	return off;
}

WAVFILE *ogg_getinfo(char *data, long size) {
	WAVFILE *wfile;
	ogg2pcm_t *o2p;
	OggVorbis_File *vf;
	vorbis_info *vinfo;
	
	o2p = g_new(ogg2pcm_t, 1);
	o2p->top = o2p->cur = data;
	o2p->size = size;
	
	vf = g_new(OggVorbis_File, 1);
	
	if (0 > ov_open_callbacks(o2p, vf, NULL, 0, ovcb)) {
		goto eexit;
	}
	wfile = g_new0(WAVFILE, 1);
	
	wfile->bytes = (int)ov_pcm_total(vf, -1);
	vinfo = ov_info(vf, -1);
	wfile->type = SND_OGG;
	wfile->rate = vinfo->rate;
	wfile->ch   = vinfo->channels;
	wfile->bits = 16; // ?
	wfile->samples = wfile->bytes / (wfile->ch * 2);
	wfile->data = vf; // とりあえず突っ込んどけ
	
	return wfile;
 eexit:
	g_free(vf);
	g_free(o2p);
	
	return NULL;
}


static int ogg_read(musstream_t *this, void *ptr, int size, int nmemb) {
	long len = size * nmemb;
	long offset;
	int current_section;
	char *buf = ptr;

#ifdef WORDS_BIGENDIAN
	int be = 1;
#else
	int be = 0;
#endif
	
	offset = 0;

	while(1) {
		int ret = ov_read(this->hidden.ogg.vf, buf + offset, len, be, 2, 1, &current_section);
		if (ret <= 0) break;
		
		len    -= ret;
		offset += ret;
	}
	
	if (this->lrswap16) {
		WORD d;
		WORD *src = ptr;
		int i, sample = offset / 4;
		
		for (i = 0; i < sample; i++) {
			d          = *src;
			*src       = *(src + 1);
			*(src + 1) = d;
			src += 2;
		}
	}
	
	return offset;
}

static int ogg_seek(musstream_t *this, int offset, int whence) {
	switch(whence) {
	case SEEK_SET:
		ov_pcm_seek(this->hidden.ogg.vf, offset);
		break;
	case SEEK_CUR:
		break;
	case SEEK_END:
		break;
	}
	return OK;
}

static int ogg_close(musstream_t *this) {
	ov_clear(this->hidden.ogg.vf);
	g_free(this);
	return 0;
}

musstream_t *ms_ogg(WAVFILE *wfile) {
	musstream_t *ms = g_new0(musstream_t, 1);
	
	ms->hidden.ogg.vf = wfile->data;

	ms->read = ogg_read;
	ms->seek = ogg_seek;
	ms->close = ogg_close;
	
	return ms;
}
