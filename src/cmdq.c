/*
 * cmdq.c  SYSTEM35 Q command
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
 *
*/
/* $Id: cmdq.c,v 1.12 2003/01/25 01:34:50 chikama Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include "portab.h"
#include "xsystem35.h"
#include "scenario.h"
#include "savedata.h"
#include "utfsjis.h"
#include "filecheck.h"
#include "cmd_check.h"

void commandQD() {
	/* 変数領域などのデータをセーブする。（全セーブ）*/
	int num   = getCaliValue();
	
	if (num <= 0) {
		sysVar[0] = 255;
	} else {
		sysVar[0] = save_saveAll(num - 1);
	}
	
	if (sysVar[0] > 200) WARNING("Failed to save (%d)", sysVar[0]);
	
	TRACE("QD %d:",num);
}

void commandQP() {
	/* 変数領域などのデータを一部セーブする。(数値変数部) */
	struct VarRef point;
	int num = getCaliValue();
	getCaliArray(&point);
	int cnt = getCaliValue();
	
	if (num <= 0) {
		sysVar[0] = 255;
	} else {
		sysVar[0] = save_savePartial(num - 1, &point, cnt);
	}
	
	if (sysVar[0] > 200) WARNING("Failed to save (%d)", sysVar[0]);
	
	TRACE("QP %d,%d,%d:", num, point.var, cnt);
}

void commandQC() {
	/* セーブファイルをnum2の領域からnum1の領域へコピー */
	int num1 = getCaliValue();
	int num2 = getCaliValue();
	
	if (num1 <= 0 || num2 <= 0) {
		sysVar[0] = 255;
	} else {
		sysVar[0] = save_copyAll(num1 - 1, num2 - 1);
	}
	
	if (sysVar[0] > 200) WARNING("Failed to save (%d)", sysVar[0]);
	
	TRACE("QC %d,%d:",num1,num2);
}

void commandQE(char terminator) {
	int type = sl_getc();
	const char *filename = sl_getString(terminator);
	char *fname_utf8 = toUTF8(filename);
	char *last_slash = NULL;
	for (char *p = fname_utf8; *p; p++) {
		if (*p == '\\') {
			*p = '/';
			last_slash = p;
		}
	}
	if (last_slash) {
		*last_slash = '\0';
		mkdir_p(fname_utf8);
		*last_slash = '/';
	}

	int var, cnt;
	struct VarRef vref;
	switch (type) {
	case 0:
		getCaliArray(&vref);
		var = vref.var;
		cnt = getCaliValue();
		sysVar[0] = save_vars_to_file(fname_utf8, &vref, cnt);
		break;
	case 1:
		var = getCaliValue();
		cnt = getCaliValue();
		sysVar[0] = save_strs_to_file(fname_utf8, var, cnt);
		break;
	default:
		var = getCaliValue();
		cnt = getCaliValue();
		WARNING("Unknown QE command %d", type);
		break;
	}
	free(fname_utf8);
	if (sysVar[0] > 200) WARNING("Failed to save (%d)", sysVar[0]);
	
	TRACE("QE %d,%s,%d,%d:", type, filename, var, cnt);
}
