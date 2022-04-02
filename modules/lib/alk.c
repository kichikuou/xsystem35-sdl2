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
#include <stdlib.h>

#include "portab.h"
#include "system.h"
#include "LittleEndian.h"
#include "alk.h"

alk_t *alk_new(const char *path) {
	mmap_t *m = map_file(path);
	
	if (0 != strncmp(m->addr, "ALK0", 4)) {
		WARNING("%s: not an ALK file\n", path);
		unmap_file(m);
		return NULL;
	}		

	int nr_entries = LittleEndian_getDW(m->addr, 4);

	alk_t *alk = malloc(sizeof(alk_t) + sizeof(struct alk_entry) * nr_entries);
	alk->mmap = m;
	alk->datanum = nr_entries;

	for (int i = 0; i < nr_entries; i++) {
		alk->entries[i].data = (uint8_t *)m->addr + LittleEndian_getDW(m->addr, 8 + i * 8);
		alk->entries[i].size = LittleEndian_getDW(m->addr, 8 + i * 8 + 4);
	}
	return alk;
}

void alk_free(alk_t *alk) {
	unmap_file(alk->mmap);
	free(alk);
}
