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

#ifndef __EFFECT_H__
#define __EFFECT_H__

// Effect numbers for the CE command.
enum effect {
	EFFECT_CROSSFADE = 31,
	EFFECT_PENTAGRAM_IN_OUT = 44,
	EFFECT_PENTAGRAM_OUT_IN = 45,
	EFFECT_HEXAGRAM_IN_OUT = 46,
	EFFECT_HEXAGRAM_OUT_IN = 47,
	EFFECT_WINDMILL = 50,
	EFFECT_WINDMILL_180 = 51,
	EFFECT_WINDMILL_360 = 52,
};

enum sact_effect {
	SACT_EFFECT_PENTAGRAM_IN_OUT = 14,
	SACT_EFFECT_PENTAGRAM_OUT_IN = 15,
	SACT_EFFECT_HEXAGRAM_IN_OUT = 16,
	SACT_EFFECT_HEXAGRAM_OUT_IN = 17,
};

#endif /* __EFFECT_H__ */
