/*
 * menu_emscripten.c  menu implementation for Emscripten
 *
 * Copyright (C) 2017 <KichikuouChrome@gmail.com>
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
#include <emscripten.h>

#include "portab.h"
#include "menu.h"

void menu_open(void) {
	return;
}

void menu_quitmenu_open(void) {
	return;
}

boolean menu_inputstring(INPUTSTRING_PARAM *p) {
	static char buf[256];
	int ok = EM_ASM_({
			var r = xsystem35.shell.inputString(UTF8ToString($0), UTF8ToString($1), $2);
			if (r) {
				stringToUTF8(r, $3, $4);
				return 1;
			}
			return 0;
		}, p->title, p->oldstring, p->max, buf, sizeof buf);
	if (ok)
		p->newstring = buf;
	else
		p->newstring = p->oldstring;
	return ok ? TRUE : FALSE;
}

boolean menu_inputstring2(INPUTSTRING_PARAM *p) {
	p->newstring = p->oldstring;
	return TRUE;
}

boolean menu_inputnumber(INPUTNUM_PARAM *p) {
	p->value = EM_ASM_({
			return xsystem35.shell.inputNumber(UTF8ToString($0), $1, $2, $3);
		}, p->title, p->min, p->max, p->def);
	return TRUE;
}

EM_JS(void, menu_msgbox_open, (char *msg), {
	window.alert(UTF8ToString(msg));
});

void menu_init(void) {
	return;
}

void menu_gtkmainiteration() {
	return;
}

EM_JS(void, menu_setSkipState, (boolean enabled, boolean activated), {
	xsystem35.shell.setSkipButtonState(enabled, activated);
});
