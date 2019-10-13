/*
 * s39init.c  System39.ini read
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
/* $Id: s39init.c,v 1.2 2003/04/27 11:00:36 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include "portab.h"
#include "system.h"
#include "nact.h"
#include "s39init.h"
#include "utfsjis.h"
#include "music.h"

// Volume Valancer で扱う最大チャンネル数
#define MAXVOLCH 16

static int vval_max;  // 最大チャンネル番号
struct _volval {
	char *label;
	int vol;
	boolean mute;
};
static struct _volval vval[MAXVOLCH];
static GtkWidget *vval_win;

#include "menu_gui_volval.c"

// 初期化
int s39ini_init(void) {
	FILE *fp;
	char s[256], s1[256];
	int i, vol[MAXVOLCH] = {0};
	char fn[256];
	
	if (nact->files.init == NULL) return NG;
	
	if (NULL == (fp = fopen(nact->files.init, "r"))) return NG;
	
	while (fgets(s, 255, fp) != NULL) {
		s1[0] = '\0';
		sscanf(s, "VolumeValancer[%d] = \"%s", &i, s1);
		if (s1[0] == '\0') continue;
		if (i >= MAXVOLCH || i < 0) continue;
		s1[strlen(s1)-1] = '\0'; // remove last '"'
		vval[i].label = sjis2lang(s1);
		vval_max = max(vval_max, i);
		//WARNING("VolumeValancer[%d] = %s\n", i, vval[i].label);
	}
	
	if (vval_max <= 0) return NG;
	
	// Volume.sav があればそれを読み込む
	snprintf(fn, sizeof(fn) -1, "%s/Volume.sav", nact->files.savedir);
	if (NULL == (fp = fopen(fn, "rb"))) {
		// とりあえず、初期ボリュームは 100
		for (i = 0; i < MAXVOLCH; i++) {
			vol[i] = vval[i].vol = 100;
		}
	} else {
		fread(vol, sizeof(int), MAXVOLCH, fp);
		fclose(fp);
		for (i = 0; i < MAXVOLCH; i++) {
			vval[i].vol = vol[i];
		}
	}
	// どちらにしても music server に送る
	mus_vol_set_valance(vol, MAXVOLCH);
	
	// System39.ini に VolumeValancer が無かったらなし
	if (vval_max > 0) {
		vval_win = vval_win_open(vval, vval_max);
	}
	
	return OK;
}

// PopupMenuから呼ばれる
int s39ini_winopen() {
	if (vval_win) {
		gtk_widget_show(vval_win);
		nact->popupmenu_opened = TRUE;
	}
	return OK;
}

// ボリューム設定Windowが閉じられたときに呼ばれる
int s39ini_winclose() {
	if (vval_win) {
		gtk_widget_hide(vval_win);
		nact->popupmenu_opened = FALSE;
	}
	return OK;
}

// ボリューム設定でスケールを動かすたびに呼ばれる
int s39ini_setvol() {
	int vol[MAXVOLCH] = {0};
	int i;
	
	if (vval_win == NULL) return OK;
	
	for (i = 0; i < MAXVOLCH; i++) {
		vol[i] = vval[i].mute ? 0 : vval[i].vol;
	}
	
	mus_vol_set_valance(vol, MAXVOLCH);
	return OK;
}

// Volume Valance をセーブ
int s39ini_remove() {
	int vol[MAXVOLCH] = {0};
	FILE *fp;
	char fn[256];
	int i;
	
	if (vval_win == NULL) return OK;
	
	for (i = 0; i < MAXVOLCH; i++) {
		vol[i] = vval[i].vol;
	}
	
	snprintf(fn, sizeof(fn) -1, "%s/Volume.sav", nact->files.savedir);
	if (NULL == (fp = fopen(fn, "wb"))) {
		WARNING("Fail to save Volume.save\n");
		return NG;
	}
	
	fwrite(vol, sizeof(int), MAXVOLCH, fp);
	fclose(fp);
	
	return OK;
	
}
