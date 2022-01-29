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

#include "config.h"

#include <assert.h>
#include <math.h>
#include <SDL.h>

#include "system.h"
#include "sdl_core.h"
#include "sdl_private.h"

#define HAS_SDL_RenderGeometry SDL_VERSION_ATLEAST(2, 0, 18)

enum sdl_effect_type from_nact_effect(enum nact_effect effect) {
	switch (effect) {
	case NACT_EFFECT_ZOOM_IN:          return EFFECT_ZOOM_IN;
	case NACT_EFFECT_BLIND_DOWN:       return EFFECT_BLIND_DOWN;
	case NACT_EFFECT_MOSAIC:           return EFFECT_MOSAIC;
	case NACT_EFFECT_FADEIN:           return EFFECT_FADEIN;
	case NACT_EFFECT_WHITEIN:          return EFFECT_WHITEIN;
	case NACT_EFFECT_FADEOUT:          return EFFECT_FADEOUT_FROM_NEW;
	case NACT_EFFECT_WHITEOUT:         return EFFECT_WHITEOUT_FROM_NEW;
	case NACT_EFFECT_CROSSFADE:        return EFFECT_CROSSFADE;
	case NACT_EFFECT_CROSSFADE_MOSAIC: return EFFECT_CROSSFADE_MOSAIC;
	case NACT_EFFECT_BLIND_UP:         return EFFECT_BLIND_UP;
	case NACT_EFFECT_BLIND_UP_DOWN:    return EFFECT_BLIND_UP_DOWN;
	case NACT_EFFECT_CROSSFADE_DOWN:   return EFFECT_CROSSFADE_DOWN;
	case NACT_EFFECT_CROSSFADE_UP:     return EFFECT_CROSSFADE_UP;
	case NACT_EFFECT_CROSSFADE_LR:     return EFFECT_CROSSFADE_LR;
	case NACT_EFFECT_CROSSFADE_RL:     return EFFECT_CROSSFADE_RL;
	case NACT_EFFECT_PENTAGRAM_IN_OUT: return EFFECT_PENTAGRAM_IN_OUT;
	case NACT_EFFECT_PENTAGRAM_OUT_IN: return EFFECT_PENTAGRAM_OUT_IN;
	case NACT_EFFECT_HEXAGRAM_IN_OUT:  return EFFECT_HEXAGRAM_IN_OUT;
	case NACT_EFFECT_HEXAGRAM_OUT_IN:  return EFFECT_HEXAGRAM_OUT_IN;
	case NACT_EFFECT_BLIND_LR:         return EFFECT_BLIND_LR;
	case NACT_EFFECT_BLIND_RL:         return EFFECT_BLIND_RL;
	case NACT_EFFECT_WINDMILL:         return EFFECT_WINDMILL;
	case NACT_EFFECT_WINDMILL_180:     return EFFECT_WINDMILL_180;
	case NACT_EFFECT_WINDMILL_360:     return EFFECT_WINDMILL_360;
	case NACT_EFFECT_LINEAR_BLUR:      return EFFECT_LINEAR_BLUR;
	default:                           return EFFECT_INVALID;
	}
}

enum sdl_effect_type from_sact_effect(enum sact_effect effect) {
	switch (effect) {
	case SACT_EFFECT_CROSSFADE:        return EFFECT_CROSSFADE;
	case SACT_EFFECT_FADEOUT:          return EFFECT_FADEOUT;
	case SACT_EFFECT_FADEIN:           return EFFECT_FADEIN;
	case SACT_EFFECT_WHITEOUT:         return EFFECT_WHITEOUT;
	case SACT_EFFECT_WHITEIN:          return EFFECT_WHITEIN;
	case SACT_EFFECT_CROSSFADE_MOSAIC: return EFFECT_CROSSFADE_MOSAIC;
	case SACT_EFFECT_BLIND_DOWN:       return EFFECT_BLIND_DOWN;
	case SACT_EFFECT_BLIND_LR:         return EFFECT_BLIND_LR;
	case SACT_EFFECT_BLIND_DOWN_LR:    return EFFECT_BLIND_DOWN_LR;
	case SACT_EFFECT_ZOOM_BLEND_BLUR:  return EFFECT_ZOOM_BLEND_BLUR;
	case SACT_EFFECT_LINEAR_BLUR:      return EFFECT_LINEAR_BLUR;
	case SACT_EFFECT_CROSSFADE_DOWN:   return EFFECT_CROSSFADE_DOWN;
	case SACT_EFFECT_CROSSFADE_UP:     return EFFECT_CROSSFADE_UP;
	case SACT_EFFECT_PENTAGRAM_IN_OUT: return EFFECT_PENTAGRAM_IN_OUT;
	case SACT_EFFECT_PENTAGRAM_OUT_IN: return EFFECT_PENTAGRAM_OUT_IN;
	case SACT_EFFECT_HEXAGRAM_IN_OUT:  return EFFECT_HEXAGRAM_IN_OUT;
	case SACT_EFFECT_HEXAGRAM_OUT_IN:  return EFFECT_HEXAGRAM_OUT_IN;
	case SACT_EFFECT_LINEAR_BLUR_VERT: return EFFECT_LINEAR_BLUR_VERT;
	case SACT_EFFECT_ROTATE_OUT:       return EFFECT_ROTATE_OUT;
	case SACT_EFFECT_ROTATE_IN:        return EFFECT_ROTATE_IN;
	case SACT_EFFECT_ROTATE_OUT_CW:    return EFFECT_ROTATE_OUT_CW;
	case SACT_EFFECT_ROTATE_IN_CW:     return EFFECT_ROTATE_IN_CW;
	case SACT_EFFECT_ZIGZAG_CROSSFADE: return EFFECT_ZIGZAG_CROSSFADE;
	default:                           return EFFECT_INVALID;
	}
}

struct sdl_effect {
	void (*step)(struct sdl_effect *this, double progress);
	void (*finish)(struct sdl_effect *this);
	enum sdl_effect_type type;
	SDL_Rect dst_rect;
	bool is_fullscreen;
	SDL_Texture *tx_old, *tx_new;
};

static void effect_init(struct sdl_effect *eff, SDL_Rect *rect, SDL_Surface *old, SDL_Surface *new, enum sdl_effect_type type) {
	eff->type = type;
	eff->dst_rect = *rect;
	eff->is_fullscreen = rect->x == 0 && rect->y == 0
		&& rect->w == sdl_display->w && rect->h == sdl_display->h;
	eff->tx_old = SDL_CreateTextureFromSurface(sdl_renderer, old);
	eff->tx_new = SDL_CreateTextureFromSurface(sdl_renderer, new);
	SDL_FreeSurface(old);
	SDL_FreeSurface(new);
}

