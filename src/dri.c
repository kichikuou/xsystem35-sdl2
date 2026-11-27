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
#include <limits.h>
#include <errno.h>
#include "portab.h"
#include "system.h"
#include "LittleEndian.h"
#include "dri.h"

static bool read_index(int volume, drifiles *d, FILE *fp) {
	if (fseek(fp, 0L, SEEK_END) != 0)
		return false;
	long fsize = ftell(fp);
	if (fsize < 6 || fsize > INT_MAX)
		return false;
	uint32_t filesize = (uint32_t)fsize;

	// Get the size of offsets table and link table.
	uint8_t hdr[6];
	if (fseek(fp, 0L, SEEK_SET) != 0 || fread(hdr, 6, 1, fp) != 1)
		return false;
	uint32_t ofssize = (uint32_t)LittleEndian_get3B(hdr, 0) << 8;
	uint32_t linkend = (uint32_t)LittleEndian_get3B(hdr, 3) << 8;
	if (ofssize < 6 || linkend <= ofssize || linkend > filesize)
		return false;
	uint32_t linksize = linkend - ofssize;

	// Read the link table into a local buffer first so a failed read leaves `d` untouched.
	uint8_t *new_link = NULL;
	uint32_t *new_offset = NULL;
	int nr_files = d->nr_files;
	const uint8_t *link = d->link;
	if (!link) {
		nr_files = (int)(linksize / 3);
		if (nr_files <= 0)
			return false;
		new_link = calloc(1, linksize);
		new_offset = calloc(sizeof(uint32_t), nr_files);
		if (!new_link || !new_offset) {
			free(new_link);
			free(new_offset);
			return false;
		}
		if (fseek(fp, (long)ofssize, SEEK_SET) != 0 || fread(new_link, linksize, 1, fp) != 1) {
			free(new_link);
			free(new_offset);
			return false;
		}
		link = new_link;
	}

	// Read the offsets table.
	uint8_t *otbl = malloc(ofssize);
	uint32_t *vol_offsets = calloc(sizeof(uint32_t), nr_files);
	if (!otbl || !vol_offsets) {
		free(otbl);
		free(vol_offsets);
		free(new_link);
		free(new_offset);
		return false;
	}
	if (fseek(fp, 0L, SEEK_SET) != 0 || fread(otbl, ofssize, 1, fp) != 1) {
		free(otbl);
		free(vol_offsets);
		free(new_link);
		free(new_offset);
		return false;
	}

	int new_maxno = d->maxno;
	for (int i = 0; i < nr_files; i++) {
		if (link[i * 3] != volume)
			continue;
		if (new_maxno < i)
			new_maxno = i;
		uint32_t offset_index = (uint32_t)LittleEndian_getW(link, i * 3 + 1);
		if (offset_index >= ofssize / 3) {
			free(otbl);
			free(vol_offsets);
			free(new_link);
			free(new_offset);
			return false;
		}
		uint32_t off = (uint32_t)LittleEndian_get3B(otbl, (int)(offset_index * 3)) << 8;
		if (off != 0) {
			if (off > filesize || filesize - off < 8) {
				free(otbl);
				free(vol_offsets);
				free(new_link);
				free(new_offset);
				return false;
			}
			uint8_t eh[8];
			if (fseek(fp, (long)off, SEEK_SET) != 0 || fread(eh, sizeof(eh), 1, fp) != 1) {
				free(otbl);
				free(vol_offsets);
				free(new_link);
				free(new_offset);
				return false;
			}
			uint32_t ptr = LittleEndian_getDW(eh, 0);
			uint32_t size = LittleEndian_getDW(eh, 4);
			if (ptr < 8 || (uint64_t)ptr + (uint64_t)size > (uint64_t)(filesize - off)) {
				free(otbl);
				free(vol_offsets);
				free(new_link);
				free(new_offset);
				return false;
			}
		}
		vol_offsets[i] = off;
	}

	// Commit validated volume index atomically.
	if (new_link) {
		d->nr_files = nr_files;
		d->link = new_link;
		d->offset = new_offset;
	}
	for (int i = 0; i < d->nr_files; i++) {
		if (d->link[i * 3] == volume)
			d->offset[i] = vol_offsets[i];
	}
	d->maxno = new_maxno;
	free(otbl);
	free(vol_offsets);
	return true;
}

#ifdef __SWITCH__
bool dri_validate(const char **files, int count, bool scenario)
{
	drifiles index = {0};
	bool valid = true;
	for (int i = 0; i < count; i++) {
		if (!files[i])
			continue;
		FILE *fp = fopen(files[i], "rb");
		if (!fp) {
			valid = false;
			break;
		}
		bool indexed = read_index(i + 1, &index, fp);
		fclose(fp);
		if (!indexed && i == 0) {
			valid = false;
			break;
		}
	}
	valid = valid && (!scenario || dri_exists(&index, 0));
	free(index.link);
	free(index.offset);
	return valid;
}
#endif

drifiles *dri_init(const char **file, int cnt, bool use_mmap) {
	drifiles *d = calloc(1, sizeof(drifiles));
#if !defined(HAVE_MEMORY_MAPPED_FILE) || defined(__EMSCRIPTEN__) || defined(__SWITCH__)
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

drifiles *dri_init_optional(const char *file, bool use_mmap) {
	if (!file || !file[0])
		return NULL;
	FILE *fp = fopen(file, "rb");
	if (!fp)
		return NULL;
	drifiles *d = calloc(1, sizeof(drifiles));
	if (!d) {
		fclose(fp);
		return NULL;
	}
#if !defined(HAVE_MEMORY_MAPPED_FILE) || defined(__EMSCRIPTEN__) || defined(__SWITCH__)
	use_mmap = false;
#endif
	if (!read_index(1, d, fp)) {
		WARNING("%s: ignoring invalid overlay ALD file", file);
		fclose(fp);
		free(d->link);
		free(d->offset);
		free(d);
		return NULL;
	}
	d->fnames[0] = strdup(file);
	fclose(fp);
	if (use_mmap) {
		mmap_t *m = map_file(file);
		if (m) {
			d->mmap[0] = m;
			d->mmapped = true;
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
	sys_show_message_box(MESSAGEBOX_WARNING, "xsystem35", msgbuf);
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
		if (!d->mmap[volume - 1])
			return NULL;
		data = d->mmap[volume - 1]->addr + d->offset[no];
		ptr  = LittleEndian_getDW(data, 0);
		size = LittleEndian_getDW(data, 4);
	} else {
		if (!d->fnames[volume - 1])
			return NULL;
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
