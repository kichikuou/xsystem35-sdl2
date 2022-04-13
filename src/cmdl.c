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

void commandLD() {
	/* 変数領域などのデータをロードする。（全ロード）*/
	int num   = getCaliValue();
	
	if (num <= 0) {
		sysVar[0] = 255;
	} else {
		sysVar[0] = save_loadAll(num - 1);
	}
	DEBUG_COMMAND("LD %d:\n",num);
}

void commandLP() {
	/* セーブデータの一部分をロードする。(数値変数部) */
	int num    = getCaliValue();
	int *point = getCaliVariable();
	int page   = preVarPage;
	int index  = preVarIndex;
	int cnt    = getCaliValue();
	
	if (num <= 0) {
		sysVar[0] = 255;
	} else {
		sysVar[0]  = save_loadPartial(num - 1, page, index, cnt);
	}
	DEBUG_COMMAND("LP %d,%p,%d:\n",num, point, cnt);
}

void commandLT() {
	/* タイムスタンプの読み込み */
	int num  = getCaliValue();
	int *var = getCaliVariable();
	int status;
	struct stat buf;
	struct tm *lc;
	
	if (num <= 0) {
		*var       = 0;
		*(var + 1) = 0;
		*(var + 2) = 0;
		*(var + 3) = 0;
		*(var + 4) = 0;
		*(var + 5) = 0;
		sysVar[0] = 255;
		return;
	}
	
	status = stat(save_get_file(num - 1), &buf);
	if (status) {
		/* んなんどこにもかいてないやん！ */
		*var       = 0;
		*(var + 1) = 0;
		*(var + 2) = 0;
		*(var + 3) = 0;
		*(var + 4) = 0;
		*(var + 5) = 0;
		sysVar[0] = 255;
	} else {
		lc = localtime(&buf.st_mtime);
		*var       = 1900 + lc->tm_year;
		*(var + 1) = 1    + lc->tm_mon;
		*(var + 2) =        lc->tm_mday;
		*(var + 3) =        lc->tm_hour;
		*(var + 4) =        lc->tm_min;
		*(var + 5) =        lc->tm_sec;
		sysVar[0]  = 0;
	}
	DEBUG_COMMAND("LT %d,%p\n",num, var);
}

void commandLE() {
	int type = sl_getc();
	const char *filename = sl_getString(':');
	int *var, _var = 0;
	int num;

	char *fname_utf8 = toUTF8(filename);
	switch(type) {
	case 0: /* T2 */
		var = getCaliVariable();
		num  = getCaliValue();
		sysVar[0] = save_load_var_with_file(fname_utf8, var, num);
		break;
	case 1: /* 456 */
		_var = getCaliValue();
		num  = getCaliValue();
		sysVar[0] = save_load_str_with_file(fname_utf8, _var, num);
		break;
	default:
		_var = getCaliValue();
		num  = getCaliValue();
		WARNING("Unknown LE command %d\n", type);
		break;
	}
	free(fname_utf8);
	
	DEBUG_COMMAND("LE %d,%s,%d,%d:\n",type, filename, _var, num);
}