static void effect_finish(struct sdl_effect *eff, bool present) {
	if (present) {
		SDL_RenderClear(sdl_renderer);
		if (!eff->is_fullscreen)
			SDL_RenderCopy(sdl_renderer, sdl_texture, NULL, NULL);
		SDL_RenderCopy(sdl_renderer, eff->tx_new, NULL, &eff->dst_rect);
		SDL_RenderPresent(sdl_renderer);
	}
	SDL_DestroyTexture(eff->tx_old);
	SDL_DestroyTexture(eff->tx_new);
}

static inline void flip_rect_h(SDL_Rect *r, int w) {
	r->x = w - r->x - r->w;
}

static inline void flip_rect_v(SDL_Rect *r, int h) {
	r->y = h - r->y - r->h;
}

static inline void move_rect(SDL_Rect *r, int off_x, int off_y) {
	r->x += off_x;
	r->y += off_y;
}

// EFFECT_CROSSFADE

static void crossfade_step(struct sdl_effect *eff, double progress);
static void crossfade_free(struct sdl_effect *eff);

static struct sdl_effect *crossfade_new(SDL_Rect *rect, SDL_Surface *old, SDL_Surface *new) {
	struct sdl_effect *eff = calloc(1, sizeof(struct sdl_effect));
	if (!eff)
		NOMEMERR();
	effect_init(eff, rect, old, new, EFFECT_CROSSFADE);
	eff->step = crossfade_step;
	eff->finish = crossfade_free;
	return eff;
}

static void crossfade_step(struct sdl_effect *eff, double progress) {
	SDL_RenderCopy(sdl_renderer, eff->tx_old, NULL, &eff->dst_rect);
	SDL_SetTextureBlendMode(eff->tx_new, SDL_BLENDMODE_BLEND);
	SDL_SetTextureAlphaMod(eff->tx_new, progress * 255);
	SDL_RenderCopy(sdl_renderer, eff->tx_new, NULL, &eff->dst_rect);
	SDL_SetTextureBlendMode(eff->tx_new, SDL_BLENDMODE_NONE);
	SDL_RenderPresent(sdl_renderer);
}

static void crossfade_free(struct sdl_effect *eff) {
	effect_finish(eff, true);
	free(eff);
}

static struct sdl_effect *fallback_effect_new(SDL_Rect *rect, SDL_Surface *old, SDL_Surface *new, enum sdl_effect_type type) {
	WARNING("Effect %d is not supported in this system. Falling back to crossfade.\n", type);
	return crossfade_new(rect, old, new);
}

// EFFECT_CROSSFADE_{UP,DOWN,LR,RL}

static void crossfade_vstep(struct sdl_effect *eff, double progress);
static void crossfade_hstep(struct sdl_effect *eff, double progress);
static void crossfade_animation_free(struct sdl_effect *eff);

static struct sdl_effect *crossfade_animation_new(SDL_Rect *rect, SDL_Surface *old, SDL_Surface *new, enum sdl_effect_type type) {
	struct sdl_effect *eff = calloc(1, sizeof(struct sdl_effect));
	if (!eff)
		NOMEMERR();
	effect_init(eff, rect, old, new, type);
	switch (type) {
	case EFFECT_CROSSFADE_DOWN:
	case EFFECT_CROSSFADE_UP:
		eff->step = crossfade_vstep;
		break;
	case EFFECT_CROSSFADE_LR:
	case EFFECT_CROSSFADE_RL:
		eff->step = crossfade_hstep;
		break;
	default: assert(!"Cannot happen");
	}
	eff->finish = crossfade_animation_free;
	return eff;
}

static void crossfade_vstep(struct sdl_effect *eff, double progress) {
	const int BAND_WIDTH = 254;

	SDL_RenderCopy(sdl_renderer, eff->tx_old, NULL, &eff->dst_rect);

	int maxstep = eff->dst_rect.h + BAND_WIDTH;
	int band_top = maxstep * progress - BAND_WIDTH;
	if (band_top > 0) {
		SDL_Rect sr = {0, 0, eff->dst_rect.w, band_top};
		if (eff->type == EFFECT_CROSSFADE_UP)
			flip_rect_v(&sr, eff->dst_rect.h);
		SDL_Rect dr = sr;
		move_rect(&dr, eff->dst_rect.x, eff->dst_rect.y);
		SDL_RenderCopy(sdl_renderer, eff->tx_new, &sr, &dr);
	}

	SDL_SetTextureBlendMode(eff->tx_new, SDL_BLENDMODE_BLEND);
	for (int i = 0; i < BAND_WIDTH; i++) {
		int y = band_top + i;
		if (y < 0 || y >= eff->dst_rect.h)
			continue;
		SDL_Rect sr = {0, y, eff->dst_rect.w, 1};
		if (eff->type == EFFECT_CROSSFADE_UP)
			flip_rect_v(&sr, eff->dst_rect.h);
		SDL_Rect dr = sr;
		move_rect(&dr, eff->dst_rect.x, eff->dst_rect.y);
		SDL_SetTextureAlphaMod(eff->tx_new, BAND_WIDTH - i);
		SDL_RenderCopy(sdl_renderer, eff->tx_new, &sr, &dr);
	}
	SDL_SetTextureBlendMode(eff->tx_new, SDL_BLENDMODE_NONE);

	SDL_RenderPresent(sdl_renderer);
}

static void crossfade_hstep(struct sdl_effect *eff, double progress) {
	const int BAND_WIDTH = 254;

	SDL_RenderCopy(sdl_renderer, eff->tx_old, NULL, &eff->dst_rect);

	int maxstep = eff->dst_rect.w + BAND_WIDTH;
	int band_lhs = maxstep * progress - BAND_WIDTH;
	if (band_lhs > 0) {
		SDL_Rect sr = {0, 0, band_lhs, eff->dst_rect.h};
		if (eff->type == EFFECT_CROSSFADE_RL)
			flip_rect_h(&sr, eff->dst_rect.w);
		SDL_Rect dr = sr;
		move_rect(&dr, eff->dst_rect.x, eff->dst_rect.y);
		SDL_RenderCopy(sdl_renderer, eff->tx_new, &sr, &dr);
	}

	SDL_SetTextureBlendMode(eff->tx_new, SDL_BLENDMODE_BLEND);
	for (int i = 0; i < BAND_WIDTH; i++) {
		int x = band_lhs + i;
		if (x < 0 || x >= eff->dst_rect.w)
			continue;
		SDL_Rect sr = {x, 0, 1, eff->dst_rect.h};
		if (eff->type == EFFECT_CROSSFADE_RL)
			flip_rect_h(&sr, eff->dst_rect.w);
		SDL_Rect dr = sr;
		move_rect(&dr, eff->dst_rect.x, eff->dst_rect.y);
		SDL_SetTextureAlphaMod(eff->tx_new, BAND_WIDTH - i);
		SDL_RenderCopy(sdl_renderer, eff->tx_new, &sr, &dr);
	}
	SDL_SetTextureBlendMode(eff->tx_new, SDL_BLENDMODE_NONE);

	SDL_RenderPresent(sdl_renderer);
}

