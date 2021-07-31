/*
 * cmd2F60.c  SYSTEM35 DLL call
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
/* $Id: cmd2F60.c,v 1.11 2003/01/12 10:48:50 chikama Exp $ */

#include <stdio.h>

#include "portab.h"
#include "system.h"
#include "nact.h"
#include "s39ain.h"
#include "xsystem35.h"
#include "scenario.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define dll  nact->ain.dll

typedef void *entrypoint (void);

EMSCRIPTEN_KEEPALIVE  // Prevent inlining, because this function is listed in ASYNCIFY_ADD
void commands2F60() {
	int type = sl_getdw();  /* DLL type */
	int fnum = sl_getdw();  /* function number */

	if (dll == NULL) {
		SYSERROR("No DLL initilized\n");
	}

	if (dll + type == NULL) goto eexit;

	if (dll[type].function_num < fnum) goto eexit;

	if (dll[type].function[fnum].entrypoint == NULL) goto eexit;

	dll[type].function[fnum].entrypoint();
	return;
	
 eexit:
	SYSERROR("Can't continue further scenario.(%d,%d)(%s,%s)\n", type, fnum, dll[type].name, dll[type].function[fnum].name);
}