void commandLL() {
	int type = sl_getc();
	int link_no = getCaliValue();
	int *var, _var = 0;
	int num, i;
	dridata *dfile = ald_getdata(DRIFILE_DATA, link_no - 1);
	WORD *data;
	
	if (dfile == NULL) {
		getCaliValue();
		getCaliValue();
		sysVar[0] = 255;
		return;
	}
	
	data = (WORD *)dfile->data;
	
	switch(type) {
	case 0: /* T2 */
		var = getCaliVariable();
		num  = getCaliValue();

		DEBUG_COMMAND("LL %d,%d,%d,%d:\n",type, link_no, _var, num);
		
		if (dfile->size < num * sizeof(WORD)) {
			WARNING("data shortage (req = %d, result = %d)\n", num, dfile->size/ sizeof(WORD));
			/* sysVar[0] = 254; 大嘘*/
			/* return; */
			num = dfile->size / sizeof(WORD);
		}
		for (i = 0; i < num; i++) {
			var[i] = SDL_SwapLE16(data[i]);
		}
		break;
		
	case 1:
		_var = getCaliValue();
		num  = getCaliValue();
		DEBUG_COMMAND_YET("LL1 not yet %d, %d\n", _var, num);
		sysVar[0] = 255;
		goto out;
		break;
		
	default:
		WARNING("Unknown LL command %d\n", type);
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
	DEBUG_COMMAND("LHD %d,%d:\n",p1,no);
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
	DEBUG_COMMAND("LHG %d,%d:\n",p1,no);
}

void commandLHM() {
	// ＣＤのデータをＨＤＤへ登録／削除する
	int p1 = sl_getc();
	int no = getCaliValue();
	// X版では全てをHDDに置くのでサポートしない
	
	sysVar[0] = 255;
	DEBUG_COMMAND("LHM %d,%d:\n",p1,no);
}

void commandLHS() {
	// ＣＤのデータをＨＤＤへ登録／削除する
	int p1 = sl_getc();
	int no = getCaliValue();
	// X版では全てをHDDに置くのでサポートしない
	
	sysVar[0] = 255;
	DEBUG_COMMAND("LHS %d,%d:\n",p1,no);
}

void commandLHW() {
	// ＣＤのデータをＨＤＤへ登録／削除する
	int p1 = sl_getc();
	int no = getCaliValue();
	// X版では全てをHDDに置くのでサポートしない
	
	sysVar[0] = 255;
	DEBUG_COMMAND("LHW %d,%d:\n",p1,no);
}

void commandLC() {
	int x = getCaliValue();
	int y = getCaliValue();
	const char *filename = sl_getString(':');
	
	char *fname_utf8 = toUTF8(filename);
	sysVar[0] = cg_load_with_filename(fname_utf8, x, y);
	free(fname_utf8);
	
	DEBUG_COMMAND("LC %d,%d,%s:\n", x, y, filename);
}

void commandLXG() {
	/* ファイルを選択する */
	int file_name = getCaliValue();
	const char *title = sl_getString(':');
	const char *filter = sl_getString(':');

	DEBUG_COMMAND_YET("LXG %d,%s,%s:\n", file_name, title, filter);
}

void commandLXO() {
	/* ファイルを作成またはオープンする */
	int num           = getCaliValue();
	int file_name     = getCaliValue();
	int how_to_create = getCaliValue();

	sysVar[0]=255;
	DEBUG_COMMAND_YET("LXO %d,%d,%d:\n",num,file_name,how_to_create);
}

void commandLXC() {
	/* ファイルをクローズする */
	int num = getCaliValue();
	
	DEBUG_COMMAND_YET("LXC %d:\n",num);
}

void commandLXL() {
	/* CGファイルロード */
	int x         = getCaliValue();
	int y         = getCaliValue();
	int file_name = getCaliValue();
	
	DEBUG_COMMAND_YET("LXL %d,%d,%d:\n",x,y,file_name);
}

void commandLXS() {
	/* ファイルサイズ(バイト数)を取得する */
	int num = getCaliValue();
	int *hi = getCaliVariable();
	int *lo = getCaliVariable();
	
	DEBUG_COMMAND_YET("LXS %d,%d,%d:\n", num, *hi, *lo);
}

void commandLXP() {
	/* ファイルポインタの位置を(先頭からのバイト数)設定する */
	int num = getCaliValue();
	int hi  = getCaliValue();
	int lo  = getCaliValue();
	
	DEBUG_COMMAND_YET("LXP %d,%d,%d:\n",num,hi,lo);
}

void commandLXR() {
	/* ファイルからデータを読み取る */
	int num  = getCaliValue();
	int *var = getCaliVariable();
	int size = getCaliValue();
	
	DEBUG_COMMAND_YET("LXR %d,%d,%d:\n",num,*var,size);
}

void commandLXW() {
	/* ファイルにデータを書き込む */
	int num  = getCaliValue();
	int *var = getCaliVariable();
	int size = getCaliValue();
	
	DEBUG_COMMAND_YET("LXW %d,%d,%d:\n",num,*var,size);
}
