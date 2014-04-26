/*
 * dri.h: dri loader
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
/* $Id: dri.h,v 1.8 2001/04/02 21:00:44 chikama Exp $ */

#ifndef __DRI__
#define __DRI__

#include "portab.h"

#define DRIFILEMAX 255     /* maximum file number for one data type */
#define DRIDATAMAX 65535   /* maximum file number in one file */

struct _drifiles {
	/* for mmap */
	boolean     mmapped;
	void        *mmapadr[DRIFILEMAX];
	/* for file access */
	char        *fnames[DRIFILEMAX];
	/* max file number in files */
	int         maxfile;
	/* file mapping */
	char        *map_disk;
	short       *map_ptr;
	/* pointers in file */
	int         *fileptr[DRIFILEMAX];
};
typedef struct _drifiles drifiles;

struct _dridata {
	int     size;      /* size of real data */
	char    *data_raw; /* dri header pointer */
	char    *data;     /* real data */
	char    *name;     /* not used */
	boolean in_use;    /* dont remove from cache if TRUE */
	drifiles *a;       /* archive file obj */
};
typedef struct _dridata dridata;

extern drifiles *dri_init(char **file, int cnt, boolean mmapping);
extern dridata  *dri_getdata(drifiles *d, int no);

#endif /* !__DRI__ */
