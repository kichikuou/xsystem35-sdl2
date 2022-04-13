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
#include <shlobj.h>
#undef min
#undef max
#include <SDL_syswm.h>
#include "menu.h"
#include "sdl_private.h"
#include "resources.h"

#define REGVAL_RECENT_FOLDER "RecentFolder"

static HWND get_hwnd(SDL_Window *window) {
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	SDL_GetWindowWMInfo(window, &info);
	return info.info.win.window;
}

static int CALLBACK select_game_callback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData) {
	if (uMsg == BFFM_INITIALIZED)
		SendMessage(hwnd, BFFM_SETSELECTION, (WPARAM)TRUE, lpData);
	return 0;
}

boolean current_folder_has_ald(void) {
	WIN32_FIND_DATA find_data;
	HANDLE hFind = FindFirstFile("*.ALD", &find_data);
	if (hFind == INVALID_HANDLE_VALUE)
		return FALSE;
	FindClose(hFind);
	return TRUE;
}

boolean select_game_folder(void) {
	char title[256];
	LoadString(GetModuleHandle(NULL), IDS_CHOOSE_GAME_FOLDER, title, sizeof(title));

	char path[MAX_PATH] = "";
	DWORD value_size = sizeof(path);
	RegGetValue(HKEY_CURRENT_USER, XSYSTEM35_REGKEY, REGVAL_RECENT_FOLDER,
				RRF_RT_REG_SZ, NULL, path, &value_size);

	BROWSEINFO bi = {
		.lpszTitle = title,
		.ulFlags = BIF_RETURNONLYFSDIRS,
		.lpfn = &select_game_callback,
		.lParam = (LPARAM)path,
	};
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if (!pidl)
		return FALSE;
	SHGetPathFromIDList(pidl, path);
	CoTaskMemFree(pidl);
	if (!SetCurrentDirectory(path))
		return FALSE;

	RegSetKeyValue(HKEY_CURRENT_USER, XSYSTEM35_REGKEY, REGVAL_RECENT_FOLDER,
				   REG_SZ, path, strlen(path) + 1);
	return TRUE;
}

static INT_PTR CALLBACK text_dialog_proc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static INPUTSTRING_PARAM *p;
	static char utf8string[256];
	wchar_t wstring[64];
	int len;

	switch (msg) {
	case WM_CLOSE:
		EndDialog(hDlg, IDCANCEL);
		break;

	case WM_INITDIALOG:
		p = (INPUTSTRING_PARAM *)lParam;
		if (MultiByteToWideChar(CP_UTF8, 0, p->title, -1, wstring, 64))
			SetWindowTextW(GetDlgItem(hDlg, IDC_TEXT), wstring);
		if (MultiByteToWideChar(CP_UTF8, 0, p->oldstring, -1, wstring, 64))
			SetWindowTextW(GetDlgItem(hDlg, IDC_EDITBOX), wstring);
		EnableWindow(GetDlgItem(hDlg, IDOK), p->oldstring[0] != '\0');
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_EDITBOX:
			GetDlgItemTextW(hDlg, IDC_EDITBOX, wstring, 64);
			len = wcslen(wstring);
			EnableWindow(GetDlgItem(hDlg, IDOK), 0 < len && len <= p->max);
			break;
		case IDOK:
			GetDlgItemTextW(hDlg, IDC_EDITBOX, wstring, 64);
			WideCharToMultiByte(CP_UTF8, 0, wstring, -1, utf8string, sizeof(utf8string), NULL, NULL);
			p->newstring = utf8string;
			EndDialog(hDlg, IDOK);
			break;
		default:
			return FALSE;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}

boolean input_string(INPUTSTRING_PARAM *p) {
	p->newstring = p->oldstring;
	DialogBoxParam(GetModuleHandle(NULL),
				   MAKEINTRESOURCE(IDD_DIALOG1),
				   get_hwnd(sdl_window),
				   text_dialog_proc,
				   (LPARAM)p);
	return TRUE;
}

boolean input_number(INPUTNUM_PARAM *p) {
	// TODO: Implement
	p->value = p->def;
	return TRUE;
}