static void crossfade_animation_free(struct sdl_effect *eff) {
	effect_finish(eff, false);
	free(eff);
}

// EFFECT_MOSAIC, EFFECT_CROSSFADE_MOSAIC

struct mosaic_effect {
	struct sdl_effect eff;
	SDL_Texture *tmp_old, *tmp_new, *tx_mosaic;
};

static void mosaic_step(struct sdl_effect *eff, double progress);
static void mosaic_free(struct sdl_effect *eff);

static struct sdl_effect *mosaic_new(SDL_Rect *rect, SDL_Surface *old, SDL_Surface *new, enum sdl_effect_type type) {
	if (!SDL_RenderTargetSupported(sdl_renderer))
		return fallback_effect_new(rect, old, new, type);

	struct mosaic_effect *eff = calloc(1, sizeof(struct mosaic_effect));
	if (!eff)
		NOMEMERR();
	effect_init(&eff->eff, rect, old, new, type);
	eff->tmp_old = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, rect->w, rect->h);
	eff->tmp_new = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, rect->w, rect->h);
	eff->tx_mosaic = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, rect->w, rect->h);
	eff->eff.step = mosaic_step;
	eff->eff.finish = mosaic_free;
	return &eff->eff;
}

static void mosaic(SDL_Texture *src, SDL_Texture *tmp, SDL_Texture *dst, int w, int h, double scale) {
	int cx = w / 2;
	int cy = h / 2;
	SDL_FRect fr = {
		.x = cx - cx / scale,
		.y = cy - cy / scale,
		.w = w / scale,
		.h = h / scale,
	};
	SDL_SetRenderTarget(sdl_renderer, tmp);
	SDL_RenderCopyF(sdl_renderer, src, NULL, &fr);

	SDL_Rect dstr = {
		.x = w * (1 - scale) / 2,
		.y = h * (1 - scale) / 2,
		.w = w * scale,
		.h = h * scale,
	};
	SDL_SetRenderTarget(sdl_renderer, dst);

#if SDL_VERSION_ATLEAST(2, 0, 12)
	SDL_SetTextureScaleMode(tmp, SDL_ScaleModeNearest);
#else
	// Should be okay because nearest is the default scaling mode.
#endif
	SDL_RenderCopy(sdl_renderer, tmp, NULL, &dstr);
	SDL_SetRenderTarget(sdl_renderer, NULL);
}

static void mosaic_step(struct sdl_effect *eff, double progress) {
	struct mosaic_effect *this = (struct mosaic_effect *)eff;

	if (eff->type == EFFECT_MOSAIC) {
		const int max_scale = 80;
		int scale = (1.0 - progress) * (max_scale - 1) + 1;
		mosaic(eff->tx_new, this->tmp_new, this->tx_mosaic, eff->dst_rect.w, eff->dst_rect.h, scale);
		SDL_RenderCopy(sdl_renderer, this->tx_mosaic, NULL, &eff->dst_rect);
	} else if (eff->type == EFFECT_CROSSFADE_MOSAIC) {
		const int max_scale = 96;
		double scale = (progress < 0.5 ? progress : 1.0 - progress) * 2 * (max_scale - 1) + 1;
		mosaic(eff->tx_old, this->tmp_old, this->tx_mosaic, eff->dst_rect.w, eff->dst_rect.h, scale);
		SDL_RenderCopy(sdl_renderer, this->tx_mosaic, NULL, &eff->dst_rect);
		mosaic(eff->tx_new, this->tmp_new, this->tx_mosaic, eff->dst_rect.w, eff->dst_rect.h, scale);
		SDL_SetTextureBlendMode(this->tx_mosaic, SDL_BLENDMODE_BLEND);
		SDL_SetTextureAlphaMod(this->tx_mosaic, progress * 255);
		SDL_RenderCopy(sdl_renderer, this->tx_mosaic, NULL, &eff->dst_rect);
		SDL_SetTextureBlendMode(this->tx_mosaic, SDL_BLENDMODE_NONE);
	} else {
		assert(!"Cannot happen");
	}

	SDL_RenderPresent(sdl_renderer);
}

static void mosaic_free(struct sdl_effect *eff) {
	struct mosaic_effect *this = (struct mosaic_effect *)eff;
	SDL_DestroyTexture(this->tmp_old);
	SDL_DestroyTexture(this->tmp_new);
	SDL_DestroyTexture(this->tx_mosaic);
	effect_finish(&this->eff, true);
	free(this);
}

// EFFECT_{FADE,WHITE}{OUT,IN}

static void brightness_step(struct sdl_effect *eff, double progress);
static void brightness_free(struct sdl_effect *eff);

static struct sdl_effect *brightness_new(SDL_Rect *rect, SDL_Surface *old, SDL_Surface *new, enum sdl_effect_type type) {
	struct sdl_effect *eff = calloc(1, sizeof(struct sdl_effect));
	if (!eff)
		NOMEMERR();
	effect_init(eff, rect, old, new, type);
	eff->step = brightness_step;
	eff->finish = brightness_free;
	return eff;
}

static void brightness_step(struct sdl_effect *eff, double progress) {
	SDL_Texture *texture;
	int color;
	int alpha;
	switch (eff->type) {
	case EFFECT_FADEOUT:
		texture = eff->tx_old;
		color = 0;
		alpha = progress * 255;
		break;
	case EFFECT_FADEOUT_FROM_NEW:
		texture = eff->tx_new;
		color = 0;
		alpha = progress * 255;
		break;
	case EFFECT_FADEIN:
		texture = eff->tx_new;
		color = 0;
		alpha = (1.0 - progress) * 255;
		break;
	case EFFECT_WHITEOUT:
		texture = eff->tx_old;
		color = 255;
		alpha = progress * 255;
		break;
	case EFFECT_WHITEOUT_FROM_NEW:
		texture = eff->tx_new;
		color = 255;
		alpha = progress * 255;
		break;
	case EFFECT_WHITEIN:
		texture = eff->tx_new;
		color = 255;
		alpha = (1.0 - progress) * 255;
		break;
	default:
		assert(!"Cannot happen");
	}
	SDL_RenderCopy(sdl_renderer, texture, NULL, &eff->dst_rect);
	SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(sdl_renderer, color, color, color, alpha);
	SDL_RenderFillRect(sdl_renderer, &eff->dst_rect);
	SDL_SetRenderDrawBlendMode(sdl_renderer, SDL_BLENDMODE_NONE);
	SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderPresent(sdl_renderer);
}

