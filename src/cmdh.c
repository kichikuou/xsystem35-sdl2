/*
 * cmdh.c  SYSTEM35 H command
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
/* $Id: cmdh.c,v 1.5 2000/09/10 10:39:48 chikama Exp $ */

#include <stdio.h>
#include <string.h>
#include "portab.h"
#include "xsystem35.h"
#include "hankaku.h"

void commandH() {
	int fig = sys_getc();
	int num = getCaliValue();
	char buf[512];

	sys_addMsg(format_number_zenkaku(num, fig, buf));

	DEBUG_COMMAND("H %d,%d:\n",fig,num);
}

void commandHH(void) {
	int fig = sys_getc();
	int num = getCaliValue();
	char buf[256];

	sys_addMsg(format_number(num, fig, buf));

	DEBUG_COMMAND("HH %d,%d:\n",fig,num);
}
