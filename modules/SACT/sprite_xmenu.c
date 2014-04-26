/*
 * sprite_xmenu.c: XMenuXXX
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
/* $Id: sprite_xmenu.c,v 1.2 2003/07/14 16:22:51 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>

#include "portab.h"
#include "nact.h"
#include "variable.h"
#include "sact.h"

struct _xm {
	char *title;
	int ret[SEL_ELEMENT_MAX];
	char *item[SEL_ELEMENT_MAX];
};

static struct _xm xmenu;

int spxm_clear(void) {
	int i;
	
	free(xmenu.title); xmenu.title = NULL;
	for (i = 0; i < SEL_ELEMENT_MAX; i++) {
		free(xmenu.item[i]); xmenu.item[i] = NULL;
		xmenu.ret[i] = 0;
	}
	return OK;
}

int spxm_register(int reginum, int menuid) {
	if (reginum > SEL_ELEMENT_MAX) return NG;

	xmenu.ret[reginum] = menuid;
	xmenu.item[reginum] = strdup(sact.msgbuf);
	sact.msgbuf[0] = '\0';
	return OK;
}

int spxm_getnum(int reginum) {
	if (reginum > SEL_ELEMENT_MAX) return 0;
	return xmenu.ret[reginum];
}

int spxm_gettext(int reginum, int strno) {
	if (reginum > SEL_ELEMENT_MAX) return NG;
	v_strcpy(strno -1, xmenu.item[reginum]);
	return OK;
}

int spxm_titlereg(void) {
	xmenu.title = strdup(sact.msgbuf);
	sact.msgbuf[0] = '\0';
	return OK;
}

int spxm_titleget(int strno) {
	v_strcpy(strno -1, xmenu.title);
	return OK;
}
