/*
 * gameresource.c  Game Resource (*.gr) file
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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "gameresource.h"
#include "system.h"

static void storeDataName(GameResource *gr, int type, int no, char *src) {
	gr->game_fname[type][no] = strdup(src);
	gr->cnt[type] = max(no + 1, gr->cnt[type]);
}

static void storeSaveName(GameResource *gr, int no, char *src) {
	char *home_dir = "";
	char *path = NULL;
	
	if (*src == '~') {
		home_dir = getenv("HOME");
		if (NULL == (path = malloc(strlen(home_dir) + strlen(src) + 1))) {
			NOMEMERR();
		}
		sprintf(path, "%s%s", home_dir, src+1);
		gr->save_fname[no] = strdup(path);
		src = path;
	} else {
		gr->save_fname[no] = strdup(src);
	}
	
	if (no == 0) {
		char *b = strrchr(src, '/');
		if (b == NULL) {
			SYSERROR("Illigal save filename %s\n", src);
		}
		*b = '\0';
		gr->save_path = strdup(src);
	}
	if (path) free(path);
}

boolean initGameResourceFromDir(GameResource *gr, DIR *dir, struct dirent *(*p_readdir)(DIR *)) {
	char cwd[256], path[256];
	getcwd(cwd, 255);

	memset(gr, 0, sizeof(GameResource));

	struct dirent* d;
	while ((d = p_readdir(dir))) {
		char *filename = d->d_name;
		int len = strlen(filename);
		sprintf(path, "%s/%s", cwd, filename);
		if (strcasecmp(filename, "adisk.ald") == 0) {
			storeDataName(gr, DRIFILE_SCO, 0, path);
		} else if (strcasecmp(filename, "system39.ain") == 0) {
			gr->ain = strdup(filename);
		} else if (len >= 6 && strcasecmp(filename + len - 4, ".ald") == 0) {
			int dno = toupper(filename[len - 5]) - 'A';
			if (dno < 0 || dno >= DRIFILEMAX) continue;
			switch (toupper(filename[len - 6])) {
			case 'S':
				storeDataName(gr, DRIFILE_SCO, dno, path);
				break;
			case 'G':
				storeDataName(gr, DRIFILE_CG, dno, path);
				break;
			case 'W':
				storeDataName(gr, DRIFILE_WAVE, dno, path);
				break;
			case 'M':
				storeDataName(gr, DRIFILE_MIDI, dno, path);
				break;
			case 'D':
				storeDataName(gr, DRIFILE_DATA, dno, path);
				break;
			case 'R':
				storeDataName(gr, DRIFILE_RSC, dno, path);
				break;
			case 'B':
				storeDataName(gr, DRIFILE_BGM, dno, path);
				break;
			}
		}
	}
	for (int i = 0; i < SAVE_MAXNUMBER; i++) {
		sprintf(path, "%s/%c%s", cwd, 'a' + i, "sleep.asd");
		storeSaveName(gr, i, path);
	}

	return (gr->cnt[DRIFILE_SCO] > 0) ? TRUE : FALSE;
}

static void trimRight(char *str) {
	for (char *p = str + strlen(str) - 1; p >= str && isspace(*p); p--)
		*p = '\0';
}

static boolean initGameResourceFromFile(GameResource *gr, FILE *fp, const char *gr_fname) {
	int linecnt = 0, dno;
	char line[256];
	char key[256], path[256];

	memset(gr, 0, sizeof(GameResource));

	while (fgets(line, 255, fp) != NULL) {
		linecnt++;
		if (line[0] == '#') continue;
		path[0] = '\0';
		sscanf(line, "%s %[^\n]", key, path);
		if (path[0] == '\0') continue;
		trimRight(path);
		if (0 == strncmp(key, "Scenario", 8)) {
			dno = key[8] - 'A';
			if (dno < 0 || dno >= DRIFILEMAX) goto errexit;
			storeDataName(gr, DRIFILE_SCO, dno, path);
		} else if (0 == strncmp(key, "Graphics", 8)) {
			dno = key[8] - 'A';
			if (dno < 0 || dno >= DRIFILEMAX) goto errexit;
			storeDataName(gr, DRIFILE_CG, dno, path);
		} else if (0 == strncmp(key, "Wave", 4)) {
			dno = key[4] - 'A';
			if (dno < 0 || dno >= DRIFILEMAX) goto errexit;
			storeDataName(gr, DRIFILE_WAVE, dno, path);
		} else if (0 == strncmp(key, "Midi", 4)) {
			dno = key[4] - 'A';
			if (dno < 0 || dno >= DRIFILEMAX) goto errexit;
			storeDataName(gr, DRIFILE_MIDI, dno, path);
		} else if (0 == strncmp(key, "Data", 4)) {
			dno = key[4] - 'A';
			if (dno < 0 || dno >= DRIFILEMAX) goto errexit;
			storeDataName(gr, DRIFILE_DATA, dno, path);
		} else if (0 == strncmp(key, "Resource", 8)) {
			dno = key[8] - 'A';
			if (dno < 0 || dno >= DRIFILEMAX) goto errexit;
			storeDataName(gr, DRIFILE_RSC, dno, path);
		} else if (0 == strncmp(key, "BGM", 3)) {
			dno = key[3] - 'A';
			if (dno < 0 || dno >= DRIFILEMAX) goto errexit;
			storeDataName(gr, DRIFILE_BGM, dno, path);
		} else if (0 == strncmp(key, "Save", 4)) {
			dno = key[4] - 'A';
			if (dno < 0 || dno >= DRIFILEMAX) goto errexit;
			storeSaveName(gr, dno, path);
		} else if (0 == strncmp(key, "Ain", 3)) {
			gr->ain = strdup(path);
		} else if (0 == strncmp(key, "WAIA", 4)) {
			gr->wai = strdup(path);
		} else if (0 == strncmp(key, "BGIA", 4)) {
			gr->bgi = strdup(path);
		} else if (0 == strncmp(key, "SACT01", 6)) {
			gr->sact01 = strdup(path);
		} else if (0 == strncmp(key, "Init", 4)) {
			gr->init = strdup(path);
		} else if (0 == strncmp(key, "ALK", 3)) {
			dno = key[4] - '0';
			if (dno < 0 || dno >= 10) goto errexit;
			gr->alk[dno] = strdup(path);
		} else {
			goto errexit;
		}
	}
	return TRUE;

 errexit:
	SYSERROR("Illigal resource at line(%d) file<%s>\n", linecnt, gr_fname);
	return FALSE;
}

boolean initGameResource(GameResource *gr, const char *gr_fname) {
	FILE *fp = fopen(gr_fname, "r");
	if (fp) {
		boolean result = initGameResourceFromFile(gr, fp, gr_fname);
		fclose(fp);
		return result;
	}
	DIR *dir = opendir(".");
	if (dir) {
		boolean result = initGameResourceFromDir(gr, dir, readdir);
		closedir(dir);
		return result;
	}
	SYSERROR("Game Resource File open failed\n");
	return FALSE;
}
