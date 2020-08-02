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

boolean select_game_folder(void) {
	char title[256];
	LoadString(GetModuleHandle(NULL), IDS_CHOOSE_GAME_FOLDER, title, sizeof(title));

	BROWSEINFO bi = {
		.lpszTitle = title,
		.ulFlags = BIF_RETURNONLYFSDIRS,
	};
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if (!pidl)
		return FALSE;
	char path[MAX_PATH];
	SHGetPathFromIDList(pidl, path);
	CoTaskMemFree(pidl);
	if (!SetCurrentDirectory(path))
		return FALSE;
	return TRUE;
}