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

#ifndef __HACKS_H_
#define __HACKS_H_

#include <stdbool.h>

enum gameId {
	GAME_UNKNOWN = 0,
	GAME_TT2,
	GAME_RANCE3,
	GAME_RANCE3_ENG,
	GAME_RANCE4_ENG,
	GAME_RANCE4_V2,
};

extern enum gameId game_id;

void enable_hack_by_gameid(const char *gameid);
void enable_hack_by_title(const char *game_title_utf8);

extern bool daiakuji_cx_hack;

#endif // __HACKS_H_
