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
#include "system.h"
#include "nact.h"
#include "menu.h"
#include "editor.h"
#include "gfx_private.h"
#ifdef _WIN32
#include "win/dialog.h"
#endif

void menu_open(void) {
	return;
}

void menu_quitmenu_open(void) {
	const SDL_MessageBoxButtonData buttons[] = {
		{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Quit" },
		{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "Cancel" },
	};
	const SDL_MessageBoxData messagebox_data = {
		.flags = SDL_MESSAGEBOX_INFORMATION,
		.window = gfx_window,
		.title = "Confirm",
		.message = "Quit xsystem35?",
		.numbuttons = SDL_arraysize(buttons),
		.buttons = buttons,
	};
	int buttonid = 0;
	if (SDL_ShowMessageBox(&messagebox_data, &buttonid) < 0) {
		WARNING("error displaying message box");
		return;
	}
	if (buttonid == 1) {
		nact_quit(false);
	}
}

void menu_resetmenu_open(void) {
	const SDL_MessageBoxButtonData buttons[] = {
		{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Restart" },
		{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "Cancel" },
	};
	const SDL_MessageBoxData messagebox_data = {
		.flags = SDL_MESSAGEBOX_INFORMATION,
		.window = gfx_window,
		.title = "Confirm",
		.message = "Restart the game?",
		.numbuttons = SDL_arraysize(buttons),
		.buttons = buttons,
	};
	int buttonid = 0;
	if (SDL_ShowMessageBox(&messagebox_data, &buttonid) < 0) {
		WARNING("error displaying message box");
		return;
	}
	if (buttonid == 1) {
		nact_quit(true);
	}
}

bool menu_inputstring(INPUTSTRING_PARAM *p) {
#ifdef _WIN32
	return input_string(p);
#else
	p->newstring = p->oldstring;
	return true;
#endif
}

bool menu_inputstring2(INPUTSTRING_PARAM *p) {
	return edit_string(p);
}

bool menu_inputnumber(INPUTNUM_PARAM *p) {
#ifdef _WIN32
	return input_number(p);
#else
	p->value = p->def;
	return true;
#endif
}

void menu_init(void) {
#ifdef _WIN32
	win_menu_init();
#endif
}

void menu_gtkmainiteration() {
	return;
}

#ifndef _WIN32
void menu_setSkipState(bool enabled, bool activated) {
}
#endif