static void brightness_free(struct sdl_effect *eff) {
	effect_finish(eff, false);
	free(eff);
}

// EFFECT_BLIND_*

static void blind_step(struct sdl_effect *eff, double progress);
static void blind_free(struct sdl_effect *eff);

static struct sdl_effect *blind_new(SDL_Rect *rect, SDL_Surface *old, SDL_Surface *new, enum sdl_effect_type type) {
	struct sdl_effect *eff = calloc(1, sizeof(struct sdl_effect));
	if (!eff)
		NOMEMERR();
	effect_init(eff, rect, old, new, type);
	eff->step = blind_step;
	eff->finish = blind_free;
	return eff;
}

static void blind_vstep(double progress, SDL_Rect *rect, SDL_Texture *tx_new, int offset_y, bool flip) {
	const int N = 16;

	int nr_bands = rect->h / N + N - 1;
	int step = nr_bands * progress;

	int top = (step - (N - 1)) * N;
	if (top > 0) {
		if (top > rect->w)
			top = rect->w;
		SDL_Rect sr = {0, 0, rect->w, top};
		if (flip)
			flip_rect_v(&sr, rect->h);
		SDL_Rect dr = sr;
		move_rect(&dr, rect->x, rect->y);
		sr.y += offset_y;
		SDL_RenderCopy(sdl_renderer, tx_new, &sr, &dr);
	}
	for (int i = 1; i < N; i++) {
		int y = (step - i) * N;
		if (y < 0 || y >= rect->h)
			continue;
		SDL_Rect sr = {0, y, rect->w, i};
		if (flip)
			flip_rect_v(&sr, rect->h);
		SDL_Rect dr = sr;
		move_rect(&dr, rect->x, rect->y);
		sr.y += offset_y;
		SDL_RenderCopy(sdl_renderer, tx_new, &sr, &dr);
	}
}

static void blind_step(struct sdl_effect *eff, double progress) {
	const int N = 16;

	SDL_RenderCopy(sdl_renderer, eff->tx_old, NULL, &eff->dst_rect);

	if (eff->type == EFFECT_BLIND_DOWN || eff->type == EFFECT_BLIND_UP || eff->type == EFFECT_BLIND_DOWN_LR) {
		blind_vstep(progress, &eff->dst_rect, eff->tx_new, 0, eff->type == EFFECT_BLIND_UP);
	}
	if (eff->type == EFFECT_BLIND_UP_DOWN) {
		SDL_Rect top_rect = eff->dst_rect;
		top_rect.h = top_rect.h / (N * 2) * N;
		blind_vstep(progress, &top_rect, eff->tx_new, 0, false);

		SDL_Rect bottom_rect = eff->dst_rect;
		bottom_rect.y += top_rect.h;
		bottom_rect.h -= top_rect.h;
		blind_vstep(progress, &bottom_rect, eff->tx_new, top_rect.h, true);
	}

	if (eff->type == EFFECT_BLIND_LR || eff->type == EFFECT_BLIND_RL || eff->type == EFFECT_BLIND_DOWN_LR) {
		int nr_bands = eff->dst_rect.w / N + N - 1;
		int step = nr_bands * progress;

		int lhs = (step - (N - 1)) * N;
		if (lhs > 0) {
			if (lhs > eff->dst_rect.w)
				lhs = eff->dst_rect.w;
			SDL_Rect sr = {0, 0, lhs, eff->dst_rect.h};
			if (eff->type == EFFECT_BLIND_RL)
				flip_rect_h(&sr, eff->dst_rect.w);
			SDL_Rect dr = sr;
			move_rect(&dr, eff->dst_rect.x, eff->dst_rect.y);
			SDL_RenderCopy(sdl_renderer, eff->tx_new, &sr, &dr);
		}
		for (int i = 1; i < N; i++) {
			int x = (step - i) * N;
			if (x < 0 || x >= eff->dst_rect.w)
				continue;
			SDL_Rect sr = {x, 0, i, eff->dst_rect.h};
			if (eff->type == EFFECT_BLIND_RL)
				flip_rect_h(&sr, eff->dst_rect.w);
			SDL_Rect dr = sr;
			move_rect(&dr, eff->dst_rect.x, eff->dst_rect.y);
			SDL_RenderCopy(sdl_renderer, eff->tx_new, &sr, &dr);
		}
	}

	SDL_RenderPresent(sdl_renderer);
}

static void blind_free(struct sdl_effect *eff) {
	effect_finish(eff, false);
	free(eff);
}

// EFFECT_ZOOM_BLEND_BLUR

#define ZOOM_BLEND_BLUR_STEPS 6

struct zoom_blend_blur_effect {
	struct sdl_effect eff;
	SDL_Texture *tx[ZOOM_BLEND_BLUR_STEPS];
	int index;
};

static void zoom_blend_blur_step(struct sdl_effect *eff, double progress);
static void zoom_blend_blur_free(struct sdl_effect *eff);

static struct sdl_effect *zoom_blend_blur_new(SDL_Rect *rect, SDL_Surface *old, SDL_Surface *new) {
	if (!SDL_RenderTargetSupported(sdl_renderer))
		return fallback_effect_new(rect, old, new, EFFECT_ZOOM_BLEND_BLUR);

	struct zoom_blend_blur_effect *eff = calloc(1, sizeof(struct zoom_blend_blur_effect));
	if (!eff)
		NOMEMERR();
	effect_init(&eff->eff, rect, old, new, EFFECT_ZOOM_BLEND_BLUR);
	eff->eff.step = zoom_blend_blur_step;
	eff->eff.finish = zoom_blend_blur_free;
	return &eff->eff;
}

static void zoom_blend_blur_step(struct sdl_effect *eff, double progress) {
	struct zoom_blend_blur_effect *this = (struct zoom_blend_blur_effect *)eff;

	if (!this->tx[this->index]) {
		this->tx[this->index] = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, eff->dst_rect.w, eff->dst_rect.h);
	}

	double scale = (progress < 0.5 ? 1 - progress * 2 : (progress - 0.5) * 2) * 0.9 + 0.1;  // 0.1 - 1.0
	int w = eff->dst_rect.w * scale;
	int h = eff->dst_rect.h * scale;
	SDL_Rect r = {
		.x = (eff->dst_rect.w - w) / 2,
		.y = (eff->dst_rect.h - h) / 2,
		.w = w,
		.h = h
	};
	SDL_SetRenderTarget(sdl_renderer, this->tx[this->index]);
	SDL_RenderCopy(sdl_renderer, eff->tx_new, &r, NULL);
	SDL_SetRenderTarget(sdl_renderer, NULL);

	this->index = (this->index + 1) % ZOOM_BLEND_BLUR_STEPS;

	SDL_RenderFillRect(sdl_renderer, &eff->dst_rect);
	for (int i = 0; i < ZOOM_BLEND_BLUR_STEPS; i++) {
		SDL_Texture *tx = this->tx[i] ? this->tx[i] : eff->tx_new;
		SDL_SetTextureBlendMode(tx, SDL_BLENDMODE_ADD);
		SDL_SetTextureAlphaMod(tx, 255 / ZOOM_BLEND_BLUR_STEPS);
		SDL_RenderCopy(sdl_renderer, tx, NULL, &eff->dst_rect);
		SDL_SetTextureBlendMode(tx, SDL_BLENDMODE_NONE);
		SDL_SetTextureAlphaMod(tx, 255);
	}
	SDL_RenderPresent(sdl_renderer);
}

