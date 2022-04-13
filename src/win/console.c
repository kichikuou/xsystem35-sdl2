/*
 * Copyright (C) 2021 <KichikuouChrome@gmail.com>
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
#include <stdio.h>
#include <signal.h>
#include "win/console.h"

static void (*ctrl_c_handler)(int);

static BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {
	if (fdwCtrlType == CTRL_C_EVENT) {
		ctrl_c_handler(SIGINT);
		return TRUE;
	}
	return FALSE;
}

void win_alloc_console(void) {
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONIN$", "r", stdin);
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
}

void win_set_ctrl_c_handler(void (*handler)(int)) {
	if (!ctrl_c_handler && handler) {
		SetConsoleCtrlHandler(CtrlHandler, TRUE);
	} else if (ctrl_c_handler && !handler) {
		SetConsoleCtrlHandler(CtrlHandler, FALSE);
	}
	ctrl_c_handler = handler;
}
