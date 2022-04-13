/*
 * menu.h  popup menu
 *
 * Copyright (C) 2000- Masaki Chikama (Wren) <masaki-c@is.aist-nara.ac.jp>
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
/* $Id: menu.h,v 1.6 2001/04/21 17:03:22 chikama Exp $ */

#ifndef __MENU__
#define __MENU__

#include "portab.h"

/* 文字列入力のパラメータ */
typedef struct inputstring_param {
	char *title;
	char *oldstring;
	const char *newstring;
	int   max;
	/* for MJ cmd */
	boolean need_window;
	int     x, y, h;
} INPUTSTRING_PARAM;

/* 数値入力のパラメータ */
typedef struct inputnum_param {
	char *title;
	int value;
	int def;
	int max;
	int min;
} INPUTNUM_PARAM;

extern void menu_open(void);
extern void menu_quitmenu_open(void);
extern boolean menu_inputstring(INPUTSTRING_PARAM *);
extern boolean menu_inputstring2(INPUTSTRING_PARAM *);
extern boolean menu_inputnumber(INPUTNUM_PARAM *);
extern void menu_msgbox_open(char *);
extern void menu_widgetinit(void);
extern void menu_init();
extern void menu_gtkmainiteration();
extern void menu_setSkipState(boolean enabled, boolean activated);

#ifdef _WIN32
struct SDL_SysWMmsg;
extern void win_menu_init(void);
extern void win_menu_onsyswmevent(struct SDL_SysWMmsg* msg);
#endif

#endif /* !__MENULL */
