/*
 * Copyright (C) 2020 <KichikuouChrome@gmail.com>
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

#include <SDL.h>

#include "system.h"
#include "portab.h"
#include "menu.h"
#include "nact.h"

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
		.window = NULL,
		.title = "Quit game?",
		.message = "Any unsaved progress will be lost.",
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

void menu_init(void) {
	return;
}

void menu_render_overlay(void) {
	return;
}

void menu_setSkipState(bool enabled, bool activated) {
}
