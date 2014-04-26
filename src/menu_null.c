/*
 * menu_null.c  popup menu for null
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
/* $Id: menu_null.c,v 1.2 2003/01/25 01:34:50 chikama Exp $ */

#include "config.h"

#include <stdio.h>

#include "portab.h"
#include "menu.h"

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
	p->newstring = p->oldstring;
	return TRUE;
}

boolean menu_inputnumber(INPUTNUM_PARAM *p) {
	p->value = p->def;
	return TRUE;
}

void menu_msgbox_open(char *msg) {
	return;
}

void menu_init(void) {
	return;
}

void menu_widget_reinit(boolean reset_colortmap) {
	return;
}

void menu_gtkmainiteration() {
	return;
}
