/*
 * cmdl.c  SYSTEM35 L command
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
/* $Id: cmdl.c,v 1.27 2003/01/17 23:23:11 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include "portab.h"
#include "xsystem35.h"
#include "scenario.h"
#include "dri.h"
#include "savedata.h"
#include "utfsjis.h"
#include "cg.h"
#include "cmd_check.h"

void commandLD() {
	/* 変数領域などのデータをロードする。（全ロード）*/
	int num   = getCaliValue();
	
	if (num <= 0) {
		sysVar[0] = 255;
	} else {
		sysVar[0] = save_loadAll(num - 1);
	}
	TRACE("LD %d:",num);
}

void commandLP() {
	/* セーブデータの一部分をロードする。(数値変数部) */
	struct VarRef point;
	int num = getCaliValue();
	getCaliArray(&point);
	int cnt = getCaliValue();
	
	if (num <= 0) {
		sysVar[0] = 255;
	} else {
		sysVar[0] = save_loadPartial(num - 1, &point, cnt);
	}
	TRACE("LP %d,%d,%d:",num, point.var, cnt);
}

void commandLT() {
	/* タイムスタンプの読み込み */
	int num  = getCaliValue();
	int *var = getCaliVariable();
	int status;
	struct stat buf;
	struct tm *lc;
	
	if (num <= 0 || num > SAVE_MAXNUMBER) {
		var[0] = 0;
		var[1] = 0;
		var[2] = 0;
		var[3] = 0;
		var[4] = 0;
		var[5] = 0;
		sysVar[0] = 255;
		return;
	}
	
	status = stat(save_get_file(num - 1), &buf);
	if (status) {
		/* んなんどこにもかいてないやん！ */
		var[0] = 0;
		var[1] = 0;
		var[2] = 0;
		var[3] = 0;
		var[4] = 0;
		var[5] = 0;
		sysVar[0] = 255;
	} else {
		lc = localtime(&buf.st_mtime);
		var[0] = lc->tm_year + 1900;
		var[1] = lc->tm_mon + 1;
		var[2] = lc->tm_mday;
		var[3] = lc->tm_hour;
		var[4] = lc->tm_min;
		var[5] = lc->tm_sec;
		sysVar[0]  = 0;
	}
	TRACE("LT %d,%p",num, var);
}

void commandLE(char terminator) {
	int type = sl_getc();
	const char *filename = sl_getString(terminator);
	char *fname_utf8 = toUTF8(filename);
	char *fallback_fname = NULL;
	if (strchr(fname_utf8, '\\')) {
		fallback_fname = strdup(fname_utf8);
		for (char *p = fname_utf8; *p; p++) {
			if (*p == '\\')
				*p = '/';
		}
	}

	int var, cnt;
	struct VarRef vref;
	switch (type) {
	case 0:
		getCaliArray(&vref);
		var = vref.var;
		cnt = getCaliValue();
		sysVar[0] = load_vars_from_file(fname_utf8, &vref, cnt);
		// For compatibility; the QE command in v2.16.1 and earlier did not
		// convert backslashes to slashes.
		if (sysVar[0] == SAVE_LOADERR && fallback_fname)
			sysVar[0] = load_vars_from_file(fallback_fname, &vref, cnt);
		break;
	case 1:
		var = getCaliValue();
		cnt = getCaliValue();
		sysVar[0] = load_strs_from_file(fname_utf8, var, cnt);
		if (sysVar[0] == SAVE_LOADERR && fallback_fname)
			sysVar[0] = load_strs_from_file(fallback_fname, var, cnt);
		break;
	default:
		var = getCaliValue();
		cnt = getCaliValue();
		WARNING("Unknown LE command %d", type);
		break;
	}
	if (fallback_fname)
		free(fallback_fname);
	free(fname_utf8);
	
	TRACE("LE %d,%s,%d,%d:",type, filename, var, cnt);
}

