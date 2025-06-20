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

#include <SDL_surface.h>
#include "ags.h"

// Effect types of the CE/CD command.
enum nact_effect {
	NACT_EFFECT_PAN_IN_DOWN       = 1,
	NACT_EFFECT_PAN_IN_UP         = 2,
	NACT_EFFECT_SKIP_LINE_UP_DOWN = 3,
	NACT_EFFECT_SKIP_LINE_LR_RL   = 4,
	NACT_SP_EFFECT_RASTER_BLEND   = 5,  // CD 5
	NACT_EFFECT_WIPE_IN           = 7,
	NACT_EFFECT_WIPE_OUT          = 8,
	NACT_EFFECT_ZOOM_IN           = 10,
	NACT_EFFECT_BLIND_DOWN        = 11,
	NACT_EFFECT_WIPE_LR           = 12,
	NACT_EFFECT_WIPE_RL           = 13,
	NACT_EFFECT_WIPE_DOWN         = 14,
	NACT_EFFECT_WIPE_UP           = 15,
	NACT_EFFECT_WIPE_OUT_V        = 18,
	NACT_EFFECT_WIPE_IN_V         = 19,
	NACT_EFFECT_WIPE_OUT_H        = 20,
	NACT_EFFECT_WIPE_IN_H         = 21,
	NACT_EFFECT_MOSAIC            = 24,
	NACT_EFFECT_CIRCLE_WIPE_OUT   = 25,
	NACT_EFFECT_CIRCLE_WIPE_IN    = 26,
	NACT_EFFECT_FADEIN            = 27,
	NACT_EFFECT_WHITEIN           = 28,
	NACT_EFFECT_FADEOUT           = 29,
	NACT_EFFECT_WHITEOUT          = 30,
	NACT_EFFECT_CROSSFADE         = 31,
	NACT_EFFECT_CROSSFADE_MOSAIC  = 32,
	NACT_EFFECT_BLIND_UP          = 33,
	NACT_EFFECT_BLIND_UP_DOWN     = 34,
	NACT_EFFECT_CROSSFADE_DOWN    = 35,
	NACT_EFFECT_CROSSFADE_UP      = 36,
	NACT_EFFECT_CROSSFADE_LR      = 37,
	NACT_EFFECT_CROSSFADE_RL      = 38,
	NACT_EFFECT_BLEND_UP_DOWN     = 39,
	NACT_EFFECT_BLEND_LR_RL       = 40,
	NACT_EFFECT_CROSSFADE_LR_RL   = 41,
	NACT_EFFECT_CROSSFADE_UP_DOWN = 42,
	NACT_EFFECT_MAGNIFY           = 43,
	NACT_EFFECT_PENTAGRAM_IN_OUT  = 44,
	NACT_EFFECT_PENTAGRAM_OUT_IN  = 45,
	NACT_EFFECT_HEXAGRAM_IN_OUT   = 46,
	NACT_EFFECT_HEXAGRAM_OUT_IN   = 47,
	NACT_EFFECT_BLIND_LR          = 48,
	NACT_EFFECT_BLIND_RL          = 49,
	NACT_EFFECT_WINDMILL          = 50,
	NACT_EFFECT_WINDMILL_180      = 51,
	NACT_EFFECT_WINDMILL_360      = 52,
	NACT_EFFECT_LINEAR_BLUR       = 53,
	NACT_EFFECT_PALETTE_SHIFT     = 1001,  // CD command only
};

// Effect types of SCAT.DrawEffect and Gpx.EffectCopy.
enum sact_effect {
	SACT_EFFECT_CROSSFADE              = 1,
	SACT_EFFECT_FADEOUT                = 2,
	SACT_EFFECT_FADEIN                 = 3,
	SACT_EFFECT_WHITEOUT               = 4,
	SACT_EFFECT_WHITEIN                = 5,
	SACT_EFFECT_CROSSFADE_MOSAIC       = 6,
	SACT_EFFECT_BLIND_DOWN             = 7,
	SACT_EFFECT_BLIND_LR               = 8,
	SACT_EFFECT_BLIND_DOWN_LR          = 9,
	SACT_EFFECT_ZOOM_BLEND_BLUR        = 10,
	SACT_EFFECT_LINEAR_BLUR            = 11,
	SACT_EFFECT_CROSSFADE_DOWN         = 12,
	SACT_EFFECT_CROSSFADE_UP           = 13,
	SACT_EFFECT_PENTAGRAM_IN_OUT       = 14,
	SACT_EFFECT_PENTAGRAM_OUT_IN       = 15,
	SACT_EFFECT_HEXAGRAM_IN_OUT        = 16,
	SACT_EFFECT_HEXAGRAM_OUT_IN        = 17,
	SACT_EFFECT_AMAP_CROSSFADE         = 18,
	SACT_EFFECT_LINEAR_BLUR_VERT       = 19,
	SACT_EFFECT_ROTATE_OUT             = 20,
	SACT_EFFECT_ROTATE_IN              = 21,
	SACT_EFFECT_ROTATE_OUT_CW          = 22,
	SACT_EFFECT_ROTATE_IN_CW           = 23,
	SACT_EFFECT_BLOCK_DISSOLVE         = 24,
	SACT_EFFECT_POLYGON_ROTATE_Y       = 25,
	SACT_EFFECT_POLYGON_ROTATE_Y_CW    = 26,
	SACT_EFFECT_OSCILLATE              = 27,
	SACT_EFFECT_POLYGON_ROTATE_X       = 28,
	SACT_EFFECT_POLYGON_ROTATE_X_CW    = 29,
	SACT_EFFECT_ROTATE_ZOOM_BLEND_BLUR = 30,
	SACT_EFFECT_ZIGZAG_CROSSFADE       = 31,
	SACT_EFFECT_TV_SWITCH_OFF          = 32,
	SACT_EFFECT_TV_SWITCH_ON           = 33,
	SACT_EFFECT_POLYGON_EXPLOSION      = 34,
	SACT_EFFECT_NOISE_CROSSFADE        = 35,
	SACT_EFFECT_TURN_PAGE              = 36,
	SACT_EFFECT_SEPIA_NOISE_CROSSFADE  = 37,
	SACT_EFFECT_CRUMPLED_PAPER_PULL    = 38,
	SACT_EFFECT_HORIZONTAL_ZIGZAG      = 39,
};

