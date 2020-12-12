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
#include <windows.h>
#undef min
#undef max
#include <SDL_syswm.h>
#include "system.h"
#include "menu.h"
#include "sdl_private.h"
#include "resources.h"
#include "ags.h"

static HWND get_hwnd(SDL_Window *window) {
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	SDL_GetWindowWMInfo(window, &info);
	return info.info.win.window;
}

void win_menu_init(void) {
	HINSTANCE hinst = (HINSTANCE)GetModuleHandle(NULL);
	HMENU hmenu = LoadMenu(hinst, MAKEINTRESOURCE(IDR_MENU1));
	SetMenu(get_hwnd(sdl_window), hmenu);
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
	// Let SDL recalc the window size, taking menu height into account.
	SDL_SetWindowSize(sdl_window, view_w, view_h);
}

void win_menu_onsyswmevent(SDL_SysWMmsg* msg) {
	switch (msg->msg.win.msg) {
	case WM_COMMAND:
		switch (msg->msg.win.wParam) {
		case ID_EXIT:
			menu_quitmenu_open();
			break;
		case ID_SCREEN_WINDOW:
			ags_fullscreen(FALSE);
			break;
		case ID_SCREEN_FULL:
			ags_fullscreen(TRUE);
			break;
		}
		break;
	}
}
