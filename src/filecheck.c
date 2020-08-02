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
#ifdef _WIN32
#include <windows.h>
#undef min
#undef max
#endif

#include "filecheck.h"

static char *saveDataPath;

static char *get_fullpath(const char* dir, const char *filename) {
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
void fc_init(const char *name) {
	saveDataPath = strdup(name);
}

#ifdef _WIN32

FILE *fopen_utf8(const char *path_utf8, char type) {
	wchar_t wpath[PATH_MAX + 1];
	if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, path_utf8, -1, wpath, PATH_MAX + 1))
		return NULL;
	return _wfopen(wpath, type == 'r' ? L"rb" : L"wb");
}

FILE *fc_open(const char *fname_utf8, char type) {
	char *path = get_fullpath(saveDataPath, fname_utf8);
	FILE *fp = fopen_utf8(path, type);
	if (!fp && type == 'r') {
		fp = fopen_utf8(fname_utf8, type);
	}
	free(path);
	return fp;
}

#else // !_WIN32

static char *fc_search(const char *fname_utf8, const char *dir) {
	DIR *d = opendir(dir);
	if (d == NULL)
		return NULL;

	char *found = NULL;
	struct dirent *entry;
	while ((entry = readdir(d)) != NULL) {
		if (strcasecmp(fname_utf8, entry->d_name) == 0) {
			found = get_fullpath(dir, entry->d_name);
			break;
		}
	}
	closedir(d);
	return found;
}

FILE *fc_open(const char *fname_utf8, char type) {
	char *fullpath = fc_search(fname_utf8, saveDataPath);
	if (!fullpath) {
		if (type == 'r') {
			fullpath = fc_search(fname_utf8, ".");
			if (!fullpath)
				return NULL;
		} else {
			fullpath = get_fullpath(saveDataPath, fname_utf8);
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
#endif // _WIN32

void fc_backup_oldfile(const char *filename) {
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
