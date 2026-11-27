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
		if (b) {
			*b = '\0';
			gr->save_path = strdup(src);
		} else {
			gr->save_path = strdup(".");
		}
	}
	if (path) free(path);
}

#if defined(ALICEHUB_IOS) || defined(__SWITCH__)
void freeGameResource(GameResource *gr) {
	for (int type = 0; type < DRIFILETYPEMAX; type++)
		for (int i = 0; i < gr->cnt[type]; i++)
			free((void *)gr->game_fname[type][i]);
	for (int i = 0; i < SAVE_MAXNUMBER; i++)
		free((void *)gr->save_fname[i]);
	for (int i = 0; i < 10; i++)
		free((void *)gr->alk[i]);
	free((void *)gr->gr_fname);
	free((void *)gr->save_path);
	free((void *)gr->ain);
	free((void *)gr->wai);
	free((void *)gr->bgi);
	free((void *)gr->sact01);
	free((void *)gr->init);
	free((void *)gr->msgskip);
}
#endif

static bool is_uncensored_overlay_ald(const char *filename) {
	if (!filename || !filename[0] || filename[0] == '.')
		return false;
	static const char needle[] = "uncensored";
	for (const char *p = filename; *p; p++) {
		size_t k = 0;
		while (needle[k] && p[k] && tolower((unsigned char)p[k]) == needle[k])
			k++;
		if (needle[k] == '\0')
			return true;
	}
	return false;
}

bool initGameResourceFromDir(GameResource *gr, DIR *dir, const char *savedir, struct dirent *(*p_readdir)(DIR *)) {
	memset(gr, 0, sizeof(GameResource));

	char *basename = NULL;
	bool found_xsleep = false;
	bool chs_slot[DRIFILETYPEMAX][DRIFILEMAX] = {{false}};
	struct dirent* d;
	while ((d = p_readdir(dir))) {
		char *filename = d->d_name;
		int len = strlen(filename);
		if (strcasecmp(filename, "adisk.ald") == 0) {
			storeDataName(gr, DRIFILE_SCO, 0, filename);
		} else if (strcasecmp(filename, "System39.ain") == 0) {
			gr->ain = strdup(filename);
		} else if (strcasecmp(filename, "SACTEFAM.KLD") == 0) {
			gr->sact01 = strdup(filename);
		} else if (strcasecmp(filename, "System39.ini") == 0) {
			gr->init = strdup(filename);
		} else if (len >= 4 && strcasecmp(filename + len - 4, ".wai") == 0) {
			gr->wai = strdup(filename);
		} else if (len >= 4 && strcasecmp(filename + len - 4, ".bgi") == 0) {
			gr->bgi = strdup(filename);
		} else if (len >= 5 && strcasecmp(filename + len - 4, ".alk") == 0) {
			// AlphaMap.alk is an unused helper archive that collides with demo *.alk on gr->alk[0].
			if (strcasecmp(filename, "AlphaMap.alk") == 0) continue;
			bool numbered = isdigit((unsigned char)filename[len - 5]);
			int idx = numbered ? filename[len - 5] - '0' : 0;
			// Prefer unnumbered <name>.alk (e.g. dDemo.alk) over <name>0.alk backups (e.g. dDemo0.alk).
			if (!gr->alk[idx] || (!numbered && isdigit((unsigned char)gr->alk[idx][strlen(gr->alk[idx]) - 5]))) {
				free((char *)gr->alk[idx]);
				gr->alk[idx] = strdup(filename);
			}
		} else if (len >= 6 && strcasecmp(filename + len - 4, ".ald") == 0) {
			if (is_uncensored_overlay_ald(filename)) continue;
			bool is_chs = len >= 10 && strcasecmp(filename + len - 8, "_chs.ald") == 0;
			int suffix_len = is_chs ? 8 : 4;
			int dno = toupper((unsigned char)filename[len - suffix_len - 1]) - 'A';
			if (dno < 0 || dno >= DRIFILEMAX) continue;
			int type = -1;
			switch (toupper((unsigned char)filename[len - suffix_len - 2])) {
			case 'S': type = DRIFILE_SCO; break;
			case 'G': type = DRIFILE_CG; break;
			case 'W': type = DRIFILE_WAVE; break;
			case 'M': type = DRIFILE_MIDI; break;
			case 'D': type = DRIFILE_DATA; break;
			case 'R': type = DRIFILE_RSC; break;
			case 'B': type = DRIFILE_BGM; break;
			}
			if (type < 0) continue;
			if (!is_chs && chs_slot[type][dno]) continue;
			if (is_chs) chs_slot[type][dno] = true;
			free(gr->game_fname[type][dno]);
			storeDataName(gr, type, dno, filename);
			if (type == DRIFILE_SCO && (!basename || is_chs)) {
				free(basename);
				int base_len = len - suffix_len - 1;
				basename = malloc((size_t)base_len + 1);
				memcpy(basename, filename, (size_t)base_len);
				basename[base_len] = '\0';
			}
		} else if (strcasecmp(filename + 1, "sleep.asd") == 0) {
			found_xsleep = true;
		}
	}
	if (basename && (savedir || !found_xsleep)) {
		const char *dir = savedir ? savedir : "";
		const char *sep = savedir ? "/" : "";
		char *buf = malloc(strlen(dir) + strlen(sep) + strlen(basename) + 12);
#if defined(__ANDROID__) || defined(__EMSCRIPTEN__) || defined(ALICEHUB_IOS) || defined(__SWITCH__)
		// Use lowercase 's[a-z].asd' on mobile/console/web platforms for compatibility.
		basename[strlen(basename) - 1] = 's';
		int a = 'a';
#else
		int a = basename[strlen(basename) - 1] == 'S' ? 'A' : 'a';
#endif
		for (int i = 0; i < SAVE_MAXNUMBER; i++) {
			sprintf(buf, "%s%s%s%c.asd", dir, sep, basename, a + i);
			storeSaveName(gr, i, buf);
		}
#if defined(__ANDROID__) || defined(__EMSCRIPTEN__) || defined(ALICEHUB_IOS) || defined(__SWITCH__)
		basename[strlen(basename) - 1] = '\0';
		if (basename[0] == '\0') {
			sprintf(buf, "%s%smsgskip.dat", dir, sep);
			char *legacy = malloc(strlen(dir) + strlen(sep) + sizeof(".msgskip"));
			if (legacy) {
				sprintf(legacy, "%s%s.msgskip", dir, sep);
				if (access(buf, F_OK) != 0 && access(legacy, F_OK) == 0) {
					if (rename(legacy, buf) != 0) {
						free(buf);
						buf = legacy;
						legacy = NULL;
					}
				}
				free(legacy);
			}
		} else {
			sprintf(buf, "%s%s%s.msgskip", dir, sep, basename);
		}
		gr->msgskip = buf;
#else
		free(buf);
#endif
	} else {
		for (int i = 0; i < SAVE_MAXNUMBER; i++) {
			char buf[] = "asleep.asd";
			buf[0] = 'a' + i;
			storeSaveName(gr, i, buf);
		}
	}
	free(basename);

	return gr->cnt[DRIFILE_SCO] > 0;
}

