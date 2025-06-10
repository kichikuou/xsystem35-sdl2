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
#include "nact.h"
#include "sdl_core.h"
#include "gfx_private.h"
#include "resources.h"
#include "msgskip.h"
#include "texthook.h"

static HMENU hmenu;

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
		.hwndOwner = get_hwnd(gfx_window),
		.lpstrFilter = "Bitmap files (*.bmp)\0*.bmp\0All files (*.*)\0*.*\0",
		.lpstrFile = pathbuf,
		.nMaxFile = MAX_PATH,
		.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR,
	};
	if (!GetSaveFileName(&ofn))
		return;
	if (!save_screenshot(pathbuf)) {
		sdl_showMessageBox(MESSAGEBOX_ERROR, "xsystem35", SDL_GetError());
		SDL_ClearError();
	}
}

static bool toggle_menu_item(UINT id, bool *checked_out) {
	MENUITEMINFO menuitem = {
		.cbSize = sizeof(MENUITEMINFO),
		.fMask = MIIM_STATE,
	};
	if (!GetMenuItemInfo(hmenu, id, false, &menuitem))
		return false;

	if (menuitem.fState & MFS_CHECKED) {
		CheckMenuItem(hmenu, id, MF_BYCOMMAND | MFS_UNCHECKED);
		*checked_out = false;
	} else {
		CheckMenuItem(hmenu, id, MF_BYCOMMAND | MFS_CHECKED);
		*checked_out = true;
	}
	return true;
}

void win_menu_init(void) {
	HINSTANCE hinst = (HINSTANCE)GetModuleHandle(NULL);
	hmenu = LoadMenu(hinst, MAKEINTRESOURCE(IDR_MENU1));
	SetMenu(get_hwnd(gfx_window), hmenu);
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
	// Let SDL recalc the window size, taking menu height into account.
	SDL_SetWindowSize(gfx_window, view_w, view_h);
	CheckMenuItem(hmenu, ID_OPTION_MOUSE_MOVE, MF_BYCOMMAND | MFS_CHECKED);
}

void win_menu_onSysWMEvent(SDL_SysWMmsg* msg) {
	bool checked;
	switch (msg->msg.win.msg) {
	case WM_COMMAND:
		switch (msg->msg.win.wParam) {
		case ID_SCREENSHOT:
			saveScreenshot();
			break;
		case ID_RESTART:
			menu_resetmenu_open();
			break;
		case ID_EXIT:
			menu_quitmenu_open();
			break;
		case ID_SCREEN_WINDOW:
			gfx_setFullscreen(false);
			SetMenu(get_hwnd(gfx_window), hmenu);
			break;
		case ID_SCREEN_FULL:
			gfx_setFullscreen(true);
			SetMenu(get_hwnd(gfx_window), NULL);
			break;
		case ID_SCREEN_INTEGER_SCALING:
			if (toggle_menu_item(ID_SCREEN_INTEGER_SCALING, &checked))
				gfx_setIntegerScaling(checked);
			break;
		case ID_OPTION_MOUSE_MOVE:
			if (toggle_menu_item(ID_OPTION_MOUSE_MOVE, &checked))
				nact->ags.mouse_warp_enabled = checked;
			break;
		case ID_OPTION_AUTO_COPY:
			if (toggle_menu_item(ID_OPTION_AUTO_COPY, &checked))
				texthook_set_mode(checked ? TEXTHOOK_COPY : TEXTHOOK_NONE);
			break;
		case ID_MSGSKIP:
			msgskip_activate(!msgskip_isActivated());
			break;
		}
		break;
	}
}

void win_menu_onMouseMotion(int x, int y) {
	if (!gfx_isFullscreen())
		return;
	SetMenu(get_hwnd(gfx_window), y > 0 ? NULL : hmenu);
}

void menu_setSkipState(bool enabled, bool activated) {
	HWND hwnd = get_hwnd(gfx_window);

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