// Internal effect numbers.
enum effect_type {
	EFFECT_INVALID,
	EFFECT_CROSSFADE,
	EFFECT_FADEOUT,
	EFFECT_FADEOUT_FROM_NEW,
	EFFECT_FADEIN,
	EFFECT_WHITEOUT,
	EFFECT_WHITEOUT_FROM_NEW,
	EFFECT_WHITEIN,
	EFFECT_DITHERING_FADEOUT,
	EFFECT_DITHERING_FADEIN,
	EFFECT_DITHERING_WHITEOUT,
	EFFECT_DITHERING_WHITEIN,
	EFFECT_PAN_IN_DOWN,
	EFFECT_PAN_IN_UP,
	EFFECT_SKIP_LINE_UP_DOWN,
	EFFECT_SKIP_LINE_LR_RL,
	EFFECT_WIPE_IN,
	EFFECT_WIPE_OUT,
	EFFECT_WIPE_LR,
	EFFECT_WIPE_RL,
	EFFECT_WIPE_DOWN,
	EFFECT_WIPE_UP,
	EFFECT_WIPE_OUT_V,
	EFFECT_WIPE_IN_V,
	EFFECT_WIPE_OUT_H,
	EFFECT_WIPE_IN_H,
	EFFECT_CIRCLE_WIPE_OUT,
	EFFECT_CIRCLE_WIPE_IN,
	EFFECT_BLIND_DOWN,
	EFFECT_BLIND_UP,
	EFFECT_BLIND_LR,
	EFFECT_BLIND_RL,
	EFFECT_BLIND_UP_DOWN,
	EFFECT_BLIND_DOWN_LR,
	EFFECT_BLEND_UP_DOWN,
	EFFECT_BLEND_LR_RL,
	EFFECT_CROSSFADE_DOWN,
	EFFECT_CROSSFADE_UP,
	EFFECT_CROSSFADE_LR,
	EFFECT_CROSSFADE_RL,
	EFFECT_CROSSFADE_LR_RL,
	EFFECT_CROSSFADE_UP_DOWN,
	EFFECT_CROSSFADE_MOSAIC,
	EFFECT_MOSAIC,
	EFFECT_ZOOM_BLEND_BLUR,
	EFFECT_LINEAR_BLUR,
	EFFECT_LINEAR_BLUR_VERT,
	EFFECT_PENTAGRAM_IN_OUT,
	EFFECT_PENTAGRAM_OUT_IN,
	EFFECT_HEXAGRAM_IN_OUT,
	EFFECT_HEXAGRAM_OUT_IN,
	EFFECT_WINDMILL,
	EFFECT_WINDMILL_180,
	EFFECT_WINDMILL_360,
	EFFECT_ZOOM_IN,
	EFFECT_ROTATE_OUT,
	EFFECT_ROTATE_IN,
	EFFECT_ROTATE_OUT_CW,
	EFFECT_ROTATE_IN_CW,
	EFFECT_POLYGON_ROTATE_Y,
	EFFECT_POLYGON_ROTATE_Y_CW,
	EFFECT_POLYGON_ROTATE_X,
	EFFECT_POLYGON_ROTATE_X_CW,
	EFFECT_ZIGZAG_CROSSFADE,
	EFFECT_MAGNIFY,
	EFFECT_RASTER_BLEND,
	EFFECT_SACTAMASK,
};

enum effect_type from_nact_effect(enum nact_effect effect);
enum effect_type from_nact_sprite_effect(enum nact_effect effect);
enum effect_type from_sact_effect(enum sact_effect effect);

struct effect;
struct effect *effect_init(SDL_Rect *rect, surface_t *old, int ox, int oy, surface_t *new, int nx, int ny, enum effect_type effect);
struct effect *sprite_effect_init(SDL_Rect *rect, int dx, int dy, int sx, int sy, int col, enum effect_type type);
struct effect *effect_magnify_init(surface_t *surface, SDL_Rect *view_rect, SDL_Rect *target_rect);
struct effect *effect_sactamask_init(SDL_Surface *mask);
void effect_step(struct effect *eff, float progress);
void effect_finish(struct effect *eff);

#endif /* __EFFECT_H__ */
