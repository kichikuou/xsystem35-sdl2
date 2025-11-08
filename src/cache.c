/*
 * cache.c  general cache manager
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
/* $Id: cache.c,v 1.5 2003/07/21 23:06:47 chikama Exp $ */

#include "config.h"
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include "portab.h"
#include "cache.h"

/* maximum cache size (in MB) */
#ifndef CACHE_TOTALSIZE
#define CACHE_TOTALSIZE 20
#endif

static int     totalsize;     /* total size in cache */
static int dummyfalse = 0; /* dummy in_use flag */
static int dummytrue  = 1;  /* dummy in_use flag */

/*
 * static methods
*/
static void remove_in_cache(cacher *id);

/*
 * Remove data in cache
 *   id: cache handler
*/
static void remove_in_cache(cacher *id) {
	cacheinfo *ip = id->top;
	cacheinfo *ic = ip->next;
	if (!ic)
		return;
	
	while(ic->next != NULL) {
		if (!*ic->in_use) {
			totalsize -= ic->size;
			ip->next = ic->next;
			id->free_(ic->data);
			free(ic);
		} else {
			ip = ic;
		}
		ic = ip->next;
	}
	return;
}

/* 
 * Create new cache object
 *   delcallback: callback function for delete cache data object
 *   return: new cache handler
*/
cacher *cache_new(void *delcallback) {
	cacher *c = calloc(1, sizeof(cacher));
	
	c->top = calloc(1, sizeof(cacheinfo));
	c->top->next = NULL;
	c->top->in_use = &dummytrue;
	c->free_ = delcallback;
	return c;
}

/*
 * Insert data to cache
 *   id    : cache handler
 *   key   : data key
 *   data  : data to be cached
 *   size  : data size
 *   in_use: in_use mark pointer, if in_use is nonzero, dont remove from cache
*/
void cache_insert(cacher *id, int key, void *data, int size, int *in_use) {
	cacheinfo *i = id->top;
	
	if (CACHE_TOTALSIZE <= (totalsize >> 20)) {
		remove_in_cache(id);
	}
	
	while(i->next != NULL) {
		i = i->next;
	}
	
	i->key = key;
	i->data = data;
	i->size = size;
	i->next = calloc(1, sizeof(cacheinfo));
	i->next->next = NULL;
	if (in_use) {
		i->in_use = in_use;
	} else {
		i->in_use = &dummyfalse;
	}
	totalsize += size;
}

/*
 * Search data in cache
 *   id : cache handler
 *   key: data search key
 *   return: pointer to cached data
*/
void *cache_foreach(cacher *id, int key) {
	cacheinfo *i = id->top;
	
	while(i != NULL) {
		if (i->key == key) {
			return i->data;
		}
		i = i->next;
	}
	return NULL;
}

void cache_clear(cacher *id) {
	cacheinfo *ic = id->top->next;
	cacheinfo *next;

	while (ic != NULL) {
		next = ic->next;
		if (ic->data) {
			totalsize -= ic->size;
			id->free_(ic->data);
		}
		free(ic);
		ic = next;
	}
	id->top->next = NULL;
}
