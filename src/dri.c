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

static bool read_index(int disk, drifiles *d, FILE *fp) {
	fseek(fp, 0L, SEEK_END);
	int filesize = ftell(fp);

	// Get ptrsize and mapsize
	uint8_t hdr[6];
	fseek(fp, 0L, SEEK_SET);
	if (fread(hdr, 6, 1, fp) != 1)
		return false;
	int ptrsize = LittleEndian_get3B(hdr, 0) << 8;
	int mapsize = (LittleEndian_get3B(hdr, 3) << 8) - ptrsize;
	if (ptrsize <= 0 || mapsize <= 0 || ptrsize + mapsize > filesize)
		return false;

	// Read the pointer table and the link table
	uint8_t *ptbl = malloc(ptrsize + mapsize);
	memcpy(ptbl, hdr, 6);
	if (fread(ptbl + 6, ptrsize + mapsize - 6, 1, fp) != 1) {
		free(ptbl);
		return false;
	}
	uint8_t *ltbl = ptbl + ptrsize;

	// (Re)allocate the index buffers
	int nr_files = mapsize / 3;
	if (d->nr_files < nr_files) {
		d->disk = realloc(d->disk, nr_files);
		d->offset = realloc(d->offset, sizeof(uint32_t) * nr_files);
		memset(d->disk + d->nr_files, 0, nr_files - d->nr_files);
		memset(d->offset + d->nr_files, 0, sizeof(uint32_t) * (nr_files - d->nr_files));
		d->nr_files = nr_files;
	}

	// Parse the index
	for (int i = 0; i < nr_files; i++) {
		if (disk != ltbl[i * 3] - 1)
			continue;
		if (d->maxno < i)
			d->maxno = i;
		d->disk[i] = ltbl[i * 3];
		int ptr = LittleEndian_getW(ltbl, i * 3 + 1);
		d->offset[i] = LittleEndian_get3B(ptbl, ptr * 3) << 8;
	}
	free(ptbl);
	return true;
}

drifiles *dri_init(const char **file, int cnt, boolean use_mmap) {
	drifiles *d = calloc(1, sizeof(drifiles));
#ifndef HAVE_MEMORY_MAPPED_FILE
	use_mmap = FALSE;
#endif

	for (int i = 0; i < cnt; i++) {
		if (!file[i])
			continue;

		FILE *fp = fopen(file[i], "rb");
		if (!fp)
			SYSERROR("%s: %s", file[i], strerror(errno));
		if (!read_index(i, d, fp)) {
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
				use_mmap = d->mmapped = FALSE;
				i = 0; /* retry */
			} else {
				d->mmap[i] = m;
				d->mmapped = TRUE;
			}
		}
	}
	return d;
}

dridata *dri_getdata(drifiles *d, int no) {
	if (no < 0 || no >= d->nr_files || !d->disk[no] || !d->offset[no])
		return NULL;
	int disk = d->disk[no] - 1;

	uint8_t *data;
	int ptr, size;
	if (d->mmapped) {
		data = d->mmap[disk]->addr + d->offset[no];
		ptr  = LittleEndian_getDW(data, 0);
		size = LittleEndian_getDW(data, 4);
	} else {
		FILE *fp = fopen(d->fnames[disk], "rb");
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