void commandLL() {
	int type = sl_getc();
	int link_no = getCaliValue();
	int *var, _var = 0;
	int num, i;
	dridata *dfile = ald_getdata(DRIFILE_DATA, link_no - 1);
	uint16_t *data;
	
	if (dfile == NULL) {
		getCaliValue();
		getCaliValue();
		sysVar[0] = 255;
		return;
	}
	
	data = (uint16_t *)dfile->data;
	
	switch(type) {
	case 0: /* T2 */
		var = getCaliVariable();
		num  = getCaliValue();

		TRACE("LL %d,%d,%d,%d:",type, link_no, _var, num);
		
		if (dfile->size < num * sizeof(uint16_t)) {
			WARNING("data shortage (link_no = %d, requested %d, loaded %d)", link_no, num, dfile->size/ sizeof(uint16_t));
			/* sysVar[0] = 254; 大嘘*/
			/* return; */
			num = dfile->size / sizeof(uint16_t);
		}
		for (i = 0; i < num; i++) {
			var[i] = SDL_SwapLE16(data[i]);
		}
		break;
		
	case 1:
		_var = getCaliValue();
		num  = getCaliValue();
		TRACE_UNIMPLEMENTED("LL1 not yet %d, %d", _var, num);
		sysVar[0] = 255;
		goto out;
		break;
		
	default:
		WARNING("Unknown LL command %d", type);
		goto out;
	}
	
	sysVar[0] = 0;

 out:
	ald_freedata(dfile);
}

void commandLHD() {
	// ＣＤのデータをＨＤＤへ登録／削除する
	int p1 = sl_getc();
	int no = getCaliValue();
	// X版では全てをHDDに置くのでサポートしない
	
	sysVar[0] = 255;
	TRACE("LHD %d,%d:",p1,no);
}

void commandLHG() {
	// ＣＤのデータをＨＤＤへ登録／削除する
	int p1 = sl_getc();
	int no = getCaliValue();

	// HACK: Remember the last registered number so that LHG3 called immediately
	// after LHG1 can return 1. This prevents "HDD is full or CD is not inserted"
	// error message in Diabolique.
	static int last_registered = -1;
	switch (p1) {
	case 1:  // register
		last_registered = no;
		break;
	case 2:  // unregister
		last_registered = -1;
		break;
	case 3:  // query
		// Unconditionally returning 1 breaks Atlach-Nacha.
		sysVar[0] = (no == last_registered) ? 1 : 0;
		break;
	}
	TRACE("LHG %d,%d:",p1,no);
}

void commandLHM() {
	// ＣＤのデータをＨＤＤへ登録／削除する
	int p1 = sl_getc();
	int no = getCaliValue();
	// X版では全てをHDDに置くのでサポートしない
	
	sysVar[0] = 255;
	TRACE("LHM %d,%d:",p1,no);
}

void commandLHS() {
	// ＣＤのデータをＨＤＤへ登録／削除する
	int p1 = sl_getc();
	int no = getCaliValue();
	// X版では全てをHDDに置くのでサポートしない
	
	sysVar[0] = 255;
	TRACE("LHS %d,%d:",p1,no);
}

void commandLHW() {
	// ＣＤのデータをＨＤＤへ登録／削除する
	int p1 = sl_getc();
	int no = getCaliValue();
	// X版では全てをHDDに置くのでサポートしない
	
	sysVar[0] = 255;
	TRACE("LHW %d,%d:",p1,no);
}

void commandLC(char terminator) {
	int x = getCaliValue();
	int y = getCaliValue();
	const char *filename = sl_getString(terminator);

	char *fname_utf8 = toUTF8(filename);
	sysVar[0] = cg_load_with_filename(fname_utf8, x, y);
	free(fname_utf8);
	
	TRACE("LC %d,%d,%s:", x, y, filename);
}

void commandLXG(char terminator) {
	/* ファイルを選択する */
	int file_name = getCaliValue();
	const char *title = sl_getString(terminator);
	const char *filter = sl_getString(terminator);

	TRACE_UNIMPLEMENTED("LXG %d,%s,%s:", file_name, title, filter);
}

void commandLXO() {
	/* ファイルを作成またはオープンする */
	int num           = getCaliValue();
	int file_name     = getCaliValue();
	int how_to_create = getCaliValue();

	sysVar[0]=255;
	TRACE_UNIMPLEMENTED("LXO %d,%d,%d:",num,file_name,how_to_create);
}

