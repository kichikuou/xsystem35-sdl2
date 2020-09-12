/*
 * menu_sdl.c  popup menu for SDL
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

#include "config.h"

#include <stdio.h>

#include "portab.h"
#include "nact.h"
#include "menu.h"
#include "sdl_core.h"
#include "sdl_private.h"

void menu_open(void) {
	return;
}

void menu_quitmenu_open(void) {
	return;
}

boolean menu_inputstring(INPUTSTRING_PARAM *p) {
	p->newstring = p->oldstring;
	return TRUE;
}

boolean menu_inputstring2(INPUTSTRING_PARAM *p) {
	return sdl_inputString(p);
}

boolean menu_inputnumber(INPUTNUM_PARAM *p) {
	p->value = p->def;
	return TRUE;
}

void menu_msgbox_open(char *msg) {
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, nact->game_title_utf8, msg, sdl_window);
}

void menu_init(void) {
	return;
}

void menu_gtkmainiteration() {
	return;
}