static void zoom_blend_blur_free(struct sdl_effect *eff) {
	struct zoom_blend_blur_effect *this = (struct zoom_blend_blur_effect *)eff;
	for (int i = 0; i < ZOOM_BLEND_BLUR_STEPS; i++) {
		if (this->tx[i])
			SDL_DestroyTexture(this->tx[i]);
	}
	effect_finish(&this->eff, true);
	free(this);
}

// EFFECT_LINEAR_BLUR, EFFECT_LINEAR_BLUR_VERT

#define LINEAR_BLUR_STEPS 10

struct linear_blur_effect {
	struct sdl_effect eff;
	SDL_Texture *blurred_old[LINEAR_BLUR_STEPS];
	SDL_Texture *blurred_new[LINEAR_BLUR_STEPS];
};

static void linear_blur_step(struct sdl_effect *eff, double progress);
static void linear_blur_free(struct sdl_effect *eff);

static struct sdl_effect *linear_blur_new(SDL_Rect *rect, SDL_Surface *old, SDL_Surface *new, enum sdl_effect_type type) {
	if (!SDL_RenderTargetSupported(sdl_renderer))
		return fallback_effect_new(rect, old, new, type);

	struct linear_blur_effect *lbe = calloc(1, sizeof(struct linear_blur_effect));
	if (!lbe)
		NOMEMERR();
	effect_init(&lbe->eff, rect, old, new, type);
	lbe->eff.step = linear_blur_step;
	lbe->eff.finish = linear_blur_free;
	return &lbe->eff;
}

static SDL_Texture *blur(struct linear_blur_effect *this, SDL_Texture *src, int stride) {
	bool vertical = this->eff.type == EFFECT_LINEAR_BLUR_VERT;
	int w = this->eff.dst_rect.w;
	int h = this->eff.dst_rect.h;
	SDL_Texture *dst = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, w, h);

	SDL_SetRenderTarget(sdl_renderer, dst);
	SDL_RenderClear(sdl_renderer);
	SDL_SetTextureBlendMode(src, SDL_BLENDMODE_ADD);
	SDL_SetTextureAlphaMod(src, 127);

	SDL_Rect dr = {0, 0, w, h};
	*(vertical ? &dr. y : &dr.x) = -stride / 2;
	SDL_RenderCopy(sdl_renderer, src, NULL, &dr);
	*(vertical ? &dr. y : &dr.x) = max(1, stride / 2);
	SDL_RenderCopy(sdl_renderer, src, NULL, &dr);

	// Copy the edge pixels to emulate texture clamping.
	SDL_Rect sr = {0, 0, vertical ? w : 1, vertical ? 1 : h};
	*(vertical ? &dr.h : &dr.w) = max(1, stride / 2);
	*(vertical ? &dr.y : &dr.x) = 0;
	SDL_RenderCopy(sdl_renderer, src, &sr, &dr);
	if (stride > 1) {
		if (vertical) {
			sr.y = h - 1;
			dr.y = h - stride / 2;
		} else {
			sr.x = w - 1;
			dr.x = w - stride / 2;
		}
		SDL_RenderCopy(sdl_renderer, src, &sr, &dr);
	}

	SDL_SetTextureAlphaMod(src, 255);
	SDL_SetTextureBlendMode(src, SDL_BLENDMODE_NONE);
	SDL_SetRenderTarget(sdl_renderer, NULL);

	return dst;
}

static void linear_blur_step(struct sdl_effect *eff, double progress) {
	struct linear_blur_effect *this = (struct linear_blur_effect *)eff;

	int step = progress * LINEAR_BLUR_STEPS * 2;
	if (step == LINEAR_BLUR_STEPS * 2) {
		SDL_RenderCopy(sdl_renderer, eff->tx_new, NULL, NULL);
		SDL_RenderPresent(sdl_renderer);
		return;
	}

	for (int i = 0; i <= step && i < LINEAR_BLUR_STEPS; i++) {
		if (this->blurred_old[i])
			continue;
		this->blurred_old[i] = blur(this, i ? this->blurred_old[i - 1] : eff->tx_old, 1 << i);
		this->blurred_new[i] = blur(this, i ? this->blurred_new[i - 1] : eff->tx_new, 1 << i);
	}

	int i = step < LINEAR_BLUR_STEPS ? step : LINEAR_BLUR_STEPS * 2 - 1 - step;
	SDL_Texture *old = this->blurred_old[i];
	SDL_Texture *new = this->blurred_new[i];
	SDL_RenderCopy(sdl_renderer, old, NULL, &eff->dst_rect);
	SDL_SetTextureBlendMode(new, SDL_BLENDMODE_BLEND);
	SDL_SetTextureAlphaMod(new, 255 * progress);
	SDL_RenderCopy(sdl_renderer, new, NULL, &eff->dst_rect);
	SDL_RenderPresent(sdl_renderer);
}

static void linear_blur_free(struct sdl_effect *eff) {
	struct linear_blur_effect *this = (struct linear_blur_effect *)eff;
	for (int i = 0; i < LINEAR_BLUR_STEPS; i++) {
		if (this->blurred_old[i])
			SDL_DestroyTexture(this->blurred_old[i]);
		if (this->blurred_new[i])
			SDL_DestroyTexture(this->blurred_new[i]);
	}
	effect_finish(&this->eff, true);
	free(this);
}

// EFFECT_PENTAGRAM_*, EFFECT_HEXAGRAM_*, EFFECT_WINDMILL*

struct polygon_mask_effect {
	struct sdl_effect f;
	SDL_Texture *tx_tmp;
};

static void polygon_mask_step(struct sdl_effect *eff, double progress);
static void polygon_mask_free(struct sdl_effect *eff);

