/*
 * alk.h  ALK archive manager
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
/* $Id: alk.h,v 1.2 2003/11/09 15:06:13 chikama Exp $ */

#ifndef __ALK_H__
#define __ALK_H__

#include "mmap.h"

struct alk_entry {
	void *data;
	uint32_t size;
};

typedef struct {
	mmap_t *mmap;
	int datanum;
	struct alk_entry entries[];
} alk_t;

alk_t *alk_new(const char *path);
void alk_free(alk_t *alk);

#endif /* __ALK_H__ */
