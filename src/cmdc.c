/*
 * cmdc.c  SYSTEM35 C command
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
/* $Id: cmdc.c,v 1.31 2006/04/21 16:40:48 chikama Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include "portab.h"
#include "xsystem35.h"
#include "ags.h"

void commandCC() {
	int src_x  = getCaliValue();
	int src_y  = getCaliValue();
	int width  = getCaliValue();
	int height = getCaliValue();
	int dst_x  = getCaliValue();
	int dst_y  = getCaliValue();
	
	ags_copyArea(src_x, src_y, width, height, dst_x, dst_y);
	ags_updateArea(dst_x, dst_y, width, height);
	
	DEBUG_COMMAND("CC %d,%d,%d,%d,%d,%d:\n", src_x, src_y, width, height, dst_x, dst_y);
}

void commandCS() {
	int src_x  = getCaliValue();
	int src_y  = getCaliValue();
	int width  = getCaliValue();
	int height = getCaliValue();
	int dst_x  = getCaliValue();
	int dst_y  = getCaliValue();
	int sprite  = getCaliValue();
	
	ags_copyAreaSP(src_x, src_y, width, height, dst_x, dst_y, sprite);
	ags_updateArea(dst_x, dst_y, width, height);

	DEBUG_COMMAND("CS %d,%d,%d,%d,%d,%d,%d:\n", src_x, src_y, width, height, dst_x, dst_y, sprite);
}

void commandCX() {
	int mode   = getCaliValue();
	int src_x  = getCaliValue();
	int src_y  = getCaliValue();
	int width  = getCaliValue();
	int height = getCaliValue();
	int dst_x  = getCaliValue();
	int dst_y  = getCaliValue();
	int col    = getCaliValue();
	
	switch(mode) {
	case 0:
		ags_copyArea_shadow(src_x, src_y, width, height, dst_x, dst_y);
		ags_updateArea(dst_x, dst_y, width, height);
		break;
	case 1:
		ags_copyArea_transparent(src_x, src_y, width, height, dst_x, dst_y, col);
		ags_updateArea(dst_x, dst_y, width, height);
		break;
	case 2:
		ags_copyFromAlpha(src_x, src_y, width, height, dst_x, dst_y, col == 1 ? TO_16L : TO_16H);
		ags_updateArea(dst_x, dst_y, width, height);
		break;
	case 3:
		ags_copyToAlpha(src_x, src_y, width, height, dst_x, dst_y, col == 1 ? FROM_16L : FROM_16H);
		break;
	case 4:
		ags_copyFromAlpha(src_x, src_y, width, height, dst_x, dst_y, col == 1 ? TO_24R : col == 2 ? TO_24G : TO_24B);
		ags_updateArea(dst_x, dst_y, width, height);
		break;
	case 5:
		ags_copyToAlpha(src_x, src_y, width, height, dst_x, dst_y, col == 1 ? FROM_24R : col == 2 ? FROM_24G : FROM_24B);
		break;
	case 6:
		DEBUG_COMMAND_YET("CX %d,%d,%d,%d,%d,%d,%d,%d:\n", mode, src_x, src_y, width, height, dst_x, dst_y, col);
		break;
	}

	DEBUG_COMMAND("CX %d,%d,%d,%d,%d,%d,%d,%d:\n", mode, src_x, src_y, width, height, dst_x, dst_y, col);
}

void commandCM() {
	int src_x      = getCaliValue();
	int src_y      = getCaliValue();
	int src_width  = getCaliValue();
	int src_height = getCaliValue();
	int dst_x      = getCaliValue();
	int dst_y      = getCaliValue();
	int dst_width  = getCaliValue();
	int dst_height = getCaliValue();
	int mirror_sw  = getCaliValue();
	
	ags_scaledCopyArea(src_x, src_y, src_width, src_height, dst_x, dst_y, dst_width, dst_height, mirror_sw);
	ags_updateArea(dst_x, dst_y, dst_width, dst_height);
	
	DEBUG_COMMAND("CM %d,%d,%d,%d,%d,%d,%d,%d,%d:\n", src_x, src_y, src_width, src_height, dst_x, dst_y, dst_width, dst_height, mirror_sw);
}

void commandCE() {
	int src_x     = getCaliValue();
	int src_y     = getCaliValue();
	int width     = getCaliValue();
	int height    = getCaliValue();
	int dst_x     = getCaliValue();
	int dst_y     = getCaliValue();
	int effect_sw = getCaliValue();
	int option    = getCaliValue();
	int wait_flag = getCaliValue();
	
	ags_eCopyArea(src_x, src_y, width, height, dst_x, dst_y, effect_sw, option, wait_flag, -1); 
	
	if (wait_flag == 1) {
		sysVar[0] = nact->waitcancel_key;
	}
	
	switch(effect_sw) {
	case 53:
	case 54:
	case 1000:
	case 1001:
		DEBUG_COMMAND_YET("CE %d,%d,%d,%d,%d,%d,%d,%d,%d:\n", src_x, src_y, width, height, dst_x, dst_y, effect_sw, option, wait_flag);
		break;
	default:
		DEBUG_COMMAND("CE %d,%d,%d,%d,%d,%d,%d,%d,%d:\n", src_x, src_y, width, height, dst_x, dst_y, effect_sw, option, wait_flag);
	}
}

void commandCD() {
	int src_x     = getCaliValue();
	int src_y     = getCaliValue();
	int width     = getCaliValue();
	int height    = getCaliValue();
	int dst_x     = getCaliValue();
	int dst_y     = getCaliValue();
	int effect_sw = getCaliValue();
	int option    = getCaliValue();
	int wait_flag = getCaliValue();
	int color     = getCaliValue();
	
	ags_eCopyArea(src_x, src_y, width, height, dst_x, dst_y, effect_sw, option, wait_flag, color);
	
	if (wait_flag == 1) {
		sysVar[0] = nact->waitcancel_key;
	}
	
	switch(effect_sw) {
	case 6:
	case 7:
	case 8:
	case 9:
	case 24:
	case 25:
	case 26:
	case 27:
	case 28:
	case 29:
	case 30:
	case 31:
	case 1000:
	case 2000:
	case 2001:
		DEBUG_COMMAND_YET("CD %d,%d,%d,%d,%d,%d,%d,%d,%d,%d:\n", src_x, src_y, width, height, dst_x, dst_y, effect_sw, option, wait_flag, color);
		break;
	default:
		DEBUG_COMMAND("CD %d,%d,%d,%d,%d,%d,%d,%d,%d,%d:\n", src_x, src_y, width, height, dst_x, dst_y, effect_sw, option, wait_flag, color);
	}
}

void commandCK() {
	int no     = sys_getc();
	int x      = getCaliValue();
	int y      = getCaliValue();
	int width  = getCaliValue();
	int height = getCaliValue();
	int d1     = getCaliValue();
	int d2     = getCaliValue();
	int d3     = getCaliValue();
	int d4     = getCaliValue();
	
	switch(no) {
	case 1:
		ags_wrapColor(x, y, width, height, d1, d2); break;
	case 2:
		DEBUG_COMMAND_YET("CK %d,%d,%d,%d,%d,%d,%d,%d,%d:\n", no, x, y, width, height, d1, d2, d3, d4); break;
	case 3: /* from T2 */
		ags_changeColorArea(x, y, width, height, d1, d2, d3); break;
	default:
		WARNING("Unknown CK Command %d\n", no);
	}
	ags_updateArea(x, y, width, height);
	DEBUG_COMMAND("CK %d,%d,%d,%d,%d,%d,%d,%d,%d:\n", no, x, y, width, height, d1, d2, d3, d4);
}