static struct sdl_effect *polygon_mask_new(SDL_Rect *rect, SDL_Surface *old, SDL_Surface *new, enum sdl_effect_type type) {
#if HAS_SDL_RenderGeometry
	if (SDL_RenderTargetSupported(sdl_renderer)) {
		struct polygon_mask_effect *pmf = calloc(1, sizeof(struct polygon_mask_effect));
		if (!pmf)
			NOMEMERR();
		effect_init(&pmf->f, rect, old, new, type);
		pmf->tx_tmp = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, rect->w, rect->h);
		pmf->f.step = polygon_mask_step;
		pmf->f.finish = polygon_mask_free;
		return &pmf->f;
	}
#endif // HAS_SDL_RenderGeometry

	return fallback_effect_new(rect, old, new, type);
}

#if HAS_SDL_RenderGeometry

static void draw_pentagram(int center_x, int center_y, double radius, double rotate) {
	const SDL_FPoint p[10] = {
		{ 1.5,     0     },
		{ 0.4635,  0.3368},
		{ 0.4635,  1.4266},
		{-0.1771,  0.5449},
		{-1.2135,  0.8817},
		{-0.5729,  0     },
		{-1.2135, -0.8817},
		{-0.1771, -0.5449},
		{ 0.4635, -1.4266},
		{ 0.4635, -0.3368},
	};
	SDL_Vertex v[10];
	double sin_r = sin(rotate);
	double cos_r = cos(rotate);
	for (int i = 0; i < 10; i++) {
		v[i].position.x = radius * (p[i].x * cos_r - p[i].y * sin_r) + center_x;
		v[i].position.y = radius * (p[i].x * sin_r + p[i].y * cos_r) + center_y;
		v[i].color = (SDL_Color){0, 0, 0, 0};
	}
	const int indices[15] = {
		0, 3, 7,
		2, 5, 9,
		4, 7, 1,
		6, 9, 3,
		8, 1, 5
	};
	SDL_RenderGeometry(sdl_renderer, NULL, v, 10, indices, 15);
}

static void draw_hexagram(int center_x, int center_y, double radius, double rotate) {
	const SDL_FPoint p[6] = {
		{ 0,     -1.0},
		{-0.866,  0.5},
		{ 0.866,  0.5},

		{-0.866, -0.5},
		{ 0,      1.0},
		{ 0.866, -0.5}
	};
	SDL_Vertex v[6];
	double sin_r = sin(rotate);
	double cos_r = cos(rotate);
	for (int i = 0; i < 6; i++) {
		v[i].position.x = radius * (p[i].x * cos_r - p[i].y * sin_r) + center_x;
		v[i].position.y = radius * (p[i].x * sin_r + p[i].y * cos_r) + center_y;
		v[i].color = (SDL_Color){0, 0, 0, 0};
	}
	SDL_RenderGeometry(sdl_renderer, NULL, v, 6, NULL, 0);
}

static void draw_windmill_90(int w, int h, double theta) {
	double cx = w / 2.0;
	double cy = h / 2.0;
	double r = max(cx, cy) * sqrt(2);
	double r_sin = r * sin(theta);
	double r_cos = r * cos(theta);

	SDL_Vertex v[13] = {
		// center
		{.position = {cx,         cy        }},
		// top-right
		{.position = {cx,         cy - r    }},
		{.position = {cx + r,     cy - r    }},
		{.position = {cx + r_sin, cy - r_cos}},
		// bottom-right
		{.position = {cx + r,     cy        }},
		{.position = {cx + r,     cy + r    }},
		{.position = {cx + r_cos, cy + r_sin}},
		// bottlm-left
		{.position = {cx,         cy + r    }},
		{.position = {cx - r,     cy + r    }},
		{.position = {cx - r_sin, cy + r_cos}},
		// top-left
		{.position = {cx - r,     cy        }},
		{.position = {cx - r,     cy - r    }},
		{.position = {cx - r_cos, cy - r_sin}},
	};

	if (theta <= M_PI / 4) {
		const int indices[12] = {
			0, 1, 3,
			0, 4, 6,
			0, 7, 9,
			0, 10, 12
		};
		SDL_RenderGeometry(sdl_renderer, NULL, v, 13, indices, 12);
	} else {
		const int indices[24] = {
			0, 1, 2,
			0, 2, 3,
			0, 4, 5,
			0, 5, 6,
			0, 7, 8,
			0, 8, 9,
			0, 10, 11,
			0, 11, 12
		};
		SDL_RenderGeometry(sdl_renderer, NULL, v, 13, indices, 24);
	}
}

static void draw_windmill_180(int w, int h, double theta) {
	double cx = w / 2.0;
	double cy = h / 2.0;
	double r = max(cx, cy) * sqrt(2);
	double r_sin = r * sin(theta);
	double r_cos = r * cos(theta);

	SDL_Vertex v[11] = {
		{.position = {cx,         cy        }},  // center

		{.position = {cx - r,     cy - r    }},  // left-top
		{.position = {cx,         cy - r    }},  // top
		{.position = {cx + r,     cy - r    }},  // top-right
		{.position = {cx + r,     cy        }},  // right
		{.position = {cx - r_cos, cy - r_sin}},  // theta

		{.position = {cx + r,     cy + r    }},  // right-bottom
		{.position = {cx,         cy + r    }},  // bottom
		{.position = {cx - r,     cy + r    }},  // left-bottom
		{.position = {cx - r,     cy        }},  // left
		{.position = {cx + r_cos, cy + r_sin}},  // theta+M_PI
	};

	int indices[24];
	int i = 0;

	indices[i++] = 0;  // center
	indices[i++] = 9;  // left
	for (int j = 1; theta > M_PI / 4 * j; j++) {
		indices[i++] = j;
		indices[i++] = 0;  // center
		indices[i++] = j;
	}
	indices[i++] = 5;  // theta

	indices[i++] = 0;  // center
	indices[i++] = 4;  // right
	for (int j = 1; theta > M_PI / 4 * j; j++) {
		indices[i++] = j + 5;
		indices[i++] = 0;  // center
		indices[i++] = j + 5;
	}
	indices[i++] = 10;  // theta+M_PI

	SDL_RenderGeometry(sdl_renderer, NULL, v, 11, indices, i);
}

static void draw_windmill_360(int w, int h, double theta) {
	double cx = w / 2.0;
	double cy = h / 2.0;
	double r = max(cx, cy) * sqrt(2);
	double r_sin = r * sin(theta);
	double r_cos = r * cos(theta);

	SDL_Vertex v[10] = {
		{.position = {cx,         cy        }},  // center
		{.position = {cx - r,     cy - r    }},  // left-top
		{.position = {cx,         cy - r    }},  // top
		{.position = {cx + r,     cy - r    }},  // top-right
		{.position = {cx + r,     cy        }},  // right
		{.position = {cx + r,     cy + r    }},  // right-bottom
		{.position = {cx,         cy + r    }},  // bottom
		{.position = {cx - r,     cy + r    }},  // left-bottom
		{.position = {cx - r,     cy        }},  // left
		{.position = {cx - r_cos, cy - r_sin}},  // theta
	};

	int indices[24];
	int i = 0;

	indices[i++] = 0;  // center
	indices[i++] = 8;  // left
	for (int j = 1; theta > M_PI / 4 * j; j++) {
		indices[i++] = j;
		indices[i++] = 0;  // center
		indices[i++] = j;
	}
	indices[i++] = 9;  // theta
	SDL_RenderGeometry(sdl_renderer, NULL, v, 10, indices, i);
}

