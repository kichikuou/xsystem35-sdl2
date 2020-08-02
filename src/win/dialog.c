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
#include "resources.h"

#define REGVAL_RECENT_FOLDER "RecentFolder"

static int CALLBACK select_game_callback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData) {
	if (uMsg == BFFM_INITIALIZED)
		SendMessage(hwnd, BFFM_SETSELECTION, (WPARAM)TRUE, lpData);
	return 0;
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
