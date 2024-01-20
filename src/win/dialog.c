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
#include "menu.h"
#include "sdl_private.h"
#include "resources.h"

static HWND get_hwnd(SDL_Window *window) {
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	SDL_GetWindowWMInfo(window, &info);
	return info.info.win.window;
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