void commandLXC() {
	/* ファイルをクローズする */
	int num = getCaliValue();
	
	TRACE_UNIMPLEMENTED("LXC %d:",num);
}

void commandLXL() {
	/* CGファイルロード */
	int x         = getCaliValue();
	int y         = getCaliValue();
	int file_name = getCaliValue();
	
	TRACE_UNIMPLEMENTED("LXL %d,%d,%d:",x,y,file_name);
}

void commandLXS() {
	/* ファイルサイズ(バイト数)を取得する */
	int num = getCaliValue();
	int *hi = getCaliVariable();
	int *lo = getCaliVariable();
	
	TRACE_UNIMPLEMENTED("LXS %d,%d,%d:", num, *hi, *lo);
}

void commandLXP() {
	/* ファイルポインタの位置を(先頭からのバイト数)設定する */
	int num = getCaliValue();
	int hi  = getCaliValue();
	int lo  = getCaliValue();
	
	TRACE_UNIMPLEMENTED("LXP %d,%d,%d:",num,hi,lo);
}

void commandLXR() {
	/* ファイルからデータを読み取る */
	int num  = getCaliValue();
	int *var = getCaliVariable();
	int size = getCaliValue();
	
	TRACE_UNIMPLEMENTED("LXR %d,%d,%d:",num,*var,size);
}

void commandLXW() {
	/* ファイルにデータを書き込む */
	int num  = getCaliValue();
	int *var = getCaliVariable();
	int size = getCaliValue();
	
	TRACE_UNIMPLEMENTED("LXW %d,%d,%d:",num,*var,size);
}

void commandLXX() {
	/* Gets file timestamp to [var, var+6] */
	int type = getCaliValue();
	int num = getCaliValue();
	int *var = getCaliVariable();

	struct stat buf;
	struct tm *lc;

	switch (type) {
	case 5: // save data
		if (num <= 0 || num > SAVE_MAXNUMBER || stat(save_get_file(num - 1), &buf)) {
			sysVar[0] = 255;
			break;
		}
		lc = localtime(&buf.st_mtime);
		var[0] = lc->tm_year + 1900;
		var[1] = lc->tm_mon + 1;
		var[2] = lc->tm_mday;
		var[3] = lc->tm_hour;
		var[4] = lc->tm_min;
		var[5] = lc->tm_sec;
		var[6] = lc->tm_wday;
		sysVar[0] = 0;
		break;

	case 0: // scenario
	case 1: // CG
	case 2: // wave
	case 3: // MIDI
	case 4: // data
	case 6: // resource
	default:
		TRACE_UNIMPLEMENTED("LXX %d,%d,%d:", type, num, *var);
		return;
	}

	TRACE("LXX %d,%d,%d:", type, num, *var);
}

void commandLXWT() {
	int eNum = getCaliValue();
	const char *sText = sl_getString(0);

	TRACE_UNIMPLEMENTED("LXWT %d, %s:", eNum, sText);
}

void commandLXWS() {
	int eNum = getCaliValue();
	int eTextNum = getCaliValue();

	TRACE_UNIMPLEMENTED("LXWS %d, %d:", eNum, eTextNum);
}

void commandLXWE() {
	int eNum = getCaliValue();
	int eType = getCaliValue();

	TRACE_UNIMPLEMENTED("LXWE %d, %d:", eNum, eType);
}

void commandLXWH() {
	int eFile = getCaliValue();
	int nFlg = sl_getc();
	int eNum = getCaliValue();

	TRACE_UNIMPLEMENTED("LXWH %d, %d, %d:", eFile, nFlg, eNum);
}

void commandLXWHH() {
	int eFile = getCaliValue();
	int nFlg  = sl_getc();
	int eNum  = getCaliValue();

	TRACE_UNIMPLEMENTED("LXWHH %d, %d, %d:", eFile, nFlg, eNum);
}

void commandLXF() {
	int file_name = getCaliValue();
	const char *title = sl_getString(0);
	const char *folder = sl_getString(0);

	TRACE_UNIMPLEMENTED("LXF %d, %s, %s:", file_name, title, folder);
}