static void polygon_mask_step(struct sdl_effect *eff, double progress) {
	struct polygon_mask_effect *this = (struct polygon_mask_effect *)eff;
	int w = eff->dst_rect.w;
	int h = eff->dst_rect.h;
	SDL_SetRenderTarget(sdl_renderer, this->tx_tmp);
	SDL_RenderCopy(sdl_renderer, eff->tx_old, NULL, NULL);
	switch (eff->type) {
	case EFFECT_PENTAGRAM_IN_OUT:
		draw_pentagram(w / 2, h / 2, max(w, h) * progress, M_PI * progress);
		break;
	case EFFECT_PENTAGRAM_OUT_IN:
		draw_pentagram(w / 2, h / 2, max(w, h) * (1 - progress), M_PI * progress);
		break;
	case EFFECT_HEXAGRAM_IN_OUT:
		draw_hexagram(w / 2, h / 2, max(w, h) * progress, M_PI * progress);
		break;
	case EFFECT_HEXAGRAM_OUT_IN:
		draw_hexagram(w / 2, h / 2, max(w, h) * (1 - progress), M_PI * progress);
		break;
	case EFFECT_WINDMILL:
		draw_windmill_90(w, h, M_PI / 2 * progress);
		break;
	case EFFECT_WINDMILL_180:
		draw_windmill_180(w, h, M_PI * progress);
		break;
	case EFFECT_WINDMILL_360:
		draw_windmill_360(w, h, M_PI * 2 * progress);
		break;
	default:
		assert(!"Cannot happen");
	}
	SDL_SetRenderTarget(sdl_renderer, NULL);
	SDL_RenderCopy(sdl_renderer, eff->tx_new, NULL, &eff->dst_rect);
	SDL_SetTextureBlendMode(this->tx_tmp, SDL_BLENDMODE_BLEND);
	SDL_RenderCopy(sdl_renderer, this->tx_tmp, NULL, &eff->dst_rect);
	SDL_RenderPresent(sdl_renderer);
}

static void polygon_mask_free(struct sdl_effect *eff) {
	struct polygon_mask_effect *this = (struct polygon_mask_effect *)eff;
	SDL_DestroyTexture(this->tx_tmp);
	effect_finish(&this->f, true);
	free(this);
}

#endif // HAS_SDL_RenderGeometry

// EFFECT_ROTATE_*

static void rotate_step(struct sdl_effect *eff, double progress);
static void rotate_free(struct sdl_effect *eff);

static struct sdl_effect *rotate_new(SDL_Rect *rect, SDL_Surface *old, SDL_Surface *new, enum sdl_effect_type type) {
	struct sdl_effect *eff = calloc(1, sizeof(struct sdl_effect));
	if (!eff)
		NOMEMERR();
	effect_init(eff, rect, old, new, type);
	eff->step = rotate_step;
	eff->finish = rotate_free;
	return eff;
}

static void rotate_step(struct sdl_effect *eff, double progress) {
	SDL_Texture *bg_texture, *fg_texture;
	double angle = progress * 360;
	double scale = progress;
	switch (eff->type) {
	case EFFECT_ROTATE_OUT:
		scale = 1.0 - scale;
		bg_texture = eff->tx_new;
		fg_texture = eff->tx_old;
		angle *= -1.0;
		break;
	case EFFECT_ROTATE_IN:
		bg_texture = eff->tx_old;
		fg_texture = eff->tx_new;
		angle *= -1.0;
		break;
	case EFFECT_ROTATE_OUT_CW:
		scale = 1.0 - scale;
		bg_texture = eff->tx_new;
		fg_texture = eff->tx_old;
		break;
	case EFFECT_ROTATE_IN_CW:
		bg_texture = eff->tx_old;
		fg_texture = eff->tx_new;
		break;
	case EFFECT_ZOOM_IN:
		bg_texture = eff->tx_old;
		fg_texture = eff->tx_new;
		angle = 0;
		break;
	default:
		assert(!"Cannot happen");
	}
	SDL_Rect r = eff->dst_rect;
	r.x += (1.0 - scale) * r.w / 2;
	r.y += (1.0 - scale) * r.h / 2;
	r.w *= scale;
	r.h *= scale;
	SDL_RenderCopy(sdl_renderer, bg_texture, NULL, &eff->dst_rect);
	SDL_RenderCopyEx(sdl_renderer, fg_texture, NULL, &r, angle, NULL, SDL_FLIP_NONE);
	SDL_RenderPresent(sdl_renderer);
}

static void rotate_free(struct sdl_effect *eff) {
	effect_finish(eff, true);
	free(eff);
}

// EFFECT_ZIGZAG_CROSSFADE

struct zigzag_crossfade_effect {
	struct sdl_effect eff;
	SDL_Texture *tmp1, *tmp2;
};

static void zigzag_crossfade_step(struct sdl_effect *eff, double progress);
static void zigzag_crossfade_free(struct sdl_effect *eff);

static struct sdl_effect *zigzag_crossfade_new(SDL_Rect *rect, SDL_Surface *old, SDL_Surface *new) {
	if (!SDL_RenderTargetSupported(sdl_renderer))
		return fallback_effect_new(rect, old, new, EFFECT_ZIGZAG_CROSSFADE);

	struct zigzag_crossfade_effect *eff = calloc(1, sizeof(struct zigzag_crossfade_effect));
	if (!eff)
		NOMEMERR();
	effect_init(&eff->eff, rect, old, new, EFFECT_ZIGZAG_CROSSFADE);
	eff->tmp1 = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, rect->w, rect->h);
	eff->tmp2 = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, rect->w, rect->h);
	eff->eff.step = zigzag_crossfade_step;
	eff->eff.finish = zigzag_crossfade_free;
	return &eff->eff;
}

static void wave_warp_h(SDL_Texture *src, SDL_Texture *dst, int w, int h, double amplitude, double length, double phase) {
	SDL_SetRenderTarget(sdl_renderer, dst);
	SDL_RenderClear(sdl_renderer);
	SDL_Rect sr = {0, 0, w, 1};
	SDL_Rect dr = {0, 0, w, 1};
	for (int y = 0; y < h; y++) {
		sr.y = dr.y = y;
		dr.x = sin(phase + y * (2 * M_PI / 360 * length)) * amplitude;
		SDL_RenderCopy(sdl_renderer, src, &sr, &dr);
	}
	SDL_SetRenderTarget(sdl_renderer, NULL);
}

