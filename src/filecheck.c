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

static char *saveDataPath;
static boolean newfile_kanjicode_utf8 = TRUE;

static char *get_fullpath(const char* dir, char *filename) {
	char *fn = malloc(strlen(filename) + strlen(dir) + 3);
	if (fn == NULL) {
		return NULL;
	}
	strcpy(fn, dir);
	strcat(fn, "/");
	strcat(fn, filename);
	return fn;
}

/* list up file in current directory */
/*   name : save/load directory      */
void fc_init(char *name) {
	saveDataPath = strdup(name);
}

static char *fc_search(const char *fname_sjis, const char *dir) {
	DIR *d = opendir(dir);
	if (d == NULL)
		return NULL;

	BYTE *fname_utf = sjis2lang(fname_sjis);
	char *found = NULL;
	struct dirent *entry;
	while ((entry = readdir(d)) != NULL) {
		if (strcasecmp(fname_sjis, entry->d_name) == 0 ||
			strcasecmp(fname_utf, entry->d_name) == 0) {
			found = get_fullpath(dir, entry->d_name);
			break;
		}
	}
	closedir(d);
	free(fname_utf);
	return found;
}

FILE *fc_open(char *filename, char type) {
	char *fullpath = fc_search(filename, saveDataPath);
	if (fullpath == NULL) {
		if (type == 'r') {
#ifdef __EMSCRIPTEN__
			fullpath = fc_search(filename, ".");
#endif
			if (fullpath == NULL)
				return NULL;
		} else {
			if (newfile_kanjicode_utf8) {
				char *fc = sjis2lang(filename);
				fullpath = get_fullpath(saveDataPath, fc);
				free(fc);
			}
			else
				fullpath = get_fullpath(saveDataPath, filename);
		}
	}

	FILE *fp;
	if (type == 'w') {
		fc_backup_oldfile(fullpath);
		fp = fopen(fullpath, "wb");
	} else {
		fp = fopen(fullpath, "rb");
	}
	free(fullpath);
	return fp;
}

void fc_backup_oldfile(char *filename) {
#ifndef __EMSCRIPTEN__
	char *newname;
	
	if (!filename) return;
	newname = malloc(strlen(filename) + 3);
	
	strcpy(newname, filename);
	strcat(newname, ".");
	rename(filename, newname);
	
	free(newname);
#endif
}

/* QE で新規ファイルをセーブする時のファイル名の漢字コード */
void fc_set_default_kanjicode(int c) {
	if (c == 0) {
		newfile_kanjicode_utf8 = TRUE;
	} else {
		newfile_kanjicode_utf8 = FALSE;
	}
}		