static void trimRight(char *str) {
	for (char *p = str + strlen(str) - 1; p >= str && isspace(*p); p--)
		*p = '\0';
}

static char *descriptor_save_path(const char *dir, const char *path) {
	const char *name = path + strcspn(path, ":");
	name = *name ? name + 1 : path;
	for (const char *p = name; *p; p++) {
		if (*p == '/' || *p == '\\')
			name = p + 1;
	}
	if (!*name || !strcmp(name, ".") || !strcmp(name, ".."))
		return NULL;
	char *result = malloc(strlen(dir) + strlen(name) + 2);
	if (!result)
		NOMEMERR();
	sprintf(result, "%s/%s", dir, name);
	return result;
}

static bool descriptor_save_path_used(const GameResource *gr, const char *path, int slots) {
	if (gr->msgskip && !strcasecmp(gr->msgskip, path))
		return true;
	for (int i = 0; i < slots; i++) {
		if (gr->save_fname[i] && !strcasecmp(gr->save_fname[i], path))
			return true;
	}
	return false;
}

static void disambiguate_descriptor_saves(GameResource *gr, const char *savedir) {
	for (int i = 0; i < SAVE_MAXNUMBER; i++) {
		const char *path = gr->save_fname[i];
		if (!path || !descriptor_save_path_used(gr, path, i))
			continue;
		unsigned attempt = 0;
		char *replacement;
		for (;;) {
			char name[32];
			snprintf(name, sizeof(name), "save-%c-%u.asd", 'A' + i, ++attempt);
			replacement = descriptor_save_path(savedir, name);
			if (!descriptor_save_path_used(gr, replacement, SAVE_MAXNUMBER))
				break;
			free(replacement);
		}
		free((void *)path);
		gr->save_fname[i] = replacement;
	}
}

static bool initGameResourceFromFile(GameResource *gr, FILE *fp, const char *gr_fname, const char *savedir) {
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
			if (dno < 0 || dno >= SAVE_MAXNUMBER) goto errexit;
			if (savedir) {
				char *save = descriptor_save_path(savedir, path);
				if (!save) goto errexit;
				storeSaveName(gr, dno, save);
				free(save);
				continue;
			}
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
		} else if (0 == strncmp(key, "MsgSkip", 7)) {
			if (savedir) {
				gr->msgskip = descriptor_save_path(savedir, path);
				if (!gr->msgskip) goto errexit;
				continue;
			}
			gr->msgskip = strdup(path);
		} else {
			goto errexit;
		}
	}
	if (savedir) {
		disambiguate_descriptor_saves(gr, savedir);
		if (!gr->save_path)
			gr->save_path = strdup(savedir);
	}
	gr->gr_fname = strdup(gr_fname);
	return true;

 errexit:
#ifndef __SWITCH__
	SYSERROR("Illigal resource at line(%d) file<%s>", linecnt, gr_fname);
#else
	WARNING("Invalid resource at line(%d) file<%s>", linecnt, gr_fname);
#endif
	return false;
}

bool initGameResource(GameResource *gr, const char *gr_fname, const char *savedir) {
	FILE *fp = fopen(gr_fname, "r");
	if (fp) {
#ifdef __SWITCH__
		bool result = initGameResourceFromFile(gr, fp, gr_fname, savedir);
#else
		bool result = initGameResourceFromFile(gr, fp, gr_fname, NULL);
#endif
		fclose(fp);
		return result;
	}
	DIR *dir = opendir(".");
	if (dir) {
		bool result = initGameResourceFromDir(gr, dir, savedir, readdir);
		closedir(dir);
		return result;
	}
	SYSERROR("Game Resource File open failed");
	return false;
}