void commandCL() {
	int x0    = getCaliValue();
	int y0    = getCaliValue();
	int x1    = getCaliValue();
	int y1    = getCaliValue();
	int color = getCaliValue();
	
	ags_drawLine(x0, y0, x1, y1, color);
	ags_updateArea(min(x0, x1), min(y0, y1), abs(x1 - x0) + 1, abs(y1 - y0) + 1); 
	
	DEBUG_COMMAND("CL %d,%d,%d,%d,%d:\n", x0, y0, x1, y1, color);
}

void commandCB() {
	int x      = getCaliValue();
	int y      = getCaliValue();
	int width  = getCaliValue();
	int height = getCaliValue();
	int color  = getCaliValue();
	
	ags_drawRectangle(x, y, width, height, color);
	ags_updateArea   (x, y, width, height);
	
	DEBUG_COMMAND("CB %d,%d,%d,%d,%d:\n", x, y, width, height, color);
}

void commandCF() {
	int x      = getCaliValue();
	int y      = getCaliValue();
	int width  = getCaliValue();
	int height = getCaliValue();
	int color  = getCaliValue();
	
	ags_fillRectangle(x, y, width, height, color);
	ags_updateArea   (x, y, width, height);

	DEBUG_COMMAND("CF %d,%d,%d,%d,%d:\n", x, y, width, height, color);
}

