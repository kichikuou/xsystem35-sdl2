/*
 * Copyright (C) 2023 kichikuou <KichikuouChrome@gmail.com>
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
#include <string.h>

#include "hacks.h"
#include "system.h"
#include "nact.h"
#include "input.h"

// Game titles.
#define GT_TOSHIN2 "闘神都市Ⅱ　ｆｏｒ　Ｗｉｎ９５　"
#define GT_RANCE4 "Ｒａｎｃｅ４　－教団の遺産－　Ｆｏｒ　Ｗｉｎ９５　"
#define GT_RANCE4_V2 "RanceⅣ　－教団の遺産－　for Windows　"
#define GT_RANCE3_ENG "Rance3"
#define GT_RANCE4_ENG "Rance4 -Legacy of the Sect- For Win95 "

/* defined by cmdy.c */
extern boolean Y3waitFlags;

void enable_hack_by_gameid(const char *gameid) {
	if (!strcmp(gameid, "toushin2"))
		nact->game = GAME_TT2;
	else if (!strcmp(gameid, "rance3_eng"))
		nact->game = GAME_RANCE3_ENG;
	else if (!strcmp(gameid, "rance4_eng"))
		nact->game = GAME_RANCE4_ENG;
	else if (!strcmp(gameid, "rance4_v2"))
		nact->game = GAME_RANCE4_V2;
	else
		sys_error("Unknown game id \"%s\"", gameid);
}

void enable_hack_by_title(const char *title_utf8) {
	if (!strcmp(title_utf8, GT_RANCE4))
		Y3waitFlags = KEYWAIT_NONCANCELABLE;

	if (nact->game != GAME_UNKNOWN)
		return;

	if (!strcmp(title_utf8, GT_TOSHIN2))
		nact->game = GAME_TT2;
	else if (!strcmp(title_utf8, GT_RANCE4_V2))
		nact->game = GAME_RANCE4_V2;
	else if (!strcmp(title_utf8, GT_RANCE3_ENG))
		nact->game = GAME_RANCE3_ENG;
	else if (!strcmp(title_utf8, GT_RANCE4_ENG))
		nact->game = GAME_RANCE4_ENG;
}
