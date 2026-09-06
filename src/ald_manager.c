/*
 * ald_manager.c  dri file manager
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
/* $Id: ald_manager.c,v 1.3 2001/05/08 05:36:07 chikama Exp $ */

#include <stdlib.h>
#include "portab.h"
#include "dri.h"
#include "cache.h"
#include "ald_manager.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

/* drifiles object */
static drifiles *dri[DRIFILETYPEMAX];

static Cache *dri_cache;

#ifndef ALD_CACHE_SIZE
#define ALD_CACHE_SIZE (10 << 20)
#endif

static uint32_t int_hash(const void *key) {
	return (uint32_t)*(const int *)key;
}

static bool int_equal(const void *a, const void *b) {
	return *(const int *)a == *(const int *)b;
}

/*
 * free dridata 
 *   dfile: dridata to be free
*/
static void ald_free(void *data) {
	dridata *dfile = data;
	free(dfile->data_raw);
	free(dfile);
}

static bool ald_is_pinned(const void *data) {
	return ((const dridata *)data)->refcnt != 0;
}

bool ald_is_linked(DRIFILETYPE type, int no) {
	if (type >= DRIFILETYPEMAX || !dri[type])
		return false;
	return dri_is_linked(dri[type], no);
}

bool ald_exists(DRIFILETYPE type, int no) {
	if (type >= DRIFILETYPEMAX || !dri[type])
		return false;
	return dri_exists(dri[type], no);
}

/*
 * load dri data
 *   type: data type
 *   no  : file no ( >= 0 )
 *   return: loaded dridata object
*/
EMSCRIPTEN_KEEPALIVE
dridata *ald_getdata(DRIFILETYPE type, int no) {
	dridata *ddata;
	
	/* check wrong request number */
	if (no < 0) return NULL;
	
	/* check wrong type */
	if (type >= DRIFILETYPEMAX) return NULL;
	
	/* check uninitilized data */
	if (dri[type] == NULL) return NULL;
	
	/* if mmapped */
	if (dri[type]->mmapped) return dri_getdata(dri[type], no);
	
	/* not mmapped */
	int key = (type << 16) + no;
	if (NULL == (ddata = cache_get(dri_cache, &key))) {
		ddata = dri_getdata(dri[type], no);
		if (ddata != NULL) {
			ddata->refcnt = 0;
			ddata->cached = cache_insert(dri_cache, &key, ddata, ddata->size) == CACHE_INSERT_OK;
		}
	}
	if (ddata != NULL)
		ddata->refcnt++;
	
	return ddata;
}

/*
 * free dri object
 *   data: object to be free
 */
EMSCRIPTEN_KEEPALIVE
void ald_freedata(dridata *data) {
	if (data == NULL) return;
	
	if (data->a->mmapped) {
		free(data);
	} else {
		data->refcnt--;
		if (!data->cached && data->refcnt == 0)
			ald_free(data);
	}
}

void ald_init(int type, const char **file, int cnt, bool use_mmap) {
	if (type >= DRIFILETYPEMAX || cnt <= 0)
		return;
	dri[type] = dri_init(file, cnt, use_mmap);
	if (!dri[type]->mmapped && !dri_cache) {
		CacheOps ops = {
			.key_size = sizeof(int),
			.hash = int_hash,
			.equal = int_equal,
			.destroy = ald_free,
			.is_pinned = ald_is_pinned,
		};
		dri_cache = cache_new((size_t)ALD_CACHE_SIZE, &ops);
	}
}

int ald_get_maxno(DRIFILETYPE type) {
	if (type >= DRIFILETYPEMAX || !dri[type])
		return 0;
	return dri[type]->maxno;
}
