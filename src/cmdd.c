/*
 * cmdd.c  SYSTEM35 D command
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
/* $Id: cmdd.c,v 1.9 2002/04/29 05:48:21 chikama Exp $ */

#include <stdio.h>
#include "portab.h"
#include "xsystem35.h"
#include "scenario.h"

void commandDC() {
	int page = getCaliValue();
	int maxindex = getCaliValue();
	int save = getCaliValue();
	boolean bool;
	
	bool = v_allocateArrayBuffer(page, maxindex + 1, save == 0 ? false : true);
	if(!bool) {
		WARNING("commandDC(): Array allocate failed\n");
	}
	DEBUG_COMMAND("DC %d,%d,%d:\n", page, maxindex, save);
}

void commandDI() {
	int page      = getCaliValue();
	int *var_use  = getCaliVariable();
	int *var_size = getCaliVariable();
	
	*var_use = v_getArrayBufferStatus(page);
	*var_size = arrayVarBuffer[page - 1].size & 0xffff;

	DEBUG_COMMAND("DI %d,%p,%p:\n", page, var_use, var_size);
}

void commandDS() {
	int *point_var = getCaliVariable();
	int *data_var  = getCaliVariable();
	int varno      = preVarNo;
	int offset     = getCaliValue();
	int page       = getCaliValue();
	boolean bool;
	
	DEBUG_COMMAND("DS %p,%p,%d,%d:\n",point_var, data_var, offset, page);
	bool = v_defineArrayVar(varno, point_var, offset, page);
	if (!bool) {
		WARNING("commandDS(): Array allocate failed\n");
		WARNING("if you are playing 'Pastel Chime', please patch to scenario(see patch/README.TXT for detail)\n");
	}
}


void commandDR() {
	int *data_var = getCaliVariable();
	v_releaseArrayVar(preVarNo);
	
	DEBUG_COMMAND("DR %p:\n", data_var);
}

void commandDF() {
	int *data_var = getCaliVariable();
	int varno     = preVarNo;
	int cnt       = getCaliValue();
	int data      = getCaliValue();
	
	DEBUG_COMMAND("DF %p,%d,%d:\n", data_var, cnt, data);

	const arrayVarStruct* array = &sysVarAttribute[varno];
	if (array->page) {
		int maxlen = arrayVarBuffer[array->page - 1].size - array->offset - *array->pointvar;
		if (cnt > maxlen) {
			WARNING("%03d:%05x: count exceeds array boundary (%d > %d)\n", sl_getPage(), sl_getIndex(), cnt, maxlen);
			cnt = maxlen;
		}
	}

	while (cnt--) {
		*data_var = data; data_var++;
	}
}
