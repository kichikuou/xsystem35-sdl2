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
#include <ctype.h>
#include <string.h>
#include <strings.h>

#include "hacks.h"
#include "system.h"
#include "nact.h"
#include "input.h"

// Game titles.
#define GT_TOSHIN2 "闘神都市Ⅱ　ｆｏｒ　Ｗｉｎ９５　"
#define GT_TOSHIN2_ENG "Toushin Toshi II for Win95"
#define GT_RANCE4 "Ｒａｎｃｅ４　－教団の遺産－　Ｆｏｒ　Ｗｉｎ９５　"
#define GT_RANCE4_V2 "RanceⅣ　－教団の遺産－　for Windows　"
#define GT_RANCE4_CN "［Ｒａｎｃｅ　ＩＶ　－教团的遗产－　Ｆｏｒ　Ｗｉｎ９５］"
#define GT_RANCE2 "ＲＡＮＣＥ２"
#define GT_RANCE3 "Ｒａｎｃｅ３"
#define GT_RANCE3_ENG "Rance3"
#define GT_RANCE4_ENG "Rance4 -Legacy of the Sect- For Win95 "
#define GT_AGAKE "俺の下であがけ"
#define GT_PERSIOM "ＰＥＲＳＩＯＭ"

/* defined by cmdy.c */
extern unsigned Y3waitFlags;

enum gameId game_id;
bool daiakuji_cx_hack;

void hacks_reset(void) {
	game_id = GAME_UNKNOWN;
	daiakuji_cx_hack = false;
	Y3waitFlags = KEYWAIT_CANCELABLE;
}

static bool is_utf8_space_at(const char *s, size_t len, size_t pos, size_t *step) {
	if (pos < len && isspace((unsigned char)s[pos])) {
		*step = 1;
		return true;
	}
	if (pos + 3 <= len &&
	    (unsigned char)s[pos] == 0xe3 &&
	    (unsigned char)s[pos + 1] == 0x80 &&
	    (unsigned char)s[pos + 2] == 0x80) {
		*step = 3;
		return true;
	}
	return false;
}

static bool title_equals_ascii_icase_trimmed(const char *title_utf8, const char *target) {
	size_t len = strlen(title_utf8);
	size_t start = 0;
	size_t step = 0;
	while (start < len && is_utf8_space_at(title_utf8, len, start, &step))
		start += step;
	size_t end = len;
	while (end > start) {
		if (isspace((unsigned char)title_utf8[end - 1])) {
			end--;
		} else if (end - start >= 3 &&
		           (unsigned char)title_utf8[end - 3] == 0xe3 &&
		           (unsigned char)title_utf8[end - 2] == 0x80 &&
		           (unsigned char)title_utf8[end - 1] == 0x80) {
			end -= 3;
		} else {
			break;
		}
	}
	size_t tlen = strlen(target);
	if (end - start != tlen)
		return false;
	return strncasecmp(title_utf8 + start, target, tlen) == 0;
}

void enable_hack_by_gameid(const char *gameid) {
	if (!strcmp(gameid, "toushin2"))
		game_id = GAME_TT2;
	else if (!strcmp(gameid, "rance2"))
		game_id = GAME_RANCE2;
	else if (!strcmp(gameid, "rance3"))
		game_id = GAME_RANCE3;
	else if (!strcmp(gameid, "rance3_eng"))
		game_id = GAME_RANCE3_ENG;
	else if (!strcmp(gameid, "rance4_eng"))
		game_id = GAME_RANCE4_ENG;
	else if (!strcmp(gameid, "rance4_v2"))
		game_id = GAME_RANCE4_V2;
	else if (!strcmp(gameid, "agake"))
		game_id = GAME_AGAKE;
	else if (!strcmp(gameid, "persiom"))
		game_id = GAME_PERSIOM;
	else
		sys_error("Unknown game id \"%s\"", gameid);
}

void enable_hack_by_title(const char *title_utf8) {
	if (!title_utf8)
		return;

	bool is_rance4_cn = !strcmp(title_utf8, GT_RANCE4_CN) ||
		strstr(title_utf8, "教团的遗产") != NULL ||
		strstr(title_utf8, "教團的遺產") != NULL;

	if (!strcmp(title_utf8, GT_RANCE4) || is_rance4_cn)
		Y3waitFlags = KEYWAIT_NONCANCELABLE;

	if (game_id != GAME_UNKNOWN)
		return;

	if (!strcmp(title_utf8, GT_TOSHIN2) || title_equals_ascii_icase_trimmed(title_utf8, GT_TOSHIN2_ENG))
		game_id = GAME_TT2;
	else if (!strcmp(title_utf8, GT_RANCE4_V2) || is_rance4_cn)
		game_id = GAME_RANCE4_V2;
	else if (!strcmp(title_utf8, GT_RANCE2))
		game_id = GAME_RANCE2;
	else if (!strcmp(title_utf8, GT_RANCE3))
		game_id = GAME_RANCE3;
	else if (!strcmp(title_utf8, GT_RANCE3_ENG))
		game_id = GAME_RANCE3_ENG;
	else if (!strcmp(title_utf8, GT_RANCE4_ENG))
		game_id = GAME_RANCE4_ENG;
	else if (!strcmp(title_utf8, GT_AGAKE))
		game_id = GAME_AGAKE;
	else if (!strcmp(title_utf8, GT_PERSIOM))
		game_id = GAME_PERSIOM;
}
