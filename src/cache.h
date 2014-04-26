/*
 * cache.h  general cache manager
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
/* $Id: cache.h,v 1.2 2003/07/21 23:06:47 chikama Exp $ */

#ifndef __CASHE__
#define __CASHE__

#include "portab.h"

/* cache controlr infomartion */
struct _cacheinfo {
	int key;            /* key of data */
	int refcnt;         /* reference count */
	int size;           /* data size */
	struct _cacheinfo *next; /* next data */
	boolean *in_use;    /* if *in_use is TRUE, dont remove from cache */
	void *data;         /* real data */
};
typedef struct _cacheinfo cacheinfo;

/* cache handler */
struct _cacher {
	int id;                 /* id of cache object */
	void (*free_)(void *);   /* free data callback */
	struct _cacheinfo *top; /* pointer to data */
};
typedef struct _cacher cacher;

extern cacher *cache_new(void *delcallback);
extern void    cache_insert(cacher *id, int key, void *data, int size, boolean *in_use);
extern void   *cache_foreach(cacher *id, int key);

#endif /* !__CASHE__ */
