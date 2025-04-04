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
	bool mute;
};
static struct _volval vval[MAXVOLCH];
static GtkWidget *vval_win;

#include "menu_gui_volval.c"

// 初期化
bool s39ini_init(void) {
	FILE *fp;
	char s[256], s1[256];
	int i, vol[MAXVOLCH] = {0};
	char fn[256];
	
	if (nact->files.init == NULL) return false;
	
	if (NULL == (fp = fopen(nact->files.init, "r"))) return false;
	
	while (fgets(s, 255, fp) != NULL) {
		s1[0] = '\0';
		sscanf(s, "VolumeValancer[%d] = \"%s", &i, s1);
		if (s1[0] == '\0') continue;
		if (i >= MAXVOLCH || i < 0) continue;
		s1[strlen(s1)-1] = '\0'; // remove last '"'
		vval[i].label = sjis2utf(s1);
		vval_max = max(vval_max, i);
		//WARNING("VolumeValancer[%d] = %s", i, vval[i].label);
	}
	
	if (vval_max <= 0) return false;
	
	// Volume.sav があればそれを読み込む
	snprintf(fn, sizeof(fn) -1, "%s/Volume.sav", nact->files.save_path);
	if (NULL == (fp = fopen(fn, "rb"))) {
		// とりあえず、初期ボリュームは 100
		for (i = 0; i < MAXVOLCH; i++) {
			vol[i] = vval[i].vol = 100;
		}
	} else {
		int n = fread(vol, sizeof(int), MAXVOLCH, fp);
		fclose(fp);
		for (i = 0; i < n; i++) {
			vval[i].vol = vol[i];
		}
	}
	// どちらにしても music server に送る
	mus_vol_set_valance(vol, MAXVOLCH);
	
	// System39.ini に VolumeValancer が無かったらなし
	if (vval_max > 0) {
		vval_win = vval_win_open(vval, vval_max);
	}
	
	return true;
}

// PopupMenuから呼ばれる
void s39ini_winopen(void) {
	if (vval_win) {
		gtk_widget_show(vval_win);
		nact->popupmenu_opened = true;
	}
}

// ボリューム設定Windowが閉じられたときに呼ばれる
void s39ini_winclose(void) {
	if (vval_win) {
		gtk_widget_hide(vval_win);
		nact->popupmenu_opened = false;
	}
}

// ボリューム設定でスケールを動かすたびに呼ばれる
void s39ini_setvol(void) {
	int vol[MAXVOLCH] = {0};
	int i;
	
	if (vval_win == NULL) return;
	
	for (i = 0; i < MAXVOLCH; i++) {
		vol[i] = vval[i].mute ? 0 : vval[i].vol;
	}
	
	mus_vol_set_valance(vol, MAXVOLCH);
}

// Volume Valance をセーブ
void s39ini_remove(void) {
	int vol[MAXVOLCH] = {0};
	char fn[256];
	
	if (vval_win == NULL) return;
	
	for (int i = 0; i < MAXVOLCH; i++) {
		vol[i] = vval[i].vol;
	}
	
	snprintf(fn, sizeof(fn) -1, "%s/Volume.sav", nact->files.save_path);
	FILE *fp = fopen(fn, "wb");
	if (!fp) {
		WARNING("Failed to save Volume.sav");
		return;
	}
	
	fwrite(vol, sizeof(int), MAXVOLCH, fp);
	fclose(fp);
}
