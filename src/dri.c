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

/*
 * static maethods
*/
static long getfilesize(FILE *fp);
static boolean filecheck (FILE *fp);
static void get_filemap(drifiles *d, FILE *fp);
static void get_fileptr(drifiles *d, FILE *fp, int disk);

/*
 * Get file size of FILE
 *   fp: FILE pointer
 *   return: filesize in byte
*/
static long getfilesize(FILE *fp) {
	fseek(fp, 0L, SEEK_END);
	return ftell(fp);
}

/*
 * Check whether dri file type or not
 *   fp: FILE pointer
 *   return: TRUE if it's dri file
*/
static boolean filecheck (FILE *fp) {
	BYTE b[6];
	int mapsize, ptrsize;
	long filesize;
	
	/* get filesize / 256  */
	filesize = (getfilesize(fp) + 255) >> 8;
	
	/* read top 6bytes */
	fseek(fp, 0L, SEEK_SET);
	fread(b, 1, 6, fp);
	
	/* get ptrsize and mapsize */
	ptrsize = LittleEndian_get3B(b, 0);
	mapsize = LittleEndian_get3B(b, 3) - ptrsize;
	
	/* must lager than 0 */
	if (ptrsize < 0 || mapsize < 0) return FALSE;
	
	/* must smaller than filesize */
	if (ptrsize > (int)filesize || mapsize > (int)filesize ) {
		return FALSE;
	}
	
	return TRUE;
}

/* 
 * Get file map
 *   d : drifile object
 *   fp: FILE object
*/
static void get_filemap(drifiles *d, FILE *fp) {
	BYTE b[6], *_b;
	int mapsize, ptrsize, i;
	
	/* read top 6bytes */
	fseek(fp, 0L, SEEK_SET);
	fread(b, 1, 6, fp);
	
	/* get ptrsize and mapsize */
	ptrsize = LittleEndian_get3B(b, 0);
	mapsize = LittleEndian_get3B(b, 3) - ptrsize;
	
	/* allocate read buffer */
	_b = malloc(sizeof(char) * (mapsize << 8));
	
	/* read filemap */
	fseek(fp, ptrsize << 8L , SEEK_SET);
	fread(_b, 256, mapsize, fp);
	
	/* get max file number from mapdata */
	d->maxfile = (mapsize << 8) / 3;
	
	/* map of disk */
	d->map_disk = malloc(sizeof(char ) * d->maxfile);
	/* map of data in disk */ 
	d->map_ptr  = malloc(sizeof(short) * d->maxfile);
	
	for (i = 0; i < d->maxfile; i++) {	
		/* map_disk[?] and map_ptr[?] are from 0 */
		*(d->map_disk + i) = _b[i * 3] - 1;
		*(d->map_ptr  + i) = LittleEndian_getW(_b, i * 3 + 1) - 1;
	}
	
	free(_b);
	return;
}

/*
 * Get data pointer in file
 *   d   : drifile object
 *   fp  : FILE object
 *   disk: no in drifile object 
*/
static void get_fileptr(drifiles *d, FILE *fp, int disk) {
	char b[6], *_b;
	int ptrsize, filecnt, i;

	/* read top 6bytes */
	fseek(fp, 0L, SEEK_SET);
	fread(b, 1, 6, fp);

	/* get pinter size */
	ptrsize = LittleEndian_get3B(b,0);
	
	/* estimate file number in file */
	filecnt = (ptrsize << 8) / 3;
	
	/* allocate read buffer */
	_b = malloc(sizeof(char) * (ptrsize << 8));
	
	/* read pointers */
	fseek(fp, 0L, SEEK_SET);
	fread(_b, 256, ptrsize, fp);
	
	/* allocate pointers buffer */
	d->fileptr[disk] = calloc(filecnt, sizeof(int));
	
	/* store pointers */
	for (i = 0; i < filecnt - 1; i++) {
		*(d->fileptr[disk] + i) = (LittleEndian_get3B(_b, i * 3 + 3) << 8);
	}
	
	free(_b);
	return;
}

drifiles *dri_init(const char **file, int cnt, boolean use_mmap) {
	drifiles *d = calloc(1, sizeof(drifiles));
	FILE *fp;
	int i;
	boolean gotmap = FALSE;
#ifndef HAVE_MEMORY_MAPPED_FILE
	use_mmap = FALSE;
#endif

	for (i = 0; i < cnt; i++) {
		if (file[i] == NULL) continue;
		/* open check */
		if (NULL == (fp = fopen(file[i], "rb"))) {
			SYSERROR("File %s is not found", file[i]);
		}
		/* check is drifile or noe */
		if (!filecheck(fp)) {
			SYSERROR("File %s is not dri file", file[i]);
		}
		/* get file map */
		if (!gotmap) {
			get_filemap(d, fp);
			gotmap = TRUE;
		}
		/* get pointer */
		get_fileptr(d, fp, i);
		/* copy filenme */
		d->fnames[i] = strdup(file[i]);
		/* close */
		fclose(fp);

		/* mmap */
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

/*
 * Get data 
 *   d : drifile object
 *   no: drifile no ( >= 0 )
 *   return: dridata obhect
 */
dridata *dri_getdata(drifiles *d, int no) {
	BYTE *data;
	dridata *dfile;
	int disk, ptr, dataptr, dataptr2, size;
	
	/* check no is lager than files which contains */
	if (no > d->maxfile) return NULL;
	
	/* check disk & ptr are negative, if negative, file does not exist */
	disk = d->map_disk[no];
	ptr  = d->map_ptr[no];
	if (disk < 0 || ptr < 0) return NULL;
	
	/* no file registered */
	if (d->fileptr[disk] == NULL) return NULL;
	
	/* get pointer in file and size */
	dataptr = *(d->fileptr[disk] + ptr);
	dataptr2 = *(d->fileptr[disk] + ptr + 1);
	if (dataptr == 0 || dataptr2 == 0) return NULL;
	
	/* get data top */
	if (d->mmapped) {
		data = d->mmap[disk]->addr + dataptr;
	} else {
		int readsize = dataptr2 - dataptr;
		FILE *fp;
		data = malloc(sizeof(char) * readsize);
		fp = fopen(d->fnames[disk], "rb");
		fseek(fp, dataptr, SEEK_SET);
		fread(data, 1, readsize, fp);
		fclose(fp);
	}
	
	/* get real data and size */
	ptr  = LittleEndian_getDW(data, 0);
	size = LittleEndian_getDW(data, 4);
	
	dfile = calloc(1, sizeof(dridata));
	dfile->data_raw = data;    /* dri data header  */
	dfile->data = data + ptr;  /* real data */
	dfile->size = size;
	dfile->a = d; /* archive file */
	return dfile;
}
