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

/* game data file name */
static char *gamefname[DRIFILETYPEMAX][DRIFILEMAX];

static void storeDataName(int type, int no, char *src) {
	gamefname[type][no] = strdup(src);
}

static void storeSaveName(int no, char *src) {
	char *home_dir = "";
	char *path = NULL;
	
	if (*src == '~') {
		home_dir = getenv("HOME");
		if (NULL == (path = malloc(strlen(home_dir) + strlen(src) + 1))) {
			NOMEMERR();
		}
		sprintf(path, "%s%s", home_dir, src+1);
		save_register_file(path, no);
		src = path;
	} else {
		save_register_file(src, no);
	}
	
	if (no == 0) {
		char *b = strrchr(src, '/');
		if (b == NULL) {
			SYSERROR("Illigal save filename %s\n", src);
		}
		*b = '\0';
		save_set_path(src);
	}
	if (path) free(path);
}

/* ゲームのデータファイルの情報をディレクトリから作成 thanx tajiri@wizard*/
static boolean sys35_initGameDataDir(int* cnt)
{
    DIR* dir;
    struct dirent* d;
    char s1[256], s2[256];
    int dno;
    int i;
    
    getcwd(s1,255);
    if(NULL == (dir= opendir(".")))
    {
        SYSERROR("Game Resouce File open failed\n");
    }
    while(NULL != (d = readdir(dir))){
        char *filename = d->d_name;
        int len = strlen(filename);
        sprintf(s2,"%s%c%s",s1,'/',filename);
        if(strcasecmp(filename,"adisk.ald")==0){
            storeDataName(DRIFILE_SCO, 0, s2);
            cnt[0] = max(1, cnt[0]);
        } else if (strcasecmp(filename, "system39.ain") == 0) {
		nact->ain.path_to_ain = strdup(filename);
	}
        else if(strcasecmp(filename+len-4,".ald")==0){
            dno = toupper(*(filename+len-5)) - 'A';
            if (dno < 0 || dno >= DRIFILEMAX) continue;
            switch(*(filename+len-6)){
                case 'S':
                case 's':
                    storeDataName(DRIFILE_SCO, dno, s2);
                    cnt[0] = max(dno + 1, cnt[0]);
                    break;
                case 'g':
                case 'G':
                    storeDataName(DRIFILE_CG, dno, s2);
                    cnt[1] = max(dno + 1, cnt[1]);
                    break;
                case 'W':
                case 'w':
                    storeDataName(DRIFILE_WAVE, dno, s2);
                    cnt[2] = max(dno + 1, cnt[2]);
                    break;
                case 'M':
                case 'm':
                    storeDataName(DRIFILE_MIDI, dno, s2);
                    cnt[3] = max(dno + 1, cnt[3]);
                    break;
                case 'D':
                case 'd':
                    storeDataName(DRIFILE_DATA, dno, s2);
                    cnt[4] = max(dno + 1, cnt[4]);
                    break;
                case 'R':
                case 'r':
                    storeDataName(DRIFILE_RSC, dno, s2);
                    cnt[5] = max(dno + 1, cnt[5]);
                    break;
                case 'B':
                case 'b':
                    storeDataName(DRIFILE_BGM, dno, s2);
                    cnt[6] = max(dno + 1, cnt[6]);
                    break;
            }
        }
    }
    for(i=0;i<SAVE_MAXNUMBER;i++){
        sprintf(s2,"%s/%c%s",s1,'a'+i,"sleep.asd");
        storeSaveName(i, s2);
    }

    if(cnt[0]>0)
        return TRUE;
    else
        return FALSE;
}

    
/* ゲームのデータファイルの情報を読み込む */
boolean initGameDataResorce(const char *gameResourceFile) {
	int cnt[] = {0, 0, 0, 0, 0, 0, 0};
	int linecnt = 0, dno;
	FILE *fp;
	char s[256];
	char s1[256], s2[256];
	
	if (NULL == (fp = fopen(gameResourceFile, "r"))) {
		if(sys35_initGameDataDir(cnt)==TRUE) {
			goto initdata;
		}
		return FALSE;
	}
	while(fgets(s, 255, fp) != NULL ) {
		linecnt++;
		if (s[0] == '#') continue;
		s2[0] = '\0';
		sscanf(s,"%s %s", s1, s2);
		if (s2[0] == '\0') continue;
		if (0 == strncmp(s1, "Scenario", 8)) {
			dno = s1[8] - 'A';
			if (dno < 0 || dno >= DRIFILEMAX) goto errexit;
			storeDataName(DRIFILE_SCO, dno, s2);
			cnt[0] = max(dno + 1, cnt[0]);
		} else if (0 == strncmp(s1, "Graphics", 8)) {
			dno = s1[8] - 'A';
			if (dno < 0 || dno >= DRIFILEMAX) goto errexit;
			storeDataName(DRIFILE_CG, dno, s2);
			cnt[1] = max(dno + 1, cnt[1]);
		} else if (0 == strncmp(s1, "Wave", 4)) {
			dno = s1[4] - 'A';
			if (dno < 0 || dno >= DRIFILEMAX) goto errexit;
			storeDataName(DRIFILE_WAVE, dno, s2);
			cnt[2] = max(dno + 1, cnt[2]);
		} else if (0 == strncmp(s1, "Midi", 4)) {
			dno = s1[4] - 'A';
			if (dno < 0 || dno >= DRIFILEMAX) goto errexit;
			storeDataName(DRIFILE_MIDI, dno, s2);
			cnt[3] = max(dno + 1, cnt[3]);
		} else if (0 == strncmp(s1, "Data", 4)) {
			dno = s1[4] - 'A';
			if (dno < 0 || dno >= DRIFILEMAX) goto errexit;
			storeDataName(DRIFILE_DATA, dno, s2);
			cnt[4] = max(dno + 1, cnt[4]);
		} else if (0 == strncmp(s1, "Resource", 8)) {
			dno = s1[8] - 'A';
			if (dno < 0 || dno >= DRIFILEMAX) goto errexit;
			storeDataName(DRIFILE_RSC, dno, s2);
			cnt[5] = max(dno + 1, cnt[5]);
		} else if (0 == strncmp(s1, "BGM", 3)) {
			dno = s1[3] - 'A';
			if (dno < 0 || dno >= DRIFILEMAX) goto errexit;
			storeDataName(DRIFILE_BGM, dno, s2);
			cnt[6] = max(dno + 1, cnt[6]);
		} else if (0 == strncmp(s1, "Save", 4)) {
			dno = s1[4] - 'A';
			if (dno < 0 || dno >= DRIFILEMAX) goto errexit;
			storeSaveName(dno, s2);
		} else if (0 == strncmp(s1, "Ain", 3)) {
			nact->ain.path_to_ain = strdup(s2);
		} else if (0 == strncmp(s1, "WAIA", 4)) {
			nact->files.wai = strdup(s2);
		} else if (0 == strncmp(s1, "BGIA", 4)) {
			nact->files.bgi = strdup(s2);
		} else if (0 == strncmp(s1, "SACT01", 6)) {
			nact->files.sact01 = strdup(s2);
		} else if (0 == strncmp(s1, "Init", 4)) {
			nact->files.init = strdup(s2);
		} else if (0 == strncmp(s1, "ALK", 3)) {
			dno = s1[4] - '0';
			if (dno < 0 || dno >= 10) goto errexit;
			nact->files.alk[dno] = strdup(s2);
		} else {
			goto errexit;
		}
	}
	fclose(fp);
initdata:
	if (cnt[0] == 0) {
		SYSERROR("No Scenario data available\n");
	}
	
	if (cnt[0] > 0)
		ald_init(DRIFILE_SCO, gamefname[DRIFILE_SCO], cnt[0], TRUE);
	if (cnt[1] > 0)
		ald_init(DRIFILE_CG,  gamefname[DRIFILE_CG], cnt[1], TRUE);
	if (cnt[2] > 0)
		ald_init(DRIFILE_WAVE, gamefname[DRIFILE_WAVE], cnt[2], TRUE);
	if (cnt[3] > 0)
		ald_init(DRIFILE_MIDI, gamefname[DRIFILE_MIDI], cnt[3], TRUE);
	if (cnt[4] > 0)
		ald_init(DRIFILE_DATA, gamefname[DRIFILE_DATA], cnt[4], TRUE);
	if (cnt[5] > 0)
		ald_init(DRIFILE_RSC, gamefname[DRIFILE_RSC], cnt[5], TRUE);
	if (cnt[6] > 0)
		ald_init(DRIFILE_BGM, gamefname[DRIFILE_BGM], cnt[6], TRUE);

	return TRUE;

 errexit:
	SYSERROR("Illigal resouce at line(%d) file<%s>\n",linecnt, gameResourceFile);
	return FALSE;
}