static void wave_warp_v(SDL_Texture *src, SDL_Texture *dst, int w, int h, double amplitude, double length, double phase) {
	SDL_SetRenderTarget(sdl_renderer, dst);
	SDL_RenderClear(sdl_renderer);
	SDL_Rect sr = {0, 0, 1, h};
	SDL_Rect dr = {0, 0, 1, h};
	for (int x = 0; x < w; x++) {
		sr.x = dr.x = x;
		dr.y = sin(phase + x * (2 * M_PI / 360 * length)) * amplitude;
		SDL_RenderCopy(sdl_renderer, src, &sr, &dr);
	}
	SDL_SetRenderTarget(sdl_renderer, NULL);
}

static void zigzag_crossfade_step(struct sdl_effect *eff, double progress) {
	struct zigzag_crossfade_effect *this = (struct zigzag_crossfade_effect *)eff;

	double amp = (progress < 0.5 ? progress : 1.0 - progress) * 80;
	double len = (progress < 0.5 ? progress : 1.0 - progress) * 20;
	double phase = progress * 10;

	wave_warp_h(eff->tx_old, this->tmp1, eff->dst_rect.w, eff->dst_rect.h, amp, len, phase);
	wave_warp_v(this->tmp1, this->tmp2, eff->dst_rect.w, eff->dst_rect.h, amp, len, phase);
	SDL_RenderCopy(sdl_renderer, this->tmp2, NULL, &eff->dst_rect);

	wave_warp_h(eff->tx_new, this->tmp1, eff->dst_rect.w, eff->dst_rect.h, amp, len, phase);
	wave_warp_v(this->tmp1, this->tmp2, eff->dst_rect.w, eff->dst_rect.h, amp, len, phase);
	SDL_SetTextureBlendMode(this->tmp2, SDL_BLENDMODE_BLEND);
	SDL_SetTextureAlphaMod(this->tmp2, progress * 255);
	SDL_RenderCopy(sdl_renderer, this->tmp2, NULL, &eff->dst_rect);
	SDL_SetTextureBlendMode(this->tmp2, SDL_BLENDMODE_NONE);

	SDL_RenderPresent(sdl_renderer);
}

static void zigzag_crossfade_free(struct sdl_effect *eff) {
	struct zigzag_crossfade_effect *this = (struct zigzag_crossfade_effect *)eff;
	SDL_DestroyTexture(this->tmp1);
	SDL_DestroyTexture(this->tmp2);
	effect_finish(&this->eff, true);
	free(this);
}


static SDL_Surface *create_surface(agsurface_t *as, int x, int y, int w, int h) {
	SDL_Surface *sf = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_RGB888);
	SDL_Rect rect = { x, y, w, h };
	if (!as) {
		SDL_BlitSurface(sdl_display, &rect, sf, NULL);
	} else if (as == sdl_dibinfo) {
		SDL_BlitSurface(sdl_dib, &rect, sf, NULL);
	} else {
		SDL_Surface *s = com2surface(as);
		SDL_BlitSurface(s, &rect, sf, NULL);
		SDL_FreeSurface(s);
	}
	return sf;
}

struct sdl_effect *sdl_effect_init(SDL_Rect *rect, agsurface_t *old, int ox, int oy, agsurface_t *new, int nx, int ny, enum sdl_effect_type type) {
	SDL_Surface *sf_old = create_surface(old, ox, oy, rect->w, rect->h);
	SDL_Surface *sf_new = create_surface(new, nx, ny, rect->w, rect->h);

	switch (type) {
	case EFFECT_CROSSFADE:
		return crossfade_new(rect, sf_old, sf_new);
	case EFFECT_CROSSFADE_DOWN:
	case EFFECT_CROSSFADE_UP:
	case EFFECT_CROSSFADE_LR:
	case EFFECT_CROSSFADE_RL:
		return crossfade_animation_new(rect, sf_old, sf_new, type);
	case EFFECT_MOSAIC:
	case EFFECT_CROSSFADE_MOSAIC:
		return mosaic_new(rect, sf_old, sf_new, type);
	case EFFECT_FADEOUT:
	case EFFECT_FADEOUT_FROM_NEW:
	case EFFECT_FADEIN:
	case EFFECT_WHITEOUT:
	case EFFECT_WHITEOUT_FROM_NEW:
	case EFFECT_WHITEIN:
		return brightness_new(rect, sf_old, sf_new, type);
	case EFFECT_BLIND_DOWN:
	case EFFECT_BLIND_UP:
	case EFFECT_BLIND_LR:
	case EFFECT_BLIND_RL:
	case EFFECT_BLIND_UP_DOWN:
	case EFFECT_BLIND_DOWN_LR:
		return blind_new(rect, sf_old, sf_new, type);
	case EFFECT_ZOOM_BLEND_BLUR:
		return zoom_blend_blur_new(rect, sf_old, sf_new);
	case EFFECT_LINEAR_BLUR:
	case EFFECT_LINEAR_BLUR_VERT:
		return linear_blur_new(rect, sf_old, sf_new, type);
	case EFFECT_PENTAGRAM_IN_OUT:
	case EFFECT_PENTAGRAM_OUT_IN:
	case EFFECT_HEXAGRAM_IN_OUT:
	case EFFECT_HEXAGRAM_OUT_IN:
	case EFFECT_WINDMILL:
	case EFFECT_WINDMILL_180:
	case EFFECT_WINDMILL_360:
		return polygon_mask_new(rect, sf_old, sf_new, type);
	case EFFECT_ROTATE_OUT:
	case EFFECT_ROTATE_IN:
	case EFFECT_ROTATE_OUT_CW:
	case EFFECT_ROTATE_IN_CW:
	case EFFECT_ZOOM_IN:
		return rotate_new(rect, sf_old, sf_new, type);
	case EFFECT_ZIGZAG_CROSSFADE:
		return zigzag_crossfade_new(rect, sf_old, sf_new);
	default:
		WARNING("Unknown effect %d\n", type);
		return crossfade_new(rect, sf_old, sf_new);
	}
}

void sdl_effect_step(struct sdl_effect *eff, double progress) {
	SDL_RenderClear(sdl_renderer);
	if (!eff->is_fullscreen)
		SDL_RenderCopy(sdl_renderer, sdl_texture, NULL, NULL);
	eff->step(eff, progress);
}

void sdl_effect_finish(struct sdl_effect *eff) {
	eff->finish(eff);
}
