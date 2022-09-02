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

	if (!v_allocatePage(page, maxindex + 1, !!save))
		WARNING("Array allocation failed: page=%d size=%d", page, maxindex);
	DEBUG_COMMAND("DC %d,%d,%d:", page, maxindex, save);
}

void commandDI() {
	int page      = getCaliValue();
	int *var_use  = getCaliVariable();
	int *var_size = getCaliVariable();
	
	v_getPageStatus(page, var_use, var_size);

	DEBUG_COMMAND("DI %d,%p,%p:", page, var_use, var_size);
}

void commandDS() {
	struct VarRef data_var;
	int *point_var = getCaliVariable();
	getCaliArray(&data_var);
	int offset     = getCaliValue();
	int page       = getCaliValue();
	
	DEBUG_COMMAND("DS %p,%d,%d,%d:",point_var, data_var.var, offset, page);
	if (!v_bindArray(data_var.var, point_var, offset, page)) {
		WARNING("commandDS(): Array allocate failed");
		WARNING("if you are playing 'Pastel Chime', please patch to scenario(see patch/README.TXT for detail)");
	}
}


void commandDR() {
	struct VarRef data_var;
	getCaliArray(&data_var);
	v_unbindArray(data_var.var);
	
	DEBUG_COMMAND("DR %d:", data_var.var);
}

void commandDF() {
	struct VarRef data_var;
	getCaliArray(&data_var);
	int cnt  = getCaliValue();
	int data = getCaliValue();
	
	DEBUG_COMMAND("DF %p,%d,%d:", data_var, cnt, data);

	if (data_var.page) {
		int maxlen = v_sliceSize(&data_var);
		if (cnt > maxlen) {
			WARNING("%03d:%05x: count exceeds array boundary (%d > %d)", sl_getPage(), sl_getIndex(), cnt, maxlen);
			cnt = maxlen;
		}
	}

	int *p = v_resolveRef(&data_var);
	while (cnt--)
		*p++ = data;
}
