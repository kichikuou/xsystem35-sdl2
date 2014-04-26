/*
 * cmdj.c  SYSTEM35 J command
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
/* $Id: cmdj.c,v 1.7 2000/09/20 10:33:15 chikama Exp $ */

#include <stdio.h>
#include "portab.h"
#include "xsystem35.h"
#include "cg.h"

void commandJ0() {
	int x = getCaliValue();
	int y = getCaliValue();
	
	cg_set_display_location(x, y, OFFSET_ABSOLUTE_GC);
	DEBUG_COMMAND("J0 %d,%d:\n", x, y);
}

void commandJ1() {
	int x = getCaliValue();
	int y = getCaliValue();
	
	cg_set_display_location(x, y, OFFSET_RELATIVE_GC);
	DEBUG_COMMAND("J1 %d,%d:\n", x, y);
}

void commandJ2() {
	int x = getCaliValue();
	int y = getCaliValue();

	cg_set_display_location(x, y, OFFSET_ABSOLUTE_JC);
	DEBUG_COMMAND("J2:%d,%d\n", x, y);
}

void commandJ3() {
	int x = getCaliValue();
	int y = getCaliValue();
	
	cg_set_display_location(x, y, OFFSET_RELATIVE_JC);
	DEBUG_COMMAND("J3:%d,%d\n", x, y);
}

void commandJ4() {
	cg_set_display_location(0, 0, OFFSET_NOMOVE);
	DEBUG_COMMAND("J4 :\n");
}
