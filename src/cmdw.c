/*
 * cmdw.c  SYSTEM35 W command
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
/* $Id: cmdw.c,v 1.15 2003/01/25 01:34:50 chikama Exp $ */

#include <stdio.h>
#include "portab.h"
#include "xsystem35.h"
#include "ags.h"

void commandWW() {
	int x_size,y_size,color;
	x_size = getCaliValue();
	y_size = getCaliValue();
	color  = getCaliValue();
	
	ags_setWorldSize(x_size, y_size, color);
	
        DEBUG_COMMAND("WW %d,%d,%d:\n",x_size, y_size, color);
}

void commandWV() {
	int start_x, start_y, size_x, size_y;
	start_x = getCaliValue();
	start_y = getCaliValue();
	size_x  = getCaliValue();
	size_y  = getCaliValue();
	
	ags_setViewArea(start_x, start_y, size_x, size_y);
	ags_updateFull();
	
	DEBUG_COMMAND("WV %d,%d,%d,%d:\n",start_x,start_y,size_x, size_y);
}

void commandWZ() {
	int p1 = sys_getc();
	int sw = getCaliValue();

	switch(p1) {
	case 0:
		ags_setExposeSwitch(sw == 0 ? FALSE : TRUE);
		if (sw == 1) {
			/* う〜ん こんな処理いれなあかんのかぁ〜 (T_T) */
			if (nact->sys_world_depth == 8) nact->sys_pal_changed = TRUE;
			ags_updateFull();
		}
		break;
	case 1:
		ags_setExposeSwitch(sw == 0 ? FALSE : TRUE);
		break;
	default:
		break;
	}
	DEBUG_COMMAND("WZ %d,%d:\n", p1, sw);
}

void commandWX() {
	int x0, y0, cx, cy;
	x0 = getCaliValue();
	y0 = getCaliValue();
	cx = getCaliValue();
	cy = getCaliValue();
	
	ags_setExposeSwitch(TRUE);
	ags_updateArea(x0, y0, cx, cy);
	
	DEBUG_COMMAND("WX %d,%d,%d,%d:\n", x0, y0, cx, cy);
}
