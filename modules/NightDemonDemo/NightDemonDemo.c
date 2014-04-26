/*
 * NightDemonDemo.c: 
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
/* $Id: NightDemonDemo.c,v 1.2 2003/09/05 14:15:49 chikama Exp $ */

#include "config.h"

#include <stdio.h>

#include "portab.h"
#include "system.h"
#include "xsystem35.h"
#include "nact.h"

extern void ndd_init(char *files[], int n);
extern void ndd_run(int demonum);

void Init() {
	int p1 = getCaliValue(); /* ISys3x */
	int p2 = getCaliValue(); /* IWinMsg */
	int p3 = getCaliValue(); /* ITimer */
	int *var = getCaliVariable();
	
	ndd_init(nact->files.alk, 4);
	*var = 1;
	
	DEBUG_COMMAND("NightDemonDemo.Init %d,%d,%d,%p:\n", p1, p2, p3, var);
}

void Run() {
	int p1 = getCaliValue(); // デモ番号 0,1,2
	int p2 = getCaliValue();
	
	ndd_run(p1);
	
	DEBUG_COMMAND("NightDemonDemo.Run %d,%d:\n", p1, p2);
}
