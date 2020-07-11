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
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "ald_manager.h"
#include "nact.h"
#include "savedata.h"
#include "system.h"

typedef struct {
	char *game_fname[DRIFILETYPEMAX][DRIFILEMAX];
	int cnt[DRIFILETYPEMAX];
	char *save_path;
	char *save_fname[SAVE_MAXNUMBER];
	char *ain;
	char *wai;
	char *bgi;
	char *sact01;
	char *init;
	char *alk[10];
} GameResource;

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

static boolean initFromDir(GameResource *gr) {
	char cwd[256], path[256];
	getcwd(cwd, 255);
	DIR *dir = opendir(".");
	if(!dir)
		SYSERROR("Game Resouce File open failed\n");

	struct dirent* d;
	while ((d = readdir(dir))) {
		char *filename = d->d_name;
		int len = strlen(filename);
		sprintf(path, "%s/%s", cwd, filename);
		if (strcasecmp(filename, "adisk.ald") == 0) {
			storeDataName(gr, DRIFILE_SCO, 0, path);
		} else if (strcasecmp(filename, "system39.ain") == 0) {
			gr->ain = strdup(filename);
		} else if (strcasecmp(filename + len - 4, ".ald") == 0) {
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

static boolean initFromFile(GameResource *gr, FILE *fp, const char *gr_fname) {
	int linecnt = 0, dno;
	char line[256];
	char key[256], path[256];

	while (fgets(line, 255, fp) != NULL) {
		linecnt++;
		if (line[0] == '#') continue;
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
	SYSERROR("Illigal resouce at line(%d) file<%s>\n", linecnt, gr_fname);
	return FALSE;
}

static void registerFiles(GameResource *gr) {
	if (gr->cnt[DRIFILE_SCO] == 0) {
		SYSERROR("No Scenario data available\n");
	}
	if (gr->cnt[DRIFILE_SCO] > 0)
		ald_init(DRIFILE_SCO, gr->game_fname[DRIFILE_SCO], gr->cnt[DRIFILE_SCO], TRUE);
	if (gr->cnt[DRIFILE_CG] > 0)
		ald_init(DRIFILE_CG,  gr->game_fname[DRIFILE_CG], gr->cnt[DRIFILE_CG], TRUE);
	if (gr->cnt[DRIFILE_WAVE] > 0)
		ald_init(DRIFILE_WAVE, gr->game_fname[DRIFILE_WAVE], gr->cnt[DRIFILE_WAVE], TRUE);
	if (gr->cnt[DRIFILE_MIDI] > 0)
		ald_init(DRIFILE_MIDI, gr->game_fname[DRIFILE_MIDI], gr->cnt[DRIFILE_MIDI], TRUE);
	if (gr->cnt[DRIFILE_DATA] > 0)
		ald_init(DRIFILE_DATA, gr->game_fname[DRIFILE_DATA], gr->cnt[DRIFILE_DATA], TRUE);
	if (gr->cnt[DRIFILE_RSC] > 0)
		ald_init(DRIFILE_RSC, gr->game_fname[DRIFILE_RSC], gr->cnt[DRIFILE_RSC], TRUE);
	if (gr->cnt[DRIFILE_BGM] > 0)
		ald_init(DRIFILE_BGM, gr->game_fname[DRIFILE_BGM], gr->cnt[DRIFILE_BGM], TRUE);

	if (gr->save_path)
		save_set_path(gr->save_path);
	for (int i = 0; i < SAVE_MAXNUMBER; i++) {
		if (gr->save_fname[i])
			save_register_file(gr->save_fname[i], i);
	}

	nact->ain.path_to_ain = gr->ain;
	nact->files.wai = gr->wai;
	nact->files.bgi = gr->bgi;
	nact->files.sact01 = gr->sact01;
	nact->files.init = gr->init;
	for (int i = 0; i < 10; i++)
		nact->files.alk[i] = gr->alk[i];
}

boolean initGameDataResorce(const char *gr_fname) {
	GameResource gr;
	memset(&gr, 0, sizeof(gr));

	FILE *fp = fopen(gr_fname, "r");
	boolean result;
	if (fp) {
		result = initFromFile(&gr, fp, gr_fname);
		fclose(fp);
	} else {
		result = initFromDir(&gr);
	}
	if (result)
		registerFiles(&gr);
	return result;
}
