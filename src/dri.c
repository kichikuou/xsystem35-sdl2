/*
 * dri.c: dri loader
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
/* $Id: dri.c,v 1.21 2004/10/31 04:18:06 chikama Exp $ */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "portab.h"
#include "system.h"
#include "LittleEndian.h"
#include "dri.h"
#include "sdl_core.h"

static bool read_index(int volume, drifiles *d, FILE *fp) {
	fseek(fp, 0L, SEEK_END);
	int filesize = ftell(fp);

	// Get the size of offsets table and link table.
	uint8_t hdr[6];
	fseek(fp, 0L, SEEK_SET);
	if (fread(hdr, 6, 1, fp) != 1)
		return false;
	int ofssize = LittleEndian_get3B(hdr, 0) << 8;
	int linksize = (LittleEndian_get3B(hdr, 3) << 8) - ofssize;
	if (ofssize <= 0 || linksize <= 0 || ofssize + linksize > filesize)
		return false;

	// Read the link table. In System 3.9, only the first volume's link table is
	// valid. It's different in System 3.8 and earlier, and games that depend
	// on the old behavior (e.g. Child Assassin in Alice CD 2.50) won't work.
	if (!d->link) {
		d->nr_files = linksize / 3;
		d->link = calloc(1, linksize);
		d->offset = calloc(sizeof(uint32_t), d->nr_files);
		fseek(fp, ofssize, SEEK_SET);
		if (fread(d->link, linksize, 1, fp) != 1)
			return false;
	}

	// Read the offsets table.
	uint8_t *otbl = malloc(ofssize);
	fseek(fp, 0L, SEEK_SET);
	if (fread(otbl, ofssize, 1, fp) != 1) {
		free(otbl);
		return false;
	}
	for (int i = 0; i < d->nr_files; i++) {
		if (d->link[i * 3] != volume)
			continue;
		if (d->maxno < i)
			d->maxno = i;
		int offset_index = LittleEndian_getW(d->link, i * 3 + 1);
		d->offset[i] = LittleEndian_get3B(otbl, offset_index * 3) << 8;
	}
	free(otbl);
	return true;
}

drifiles *dri_init(const char **file, int cnt, bool use_mmap) {
	drifiles *d = calloc(1, sizeof(drifiles));
#if !defined(HAVE_MEMORY_MAPPED_FILE) || defined(__EMSCRIPTEN__)
	use_mmap = false;
#endif

	for (int i = 0; i < cnt; i++) {
		if (!file[i])
			continue;

		FILE *fp = fopen(file[i], "rb");
		if (!fp)
			SYSERROR("%s: %s", file[i], strerror(errno));
		if (!read_index(i + 1, d, fp)) {
			// Only errors in *A.ALD are fatal, because some games have
			// dummy (invalid) *[B-Z].ALD files.
			if (i == 0)
				SYSERROR("%s: not an ALD file", file[i]);
			WARNING("%s: not an ALD file", file[i]);
			fclose(fp);
			continue;
		}
		d->fnames[i] = strdup(file[i]);
		fclose(fp);

		if (use_mmap) {
			mmap_t *m = map_file(file[i]);
			if (!m) {
				use_mmap = d->mmapped = false;
				i = 0; /* retry */
			} else {
				d->mmap[i] = m;
				d->mmapped = true;
			}
		}
	}
	return d;
}

bool dri_is_linked(drifiles *d, int no) {
	return no >= 0 && no < d->nr_files && d->link[no * 3];
}

bool dri_exists(drifiles *d, int no) {
	return dri_is_linked(d, no) && d->offset[no];
}

static void warn_missing_ald(drifiles *d, int no) {
	static bool warned = false;
	if (warned) return;
	warned = true;

	int vol = d->link[no * 3];
	char msgbuf[1024];
	if (d->fnames[vol - 1]) {
		snprintf(msgbuf, sizeof(msgbuf), "Cannot read resource #%d from %s.", no, d->fnames[vol - 1]);
	} else if (d->fnames[0]) {
		char *fname = strdup(d->fnames[0]);
		char *p = strrchr(fname, '.');
		if (p && p > fname)
			p[-1] = vol - 1 + 'A';
		snprintf(msgbuf, sizeof(msgbuf), "%s is not found.\nPlease make sure you have all .ALD files copied from the CD-ROM.", fname);
		free(fname);
	} else {
		snprintf(msgbuf, sizeof(msgbuf), "Cannot read resource #%d.", no);
	}
	sdl_showMessageBox(MESSAGEBOX_WARNING, "xsystem35", msgbuf);
}

dridata *dri_getdata(drifiles *d, int no) {
	if (!dri_exists(d, no)) {
		if (dri_is_linked(d, no))
			warn_missing_ald(d, no);
		return NULL;
	}
	int volume = d->link[no * 3];

	uint8_t *data;
	int ptr, size;
	if (d->mmapped) {
		data = d->mmap[volume - 1]->addr + d->offset[no];
		ptr  = LittleEndian_getDW(data, 0);
		size = LittleEndian_getDW(data, 4);
	} else {
		FILE *fp = fopen(d->fnames[volume - 1], "rb");
		if (!fp)
			return NULL;
		uint8_t entry_header[8];
		fseek(fp, d->offset[no], SEEK_SET);
		if (fread(entry_header, sizeof(entry_header), 1, fp) != 1) {
			fclose(fp);
			return NULL;
		}
		ptr  = LittleEndian_getDW(entry_header, 0);
		size = LittleEndian_getDW(entry_header, 4);
		data = malloc(ptr + size);
		memcpy(data, entry_header, sizeof(entry_header));
		if (fread(data + sizeof(entry_header), ptr + size - sizeof(entry_header), 1, fp) != 1) {
			free(data);
			fclose(fp);
			return NULL;
		}
		fclose(fp);
	}
	
	dridata *dfile = calloc(1, sizeof(dridata));
	dfile->data_raw = data;    /* dri data header  */
	dfile->data = data + ptr;  /* real data */
	dfile->size = size;
	dfile->name = data + 16;
	dfile->a = d; /* archive file */
	return dfile;
}
