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
#include <time.h>
#include <SDL_syswm.h>
#include "system.h"
#include "menu.h"
#include "sdl_core.h"
#include "sdl_private.h"
#include "resources.h"
#include "msgskip.h"

static HWND get_hwnd(SDL_Window *window) {
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	SDL_GetWindowWMInfo(window, &info);
	return info.info.win.window;
}

static void saveScreenshot(void) {
	char pathbuf[MAX_PATH];
	time_t t = time(NULL);
	struct tm *lt = localtime(&t);
	strftime(pathbuf, sizeof(pathbuf), "xsystem35-%Y%m%d-%H%M%S.bmp", lt);

	OPENFILENAME ofn = {
		.lStructSize = sizeof(OPENFILENAME),
		.hwndOwner = get_hwnd(sdl_window),
		.lpstrFilter = "Bitmap files (*.bmp)\0*.bmp\0All files (*.*)\0*.*\0",
		.lpstrFile = pathbuf,
		.nMaxFile = MAX_PATH,
		.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR,
	};
	if (!GetSaveFileName(&ofn))
		return;
	if (SDL_SaveBMP(sdl_display, pathbuf) != 0) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "xsystem35",
								 SDL_GetError(), sdl_window);
		SDL_ClearError();
	}
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
		case ID_SCREENSHOT:
			saveScreenshot();
			break;
		case ID_EXIT:
			menu_quitmenu_open();
			break;
		case ID_SCREEN_WINDOW:
			sdl_setFullscreen(FALSE);
			break;
		case ID_SCREEN_FULL:
			sdl_setFullscreen(TRUE);
			break;
		case ID_MSGSKIP:
			msgskip_activate(!msgskip_isActivated());
			break;
		}
		break;
	}
}

void menu_setSkipState(boolean enabled, boolean activated) {
	HWND hwnd = get_hwnd(sdl_window);
	HMENU hmenu = GetMenu(hwnd);

	EnableMenuItem(hmenu, ID_MSGSKIP, enabled ? MF_ENABLED : MF_GRAYED);

	wchar_t buf[20];
	MENUITEMINFOW menuitem = {
		.cbSize = sizeof(MENUITEMINFOW),
		.fMask = MIIM_STRING,
		.dwTypeData = buf,
		.cch = sizeof(buf) / sizeof(wchar_t),
	};
	if (GetMenuItemInfoW(hmenu, ID_MSGSKIP, false, &menuitem)) {
		wchar_t *p = wcsrchr(buf, L'[');
		if (p) {
			wcscpy(p, activated ? L"[on]" : L"[off]");
			SetMenuItemInfoW(hmenu, ID_MSGSKIP, false, &menuitem);
		}
	}

	DrawMenuBar(hwnd);
}