void commandCP() {
	int x     = getCaliValue();
	int y     = getCaliValue();
	int color = getCaliValue();

	MyRectangle* rec = ags_imageFlood(x, y, color);

	if (rec != NULL) {
		ags_updateArea(rec->x, rec->y, rec->width, rec->height);
	}

	DEBUG_COMMAND("CP %d,%d,%d:\n", x, y, color);
}

void commandCT() {
	/* (24bitのみ) 影データを取得する。*/
	int *var  = getCaliVariable();
	int x     = getCaliValue();
	int y     = getCaliValue();

	ags_alpha_getPixel(x, y, var);
	DEBUG_COMMAND("CT %d,%d,%d:\n", *var, x, y);
}

void commandCU() {
	/* (24bitのみ) 指定矩形領域内の影データのborder以下のデータをsetに変更する。*/
	int x      = getCaliValue();
	int y      = getCaliValue();
	int width  = getCaliValue();
	int height = getCaliValue();
	int border = getCaliValue();
	int set    = getCaliValue();
	
	ags_alpha_lowercut(x, y, width, height, border, set);
	DEBUG_COMMAND("CU %d,%d,%d,%d,%d,%d:\n", x, y, width, height, border, set);
}

void commandCV() {
	/* (24bitのみ) 指定矩形領域内の影データのborder以上のデータをsetに変更する。*/
	int x      = getCaliValue();
	int y      = getCaliValue();
	int width  = getCaliValue();
	int height = getCaliValue();
	int border = getCaliValue();
	int set    = getCaliValue();
	
	ags_alpha_uppercut(x, y, width, height, border, set);
	DEBUG_COMMAND("CV %d,%d,%d,%d,%d,%d:\n", x, y, width, height, border, set);
}

void commandCY() {
	/* (24bitのみ) 指定矩形領域内の影データを lv に設定する */
	int x  = getCaliValue();
	int y  = getCaliValue();
	int w  = getCaliValue();
	int h  = getCaliValue();
	int lv = getCaliValue();
	
	ags_alpha_setLevel(x, y, w, h, lv);
	DEBUG_COMMAND("CY %d,%d,%d,%d,%d:\n", x, y, w, h, lv);
}

void commandCZ() {
	/* (24bitのみ) 指定矩形領域内の影データを (dx,dy)に転送する */
	int sx = getCaliValue();
	int sy = getCaliValue();
	int w  = getCaliValue();
	int h  = getCaliValue();
	int dx = getCaliValue();
	int dy = getCaliValue();
	int op = getCaliValue();
	
	ags_alpha_copyArea(sx, sy, w, h, dx, dy);
	DEBUG_COMMAND("CZ %d,%d,%d,%d,%d,%d,%d:\n", sx, sy, w, h, dx, dy, op);
}
