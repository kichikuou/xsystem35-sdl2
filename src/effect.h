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

// Effect types of the CE command.
enum nact_effect {
	NACT_EFFECT_BLIND_DOWN       = 11,
	NACT_EFFECT_FADEIN           = 27,
	NACT_EFFECT_WHITEIN          = 28,
	NACT_EFFECT_FADEOUT          = 29,
	NACT_EFFECT_WHITEOUT         = 30,
	NACT_EFFECT_CROSSFADE        = 31,
	NACT_EFFECT_BLIND_UP         = 33,
	NACT_EFFECT_BLIND_UP_DOWN    = 34,
	NACT_EFFECT_CROSSFADE_DOWN   = 35,
	NACT_EFFECT_CROSSFADE_UP     = 36,
	NACT_EFFECT_CROSSFADE_LR     = 37,
	NACT_EFFECT_CROSSFADE_RL     = 38,
	NACT_EFFECT_PENTAGRAM_IN_OUT = 44,
	NACT_EFFECT_PENTAGRAM_OUT_IN = 45,
	NACT_EFFECT_HEXAGRAM_IN_OUT  = 46,
	NACT_EFFECT_HEXAGRAM_OUT_IN  = 47,
	NACT_EFFECT_BLIND_LR         = 48,
	NACT_EFFECT_BLIND_RL         = 49,
	NACT_EFFECT_WINDMILL         = 50,
	NACT_EFFECT_WINDMILL_180     = 51,
	NACT_EFFECT_WINDMILL_360     = 52,
	NACT_EFFECT_LINEAR_BLUR      = 53,
};

// Effect types of SCAT.DrawEffect and Gpx.EffectCopy.
enum sact_effect {
	SACT_EFFECT_CROSSFADE        = 1,
	SACT_EFFECT_FADEOUT          = 2,
	SACT_EFFECT_FADEIN           = 3,
	SACT_EFFECT_WHITEOUT         = 4,
	SACT_EFFECT_WHITEIN          = 5,
	SACT_EFFECT_BLIND_DOWN       = 7,
	SACT_EFFECT_BLIND_LR         = 8,
	SACT_EFFECT_BLIND_DOWN_LR    = 9,
	SACT_EFFECT_LINEAR_BLUR      = 11,
	SACT_EFFECT_CROSSFADE_DOWN   = 12,
	SACT_EFFECT_CROSSFADE_UP     = 13,
	SACT_EFFECT_PENTAGRAM_IN_OUT = 14,
	SACT_EFFECT_PENTAGRAM_OUT_IN = 15,
	SACT_EFFECT_HEXAGRAM_IN_OUT  = 16,
	SACT_EFFECT_HEXAGRAM_OUT_IN  = 17,
	SACT_EFFECT_LINEAR_BLUR_VERT = 19,
	SACT_EFFECT_ROTATE_OUT       = 20,
	SACT_EFFECT_ROTATE_IN        = 21,
	SACT_EFFECT_ROTATE_OUT_CW    = 22,
	SACT_EFFECT_ROTATE_IN_CW     = 23,
};

// Internal effect numbers.
enum sdl_effect_type {
	EFFECT_INVALID,
	EFFECT_CROSSFADE,
	EFFECT_FADEOUT,
	EFFECT_FADEOUT_FROM_NEW,
	EFFECT_FADEIN,
	EFFECT_WHITEOUT,
	EFFECT_WHITEOUT_FROM_NEW,
	EFFECT_WHITEIN,
	EFFECT_BLIND_DOWN,
	EFFECT_BLIND_UP,
	EFFECT_BLIND_LR,
	EFFECT_BLIND_RL,
	EFFECT_BLIND_UP_DOWN,
	EFFECT_BLIND_DOWN_LR,
	EFFECT_CROSSFADE_DOWN,
	EFFECT_CROSSFADE_UP,
	EFFECT_CROSSFADE_LR,
	EFFECT_CROSSFADE_RL,
	EFFECT_LINEAR_BLUR,
	EFFECT_LINEAR_BLUR_VERT,
	EFFECT_PENTAGRAM_IN_OUT,
	EFFECT_PENTAGRAM_OUT_IN,
	EFFECT_HEXAGRAM_IN_OUT,
	EFFECT_HEXAGRAM_OUT_IN,
	EFFECT_WINDMILL,
	EFFECT_WINDMILL_180,
	EFFECT_WINDMILL_360,
	EFFECT_ROTATE_OUT,
	EFFECT_ROTATE_IN,
	EFFECT_ROTATE_OUT_CW,
	EFFECT_ROTATE_IN_CW,
};

enum sdl_effect_type from_nact_effect(enum nact_effect effect);
enum sdl_effect_type from_sact_effect(enum sact_effect effect);

#endif /* __EFFECT_H__ */
