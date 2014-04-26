/*
 * cmdg.c  SYSTEM35 G command
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
/* $Id: cmdg.c,v 1.33 2001/09/16 15:59:11 chikama Exp $ */

#include "portab.h"
#include "xsystem35.h"
#include "cg.h"

void commandG0() {
	int num = getCaliValue();
	
	if (num == 0) {
		WARNING("G0 num: num = 0!!\n");
		cg_clear_display_loc();
		return;
	}
	
	cg_load(num - 1, -1);
	
	DEBUG_COMMAND("G %d:\n",num);
}

void commandG1() {
	int num = getCaliValue();
	int sprite = getCaliValue();
	
	if (num == 0) {
		WARNING("G1 num: num = 0!!\n");
		cg_clear_display_loc();
		return;
	}
	
	cg_load(num - 1, sprite);
	
	DEBUG_COMMAND("G %d,%d:\n",num,sprite);
}

void commandGS() {
        /* num 番にリンクされているＣＧの座標とサイズを取得する */
	MyRectangle r;
	int num = getCaliValue();
	int *var = getCaliVariable();

	if (num == 0) {
		WARNING("GS num: num = 0!!\n");
		return;
	}
	
	cg_get_info(num - 1, &r);
	var[0] = r.x;
	var[1] = r.y;
	var[2] = r.width;
	var[3] = r.height;
	
	DEBUG_COMMAND("GS %d,%p:\n",num,var);
}

void commandGX() {
	int cg_num = getCaliValue();
	int shadow_num = getCaliValue();

	if (shadow_num == 0) {
		WARNING("GX shadow_now = 0!!\n");
		return;
	}
	
	cg_load_with_alpha(cg_num -1, shadow_num -1);
	
	DEBUG_COMMAND("GX %d,%d:\n",cg_num, shadow_num);
}
