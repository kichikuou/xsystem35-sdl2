/*
 * alk.c  ALK archive manager
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
 * @version 1.0     01/11/29 initial version
*/
/* $Id: alk.c,v 1.2 2003/01/12 10:48:50 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <glib.h>

#include "portab.h"
#include "system.h"
#include "LittleEndian.h"
#include "alk.h"

alk_t *alk_new(char *path) {
	alk_t *alk;
	int i, fd;
	char *adr;
	struct stat sbuf;
	
	if (0 > (fd = open(path, O_RDONLY))) {
		WARNING("open: %s\n", strerror(errno));
		return NULL;
	}

	if (0 > fstat(fd, &sbuf)) {
		WARNING("fstat: %s\n", strerror(errno));
		close(fd);
		return NULL;
	}
	
	if (MAP_FAILED == (adr = mmap(0, sbuf.st_size, PROT_READ, MAP_SHARED, fd, 0))) {
		WARNING("mmap: %s\n", strerror(errno));
		close(fd);
		return NULL;
	}
	
	if (0 != strncmp(adr, "ALK0", 4)) {
		WARNING("mmap: %s\n", strerror(errno));
		munmap(adr, sbuf.st_size);
		close(fd);
		return NULL;
	}		

	alk = g_new0(alk_t, 1);
	alk->mapadr = adr;
	alk->size = sbuf.st_size;
	alk->fd = fd;
	
	alk->datanum = LittleEndian_getDW(adr, 4);
	alk->offset = g_new(int, alk->datanum);
	
	for (i = 0; i < alk->datanum; i++) {
		alk->offset[i] = LittleEndian_getDW(adr, 8 + i * 8);
	}
	
	return alk;
}

int alk_free(alk_t *alk) {
	if (alk == NULL) return OK;

	munmap(alk->mapadr, alk->size);
	close(alk->fd);
	g_free(alk->offset);
	g_free(alk);

	return OK;
}

char *alk_get(alk_t *alk, int no) {
	if (no >= alk->datanum) return NULL;
	
	return alk->mapadr + alk->offset[no];
}
