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

/* drifiles object */
static drifiles *dri[DRIFILETYPEMAX];

/* cache handler for dri file */
static cacher *cacheid;

/*
 * static maethods
*/
static void ald_free(dridata *dfile);

/*
 * free dridata 
 *   dfile: dridata to be free
*/
static void ald_free(dridata *dfile) {
	free(dfile->data_raw);
	free(dfile);
}

/*
 * load dri data
 *   type: data type
 *   no  : file no ( >= 0 )
 *   return: loaded dridata object
*/
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
	if (NULL == (ddata = (dridata *)cache_foreach(cacheid, (type << 16) + no))) {
		ddata = dri_getdata(dri[type], no);
		if (ddata != NULL) {
			ddata->refcnt = 0;
			cache_insert(cacheid, (type << 16) + no, (void *)ddata, ddata->size, &(ddata->refcnt));
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
void ald_freedata(dridata *data) {
	if (data == NULL) return;
	
	if (data->a->mmapped) {
		free(data);
	} else {
		data->refcnt--;
	}
}

/*
 * Initilize ald manager
 *   type: data type
 *   file: file name array
 *   cnt : number in file name array
 *   mmap: mmap file or not
*/
void ald_init(int type, const char **file, int cnt, boolean mmap) {
	if (type >= DRIFILETYPEMAX || cnt <= 0)
		return;
	dri[type] = dri_init(file, cnt, mmap);
	if (!dri[type]->mmapped) {
		cacheid = cache_new(ald_free);
	}
}
