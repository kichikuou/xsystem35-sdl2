/*
 * filecheck.c  save/load file existance and kanjicode check 
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
/* $Id: filecheck.c,v 1.5 2006/04/21 16:40:48 chikama Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

#include "filecheck.h"
#include "utfsjis.h"

struct fnametable {
	char *realname;
	char *transname;
};

#define FILEMAX 100 /* ???? */
static struct fnametable tbl[FILEMAX];
static int fnametable_cnt;
static char *saveDataPath;
static boolean initilized = FALSE;
static boolean newfile_kanjicode_utf8 = TRUE;

static char *get_fullpath(char *filename) {
	char *fn = malloc(strlen(filename) + strlen(saveDataPath) + 3);
	if (fn == NULL) {
		return NULL;
	}
	strcpy(fn, saveDataPath);
	strcat(fn, "/");
	strcat(fn, filename);
	return fn;
}

/* list up file in current directory */
/*   name : save/load directory      */
void fc_init(char *name) {
	DIR *dir;
	struct dirent *entry;
	int c = 0;
	
	initilized = FALSE;
	
	saveDataPath = strdup(name);
	if (NULL == (dir = opendir(name))) {
		return;
	}
	
	while(0 < (entry = readdir(dir))) {
		if (c >= FILEMAX) {
			fprintf(stderr, "Over " "FILEMAX" "files in savefile directry\n");
			break;
		}
		tbl[c].realname = strdup(entry->d_name);
		tbl[c].transname = strdup(entry->d_name);
		sjis_toupper(tbl[c].transname);
		c++;
	}
	fnametable_cnt = c;
	closedir(dir);
	initilized = TRUE;
}

/* req must sjis */
static char *fc_search(char *req) {
	int i;
	BYTE *b;
	
	if (!initilized) return req;
	
	for (i = 0; i < fnametable_cnt; i++) {
		/* match exeactly */
		if (0 == strcmp(req, tbl[i].realname)) return req;

		/* capital match */
		b = sjis_toupper2(req);
		if (0 == strcmp(b, tbl[i].transname)) {
			free(b);
			return tbl[i].realname;
		}
		
		/* utf-8 match */
		b = sjis2lang(req);
		sjis_toupper(b);
		if (0 == strcmp(b, tbl[i].transname)) {
			free(b);
			return tbl[i].realname;
		} 
		free(b);
	}
	return NULL;
}

/* add new file to entry */
static char *fc_add(char *req) {
	BYTE *b;
	
	if (!initilized) return req;
	
	if (fnametable_cnt >= FILEMAX) {
		fprintf(stderr, "Over " "FILEMAX" "files in savefile directry\n");
		return req;
	}

	if (newfile_kanjicode_utf8) {
		tbl[fnametable_cnt].realname = sjis2lang(req);
	} else {
		tbl[fnametable_cnt].realname = strdup(req);
	}

	b = sjis_toupper2(req);
	tbl[fnametable_cnt].transname = b;
	
	b = tbl[fnametable_cnt].realname;
	fnametable_cnt++;
	return b;
}

FILE *fc_open(char *filename, char type) {
	char *fc = fc_search(filename);
	char *fullpath;
	FILE *fp;
	
	if (fc == NULL) { /* if file does not exist */
		if (type == 'r') return NULL;
		fc = fc_add(filename);
	}
	fullpath = get_fullpath(fc);
	
	if (type == 'w') {
		fc_backup_oldfile(fullpath);
		fp = fopen(fullpath, "w");
	} else {
		fp = fopen(fullpath, "r");
	}
	free(fullpath);
	return fp;
}

void fc_backup_oldfile(char *filename) {
	char *newname;
	
	if (!filename) return;
	newname = malloc(strlen(filename) + 3);
	
	strcpy(newname, filename);
	strcat(newname, ".");
	rename(filename, newname);
	
	free(newname);
}

/* QE で新規ファイルをセーブする時のファイル名の漢字コード */
void fc_set_default_kanjicode(int c) {
	if (c == 0) {
		newfile_kanjicode_utf8 = TRUE;
	} else {
		newfile_kanjicode_utf8 = FALSE;
	}
}		
