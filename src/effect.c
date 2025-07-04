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
#include "gfx.h"
#include "effect.h"
#include "gfx_private.h"

#define M_PIf ((float)M_PI)
#define HAS_SDL_RenderGeometry SDL_VERSION_ATLEAST(2, 0, 18)

enum effect_type from_nact_effect(enum nact_effect effect) {
	switch (effect) {
	case NACT_EFFECT_PAN_IN_DOWN:       return EFFECT_PAN_IN_DOWN;
	case NACT_EFFECT_PAN_IN_UP:         return EFFECT_PAN_IN_UP;
	case NACT_EFFECT_SKIP_LINE_UP_DOWN: return EFFECT_SKIP_LINE_UP_DOWN;
	case NACT_EFFECT_SKIP_LINE_LR_RL:   return EFFECT_SKIP_LINE_LR_RL;
	case NACT_EFFECT_WIPE_IN:           return EFFECT_WIPE_IN;
	case NACT_EFFECT_WIPE_OUT:          return EFFECT_WIPE_OUT;
	case NACT_EFFECT_ZOOM_IN:           return EFFECT_ZOOM_IN;
	case NACT_EFFECT_BLIND_DOWN:        return EFFECT_BLIND_DOWN;
	case NACT_EFFECT_WIPE_LR:           return EFFECT_WIPE_LR;
	case NACT_EFFECT_WIPE_RL:           return EFFECT_WIPE_RL;
	case NACT_EFFECT_WIPE_DOWN:         return EFFECT_WIPE_DOWN;
	case NACT_EFFECT_WIPE_UP:           return EFFECT_WIPE_UP;
	case NACT_EFFECT_WIPE_OUT_V:        return EFFECT_WIPE_OUT_V;
	case NACT_EFFECT_WIPE_IN_V:         return EFFECT_WIPE_IN_V;
	case NACT_EFFECT_WIPE_OUT_H:        return EFFECT_WIPE_OUT_H;
	case NACT_EFFECT_WIPE_IN_H:         return EFFECT_WIPE_IN_H;
	case NACT_EFFECT_MOSAIC:            return EFFECT_MOSAIC;
	case NACT_EFFECT_CIRCLE_WIPE_OUT:   return EFFECT_CIRCLE_WIPE_OUT;
	case NACT_EFFECT_CIRCLE_WIPE_IN:    return EFFECT_CIRCLE_WIPE_IN;
	case NACT_EFFECT_FADEIN:            return EFFECT_FADEIN;
	case NACT_EFFECT_WHITEIN:           return EFFECT_WHITEIN;
	case NACT_EFFECT_FADEOUT:           return EFFECT_FADEOUT_FROM_NEW;
	case NACT_EFFECT_WHITEOUT:          return EFFECT_WHITEOUT_FROM_NEW;
	case NACT_EFFECT_CROSSFADE:         return EFFECT_CROSSFADE;
	case NACT_EFFECT_CROSSFADE_MOSAIC:  return EFFECT_CROSSFADE_MOSAIC;
	case NACT_EFFECT_BLIND_UP:          return EFFECT_BLIND_UP;
	case NACT_EFFECT_BLIND_UP_DOWN:     return EFFECT_BLIND_UP_DOWN;
	case NACT_EFFECT_CROSSFADE_DOWN:    return EFFECT_CROSSFADE_DOWN;
	case NACT_EFFECT_CROSSFADE_UP:      return EFFECT_CROSSFADE_UP;
	case NACT_EFFECT_CROSSFADE_LR:      return EFFECT_CROSSFADE_LR;
	case NACT_EFFECT_CROSSFADE_RL:      return EFFECT_CROSSFADE_RL;
	case NACT_EFFECT_BLEND_UP_DOWN:     return EFFECT_BLEND_UP_DOWN;
	case NACT_EFFECT_BLEND_LR_RL:       return EFFECT_BLEND_LR_RL;
	case NACT_EFFECT_CROSSFADE_LR_RL:   return EFFECT_CROSSFADE_LR_RL;
	case NACT_EFFECT_CROSSFADE_UP_DOWN: return EFFECT_CROSSFADE_UP_DOWN;
	case NACT_EFFECT_MAGNIFY:           return EFFECT_MAGNIFY;
	case NACT_EFFECT_PENTAGRAM_IN_OUT:  return EFFECT_PENTAGRAM_IN_OUT;
	case NACT_EFFECT_PENTAGRAM_OUT_IN:  return EFFECT_PENTAGRAM_OUT_IN;
	case NACT_EFFECT_HEXAGRAM_IN_OUT:   return EFFECT_HEXAGRAM_IN_OUT;
	case NACT_EFFECT_HEXAGRAM_OUT_IN:   return EFFECT_HEXAGRAM_OUT_IN;
	case NACT_EFFECT_BLIND_LR:          return EFFECT_BLIND_LR;
	case NACT_EFFECT_BLIND_RL:          return EFFECT_BLIND_RL;
	case NACT_EFFECT_WINDMILL:          return EFFECT_WINDMILL;
	case NACT_EFFECT_WINDMILL_180:      return EFFECT_WINDMILL_180;
	case NACT_EFFECT_WINDMILL_360:      return EFFECT_WINDMILL_360;
	case NACT_EFFECT_LINEAR_BLUR:       return EFFECT_LINEAR_BLUR;
	default:                            return EFFECT_INVALID;
	}
}

enum effect_type from_nact_sprite_effect(enum nact_effect effect) {
	switch (effect) {
	case NACT_EFFECT_PAN_IN_DOWN:       return EFFECT_PAN_IN_DOWN;
	case NACT_EFFECT_PAN_IN_UP:         return EFFECT_PAN_IN_UP;
	case NACT_EFFECT_SKIP_LINE_UP_DOWN: return EFFECT_SKIP_LINE_UP_DOWN;
	case NACT_EFFECT_SKIP_LINE_LR_RL:   return EFFECT_SKIP_LINE_LR_RL;
	case NACT_SP_EFFECT_RASTER_BLEND:   return EFFECT_RASTER_BLEND;
	default:                            return EFFECT_INVALID;
	}
}

enum effect_type from_sact_effect(enum sact_effect effect) {
	switch (effect) {
	case SACT_EFFECT_CROSSFADE:           return EFFECT_CROSSFADE;
	case SACT_EFFECT_FADEOUT:             return EFFECT_FADEOUT;
	case SACT_EFFECT_FADEIN:              return EFFECT_FADEIN;
	case SACT_EFFECT_WHITEOUT:            return EFFECT_WHITEOUT;
	case SACT_EFFECT_WHITEIN:             return EFFECT_WHITEIN;
	case SACT_EFFECT_CROSSFADE_MOSAIC:    return EFFECT_CROSSFADE_MOSAIC;
	case SACT_EFFECT_BLIND_DOWN:          return EFFECT_BLIND_DOWN;
	case SACT_EFFECT_BLIND_LR:            return EFFECT_BLIND_LR;
	case SACT_EFFECT_BLIND_DOWN_LR:       return EFFECT_BLIND_DOWN_LR;
	case SACT_EFFECT_ZOOM_BLEND_BLUR:     return EFFECT_ZOOM_BLEND_BLUR;
	case SACT_EFFECT_LINEAR_BLUR:         return EFFECT_LINEAR_BLUR;
	case SACT_EFFECT_CROSSFADE_DOWN:      return EFFECT_CROSSFADE_DOWN;
	case SACT_EFFECT_CROSSFADE_UP:        return EFFECT_CROSSFADE_UP;
	case SACT_EFFECT_PENTAGRAM_IN_OUT:    return EFFECT_PENTAGRAM_IN_OUT;
	case SACT_EFFECT_PENTAGRAM_OUT_IN:    return EFFECT_PENTAGRAM_OUT_IN;
	case SACT_EFFECT_HEXAGRAM_IN_OUT:     return EFFECT_HEXAGRAM_IN_OUT;
	case SACT_EFFECT_HEXAGRAM_OUT_IN:     return EFFECT_HEXAGRAM_OUT_IN;
	case SACT_EFFECT_LINEAR_BLUR_VERT:    return EFFECT_LINEAR_BLUR_VERT;
	case SACT_EFFECT_ROTATE_OUT:          return EFFECT_ROTATE_OUT;
	case SACT_EFFECT_ROTATE_IN:           return EFFECT_ROTATE_IN;
	case SACT_EFFECT_ROTATE_OUT_CW:       return EFFECT_ROTATE_OUT_CW;
	case SACT_EFFECT_ROTATE_IN_CW:        return EFFECT_ROTATE_IN_CW;
	case SACT_EFFECT_POLYGON_ROTATE_Y:    return EFFECT_POLYGON_ROTATE_Y;
	case SACT_EFFECT_POLYGON_ROTATE_Y_CW: return EFFECT_POLYGON_ROTATE_Y_CW;
	case SACT_EFFECT_POLYGON_ROTATE_X:    return EFFECT_POLYGON_ROTATE_X;
	case SACT_EFFECT_POLYGON_ROTATE_X_CW: return EFFECT_POLYGON_ROTATE_X_CW;
	case SACT_EFFECT_ZIGZAG_CROSSFADE:    return EFFECT_ZIGZAG_CROSSFADE;
	default:                              return EFFECT_INVALID;
	}
}

typedef struct {
	SDL_Texture *tx;
	SDL_Rect rect;
} EffectTexture;

static EffectTexture *create_effect_texture(surface_t *as, int x, int y, int w, int h) {
	EffectTexture *t = calloc(1, sizeof(EffectTexture));
	if (!as) {
		t->tx = gfx_texture;
		t->rect = (SDL_Rect){ x, y, w, h };
	} else {
		SDL_Surface *sf = gfx_createSurfaceView(as->sdl_surface, x, y, w, h);
		t->tx = SDL_CreateTextureFromSurface(gfx_renderer, sf);
		SDL_FreeSurface(sf);
		t->rect = (SDL_Rect){ 0, 0, w, h };
	}
	return t;
}

static EffectTexture *create_effect_texture_from_surface(SDL_Surface *sf) {
	EffectTexture *t = calloc(1, sizeof(EffectTexture));
	t->tx = SDL_CreateTextureFromSurface(gfx_renderer, sf);
	t->rect = (SDL_Rect){ 0, 0, sf->w, sf->h };
	return t;
}

static void destroy_effect_texture(EffectTexture *t) {
	if (!t)
		return;
	if (t->tx && t->tx != gfx_texture)
		SDL_DestroyTexture(t->tx);
	free(t);
}

static int render_effect_texture(EffectTexture *t, const SDL_Rect* srcrect, const SDL_Rect* dstrect) {
	SDL_Rect real_srcrect;
	if (!srcrect) {
		real_srcrect = t->rect;
	} else {
		real_srcrect = *srcrect;
		real_srcrect.x += t->rect.x;
		real_srcrect.y += t->rect.y;
	}
	return SDL_RenderCopy(gfx_renderer, t->tx, &real_srcrect, dstrect);
}

struct effect {
	void (*step)(struct effect *this, float progress);
	void (*finish)(struct effect *this);
	enum effect_type type;
	SDL_Rect dst_rect;
	bool is_fullscreen;
	EffectTexture *tx_old, *tx_new;
};

static void eff_init(struct effect *eff, SDL_Rect *rect, EffectTexture *old, EffectTexture *new, enum effect_type type) {
	eff->type = type;
	eff->dst_rect = *rect;
	eff->is_fullscreen = rect->x == 0 && rect->y == 0
		&& rect->w == view_w && rect->h == view_h;
	eff->tx_old = old;
	eff->tx_new = new;

	if (!eff->is_fullscreen)
		gfx_updateScreen();  // Flush pending display changes.
}

static void eff_finish(struct effect *eff, bool present) {
	if (present) {
		SDL_RenderClear(gfx_renderer);
		if (!eff->is_fullscreen)
			SDL_RenderCopy(gfx_renderer, gfx_texture, NULL, NULL);
		render_effect_texture(eff->tx_new, NULL, &eff->dst_rect);
		SDL_RenderPresent(gfx_renderer);
	}
	destroy_effect_texture(eff->tx_old);
	destroy_effect_texture(eff->tx_new);
}

static inline void flip_rect_h(SDL_Rect *r, int w) {
	r->x = w - r->x - r->w;
}

static inline void flip_rect_v(SDL_Rect *r, int h) {
	r->y = h - r->y - r->h;
}

static void transpose_rect(SDL_Rect *r) {
	int tx = r->x;
	r->x = r->y;
	r->y = tx;
	int tw = r->w;
	r->w = r->h;
	r->h = tw;
}

static inline void move_rect(SDL_Rect *r, int off_x, int off_y) {
	r->x += off_x;
	r->y += off_y;
}

// EFFECT_CROSSFADE

static void crossfade_step(struct effect *eff, float progress);
static void crossfade_free(struct effect *eff);

static struct effect *crossfade_new(SDL_Rect *rect, EffectTexture *old, EffectTexture *new) {
	struct effect *eff = calloc(1, sizeof(struct effect));
	if (!eff)
		NOMEMERR();
	eff_init(eff, rect, old, new, EFFECT_CROSSFADE);
	eff->step = crossfade_step;
	eff->finish = crossfade_free;
	return eff;
}

static void crossfade_step(struct effect *eff, float progress) {
	render_effect_texture(eff->tx_old, NULL, &eff->dst_rect);
	SDL_SetTextureBlendMode(eff->tx_new->tx, SDL_BLENDMODE_BLEND);
	SDL_SetTextureAlphaMod(eff->tx_new->tx, progress * 255);
	render_effect_texture(eff->tx_new, NULL, &eff->dst_rect);
	SDL_SetTextureAlphaMod(eff->tx_new->tx, 255);
	SDL_SetTextureBlendMode(eff->tx_new->tx, SDL_BLENDMODE_NONE);
	SDL_RenderPresent(gfx_renderer);
}

static void crossfade_free(struct effect *eff) {
	eff_finish(eff, true);
	free(eff);
}

static struct effect *fallback_effect_new(SDL_Rect *rect, EffectTexture *old, EffectTexture *new, enum effect_type type) {
	WARNING("Effect %d is not supported in this system. Falling back to crossfade.", type);
	return crossfade_new(rect, old, new);
}

// EFFECT_CROSSFADE_{UP,DOWN,LR,RL}

static void crossfade_animation_step(struct effect *eff, float progress);
static void crossfade_animation_free(struct effect *eff);

static struct effect *crossfade_animation_new(SDL_Rect *rect, EffectTexture *old, EffectTexture *new, enum effect_type type) {
	struct effect *eff = calloc(1, sizeof(struct effect));
	if (!eff)
		NOMEMERR();
	eff_init(eff, rect, old, new, type);
	eff->step = crossfade_animation_step;
	eff->finish = crossfade_animation_free;
	return eff;
}

static void crossfade_animation_step_sub(SDL_Rect *rect, EffectTexture *tx, int tx_x, int tx_y, float progress, int band_width, bool lr, bool flip) {
	int maxstep = (lr ? rect->w : rect->h) + band_width;
	int band_top = maxstep * progress - band_width;
	if (band_top > 0) {
		SDL_Rect sr = {0, 0, rect->w, rect->h};
		*(lr ? &sr.w : &sr.h) = band_top;
		if (flip) {
			if (lr)
				flip_rect_h(&sr, rect->w);
			else
				flip_rect_v(&sr, rect->h);
		}
		SDL_Rect dr = sr;
		move_rect(&sr, tx_x, tx_y);
		move_rect(&dr, rect->x, rect->y);
		render_effect_texture(tx, &sr, &dr);
	}

	SDL_SetTextureBlendMode(tx->tx, SDL_BLENDMODE_BLEND);
	for (int i = 0; i < band_width; i++) {
		SDL_Rect sr;
		if (lr) {
			int x = band_top + i;
			if (x < 0 || x >= rect->w)
				continue;
			sr = (SDL_Rect){x, 0, 1, rect->h};
			if (flip)
				flip_rect_h(&sr, rect->w);
		} else {
			int y = band_top + i;
			if (y < 0 || y >= rect->h)
				continue;
			sr = (SDL_Rect){0, y, rect->w, 1};
			if (flip)
				flip_rect_v(&sr, rect->h);
		}
		SDL_Rect dr = sr;
		move_rect(&sr, tx_x, tx_y);
		move_rect(&dr, rect->x, rect->y);
		SDL_SetTextureAlphaMod(tx->tx, 255 - (i + 1) * (256 / band_width));
		render_effect_texture(tx, &sr, &dr);
	}
	SDL_SetTextureAlphaMod(tx->tx, 255);
	SDL_SetTextureBlendMode(tx->tx, SDL_BLENDMODE_NONE);
}

static void crossfade_animation_step(struct effect *eff, float progress) {
	render_effect_texture(eff->tx_old, NULL, &eff->dst_rect);

	switch (eff->type) {
	case EFFECT_CROSSFADE_DOWN:
		crossfade_animation_step_sub(&eff->dst_rect, eff->tx_new, 0, 0, progress, 254, false, false);
		break;
	case EFFECT_CROSSFADE_UP:
		crossfade_animation_step_sub(&eff->dst_rect, eff->tx_new, 0, 0, progress, 254, false, true);
		break;
	case EFFECT_CROSSFADE_LR:
		crossfade_animation_step_sub(&eff->dst_rect, eff->tx_new, 0, 0, progress, 254, true, false);
		break;
	case EFFECT_CROSSFADE_RL:
		crossfade_animation_step_sub(&eff->dst_rect, eff->tx_new, 0, 0, progress, 254, true, true);
		break;
	case EFFECT_CROSSFADE_UP_DOWN:
		{
			SDL_Rect dr = eff->dst_rect;
			int th = dr.h /= 2;
			dr.h = th;
			crossfade_animation_step_sub(&dr, eff->tx_new, 0, 0, progress, 127, false, true);
			dr.y += th;
			dr.h = eff->dst_rect.h - th;
			crossfade_animation_step_sub(&dr, eff->tx_new, 0, th, progress, 127, false, false);
		}
		break;
	case EFFECT_CROSSFADE_LR_RL:
		{
			SDL_Rect dr = eff->dst_rect;
			int lw = dr.w /= 2;
			dr.w = lw;
			crossfade_animation_step_sub(&dr, eff->tx_new, 0, 0, progress, 127, true, true);
			dr.x += lw;
			dr.w = eff->dst_rect.w - lw;
			crossfade_animation_step_sub(&dr, eff->tx_new, lw, 0, progress, 127, true, false);
		}
		break;
	default:
		assert(!"Cannot happen");
	}

	SDL_RenderPresent(gfx_renderer);
}

static void crossfade_animation_free(struct effect *eff) {
	eff_finish(eff, false);
	free(eff);
}

// EFFECT_MOSAIC, EFFECT_CROSSFADE_MOSAIC

struct mosaic_effect {
	struct effect eff;
	SDL_Texture *tmp_old, *tmp_new, *tx_mosaic;
};

static void mosaic_step(struct effect *eff, float progress);
static void mosaic_free(struct effect *eff);

static struct effect *mosaic_new(SDL_Rect *rect, EffectTexture *old, EffectTexture *new, enum effect_type type) {
	if (!SDL_RenderTargetSupported(gfx_renderer))
		return fallback_effect_new(rect, old, new, type);

	struct mosaic_effect *eff = calloc(1, sizeof(struct mosaic_effect));
	if (!eff)
		NOMEMERR();
	eff_init(&eff->eff, rect, old, new, type);
	eff->tmp_old = SDL_CreateTexture(gfx_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, rect->w, rect->h);
	eff->tmp_new = SDL_CreateTexture(gfx_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, rect->w, rect->h);
	eff->tx_mosaic = SDL_CreateTexture(gfx_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, rect->w, rect->h);
	eff->eff.step = mosaic_step;
	eff->eff.finish = mosaic_free;
	return &eff->eff;
}

static void mosaic(EffectTexture *src, SDL_Texture *tmp, SDL_Texture *dst, int w, int h, float scale) {
	int cx = w / 2;
	int cy = h / 2;
	SDL_FRect fr = {
		.x = cx - cx / scale,
		.y = cy - cy / scale,
		.w = w / scale,
		.h = h / scale,
	};
	SDL_SetRenderTarget(gfx_renderer, tmp);
	SDL_RenderCopyF(gfx_renderer, src->tx, &src->rect, &fr);

	SDL_Rect dstr = {
		.x = w * (1 - scale) / 2,
		.y = h * (1 - scale) / 2,
		.w = w * scale,
		.h = h * scale,
	};
	SDL_SetRenderTarget(gfx_renderer, dst);

#if SDL_VERSION_ATLEAST(2, 0, 12)
	SDL_SetTextureScaleMode(tmp, SDL_ScaleModeNearest);
#else
	// Should be okay because nearest is the default scaling mode.
#endif
	SDL_RenderCopy(gfx_renderer, tmp, NULL, &dstr);
	SDL_SetRenderTarget(gfx_renderer, NULL);
}

static void mosaic_step(struct effect *eff, float progress) {
	struct mosaic_effect *this = (struct mosaic_effect *)eff;

	if (eff->type == EFFECT_MOSAIC) {
		const int max_scale = 80;
		int scale = (1.f - progress) * (max_scale - 1) + 1;
		mosaic(eff->tx_new, this->tmp_new, this->tx_mosaic, eff->dst_rect.w, eff->dst_rect.h, scale);
		SDL_RenderCopy(gfx_renderer, this->tx_mosaic, NULL, &eff->dst_rect);
	} else if (eff->type == EFFECT_CROSSFADE_MOSAIC) {
		const int max_scale = 96;
		float scale = (progress < 0.5f ? progress : 1.f - progress) * 2 * (max_scale - 1) + 1;
		mosaic(eff->tx_old, this->tmp_old, this->tx_mosaic, eff->dst_rect.w, eff->dst_rect.h, scale);
		SDL_RenderCopy(gfx_renderer, this->tx_mosaic, NULL, &eff->dst_rect);
		mosaic(eff->tx_new, this->tmp_new, this->tx_mosaic, eff->dst_rect.w, eff->dst_rect.h, scale);
		SDL_SetTextureBlendMode(this->tx_mosaic, SDL_BLENDMODE_BLEND);
		SDL_SetTextureAlphaMod(this->tx_mosaic, progress * 255);
		SDL_RenderCopy(gfx_renderer, this->tx_mosaic, NULL, &eff->dst_rect);
		SDL_SetTextureBlendMode(this->tx_mosaic, SDL_BLENDMODE_NONE);
	} else {
		assert(!"Cannot happen");
	}

	SDL_RenderPresent(gfx_renderer);
}

static void mosaic_free(struct effect *eff) {
	struct mosaic_effect *this = (struct mosaic_effect *)eff;
	SDL_DestroyTexture(this->tmp_old);
	SDL_DestroyTexture(this->tmp_new);
	SDL_DestroyTexture(this->tx_mosaic);
	eff_finish(&this->eff, true);
	free(this);
}

// EFFECT_{FADE,WHITE}{OUT,IN}

static void brightness_step(struct effect *eff, float progress);
static void brightness_free(struct effect *eff);

static struct effect *brightness_new(SDL_Rect *rect, EffectTexture *old, EffectTexture *new, enum effect_type type) {
	struct effect *eff = calloc(1, sizeof(struct effect));
	if (!eff)
		NOMEMERR();
	eff_init(eff, rect, old, new, type);
	eff->step = brightness_step;
	eff->finish = brightness_free;
	return eff;
}

static void brightness_step(struct effect *eff, float progress) {
	EffectTexture *texture;
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
		alpha = (1.f - progress) * 255;
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
		alpha = (1.f - progress) * 255;
		break;
	default:
		assert(!"Cannot happen");
	}
	render_effect_texture(texture, NULL, &eff->dst_rect);
	SDL_SetRenderDrawBlendMode(gfx_renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(gfx_renderer, color, color, color, alpha);
	SDL_RenderFillRect(gfx_renderer, &eff->dst_rect);
	SDL_SetRenderDrawBlendMode(gfx_renderer, SDL_BLENDMODE_NONE);
	SDL_SetRenderDrawColor(gfx_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	SDL_RenderPresent(gfx_renderer);
}

static void brightness_free(struct effect *eff) {
	eff_finish(eff, false);
	free(eff);
}

// EFFECT_DITHERING_{FADE,WHITE}{OUT,IN}

static void dithering_fade_step(struct effect *eff, float progress);
static void dithering_fade_free(struct effect *eff);

static EffectTexture *create_dither_pattern_texture(int w, int h, int val) {
	SDL_PixelFormat *fmt = SDL_AllocFormat(SDL_PIXELFORMAT_ARGB8888);
	Uint32 col = SDL_MapRGBA(fmt, val, val, val, SDL_ALPHA_OPAQUE);
	SDL_FreeFormat(fmt);

	Uint32 *pixels = calloc(w * h, sizeof(Uint32));
	if (!pixels)
		NOMEMERR();
	for (int y = 0; y < h; y += 4) {
		Uint32 *row = pixels + y * w;
		for (int x = 0; x < w; x += 4)
			row[x] = col;
	}

	EffectTexture *t = calloc(1, sizeof(EffectTexture));
	t->tx = SDL_CreateTexture(gfx_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, w, h);
	SDL_UpdateTexture(t->tx, NULL, pixels, w * 4);
	free(pixels);
	SDL_SetTextureBlendMode(t->tx, SDL_BLENDMODE_BLEND);
	t->rect = (SDL_Rect){ 0, 0, w, h };
	return t;
}

static struct effect *dithering_fade_new(SDL_Rect *rect, EffectTexture *old, EffectTexture *new, enum effect_type type) {
	struct effect *eff = calloc(1, sizeof(struct effect));
	if (!eff)
		NOMEMERR();
	switch (type) {
	case EFFECT_DITHERING_FADEOUT:
		destroy_effect_texture(new);
		eff_init(eff, rect, old, NULL, type);
		eff->tx_new = create_dither_pattern_texture(rect->w, rect->h, 0);
		break;
	case EFFECT_DITHERING_FADEIN:
		destroy_effect_texture(old);
		eff_init(eff, rect, new, NULL, type);
		eff->tx_new = create_dither_pattern_texture(rect->w, rect->h, 0);
		break;
	case EFFECT_DITHERING_WHITEOUT:
		destroy_effect_texture(new);
		eff_init(eff, rect, old, NULL, type);
		eff->tx_new = create_dither_pattern_texture(rect->w, rect->h, 255);
		break;
	case EFFECT_DITHERING_WHITEIN:
		destroy_effect_texture(old);
		eff_init(eff, rect, new, NULL, type);
		eff->tx_new = create_dither_pattern_texture(rect->w, rect->h, 255);
		break;
	default:
		assert(!"Cannot happen");
	}
	eff->step = dithering_fade_step;
	eff->finish = dithering_fade_free;
	return eff;
}

static void dithering_fade_step(struct effect *eff, float progress) {
	static const int dither_x[16] = {0,2,2,0,1,3,3,1,1,3,3,1,0,2,2,0};
	static const int dither_y[16] = {0,2,0,2,1,3,1,3,0,2,0,2,1,3,1,3};

	render_effect_texture(eff->tx_old, NULL, &eff->dst_rect);

	if (eff->type == EFFECT_DITHERING_FADEIN || eff->type == EFFECT_DITHERING_WHITEIN)
		progress = 1.f - progress;
	int level = roundf(progress * 16);

	assert(eff->is_fullscreen);
	SDL_Rect dr = eff->dst_rect;
	for (int i = 0; i < level; i++) {
		dr.x = eff->dst_rect.x + dither_x[i];
		dr.y = eff->dst_rect.y + dither_y[i];
		render_effect_texture(eff->tx_new, NULL, &dr);
	}

	SDL_RenderPresent(gfx_renderer);
}

static void dithering_fade_free(struct effect *eff) {
	if (eff->type == EFFECT_DITHERING_FADEOUT) {
		SDL_SetRenderDrawColor(gfx_renderer, 0, 0, 0, 255);
		SDL_RenderFillRect(gfx_renderer, &eff->dst_rect);
	} else if (eff->type == EFFECT_DITHERING_WHITEOUT) {
		SDL_SetRenderDrawColor(gfx_renderer, 255, 255, 255, 255);
		SDL_RenderFillRect(gfx_renderer, &eff->dst_rect);
	}
	eff_finish(eff, false);
	free(eff);
}

// EFFECT_PAN_IN_*

static void pan_in_step(struct effect *eff, float progress);
static void pan_in_free(struct effect *eff);

static struct effect *pan_in_new(SDL_Rect *rect, EffectTexture *old, EffectTexture *new, enum effect_type type) {
	struct effect *eff = calloc(1, sizeof(struct effect));
	if (!eff)
		NOMEMERR();
	eff_init(eff, rect, old, new, type);
	eff->step = pan_in_step;
	eff->finish = pan_in_free;
	return eff;
}

static void pan_in_step(struct effect *eff, float progress) {
	render_effect_texture(eff->tx_old, NULL, &eff->dst_rect);

	int h = eff->dst_rect.h * progress;
	SDL_Rect sr = { 0, 0, eff->dst_rect.w, h };
	SDL_Rect dr = eff->dst_rect;
	dr.h = h;
	if (eff->type == EFFECT_PAN_IN_DOWN) {
		sr.y = eff->dst_rect.h - h;
	} else {
		dr.y += eff->dst_rect.h - h;
	}
	render_effect_texture(eff->tx_new, &sr, &dr);

	SDL_RenderPresent(gfx_renderer);
}

static void pan_in_free(struct effect *eff) {
	eff_finish(eff, true);
	free(eff);
}

// EFFECT_SKIP_LINE_*

static void skip_line_step(struct effect *eff, float progress);
static void skip_line_free(struct effect *eff);

static struct effect *skip_line_new(SDL_Rect *rect, EffectTexture *old, EffectTexture *new, enum effect_type type) {
	struct effect *eff = calloc(1, sizeof(struct effect));
	if (!eff)
		NOMEMERR();
	eff_init(eff, rect, old, new, type);
	eff->step = skip_line_step;
	eff->finish = skip_line_free;
	return eff;
}

static void skip_line_step(struct effect *eff, float progress) {
	render_effect_texture(eff->tx_old, NULL, &eff->dst_rect);

	if (eff->type == EFFECT_SKIP_LINE_UP_DOWN) {
		int h = eff->dst_rect.h * progress;
		SDL_Rect sr = { 0, 0, eff->dst_rect.w, 1 };
		SDL_Rect dr = eff->dst_rect;
		dr.h = 1;

		int bottom = (eff->dst_rect.h - 1) | 1;
		for (int y = 0; y < h; y += 2) {
			sr.y = y;
			dr.y = eff->dst_rect.y + y;
			render_effect_texture(eff->tx_new, &sr, &dr);

			if (y == 0 && eff->dst_rect.h & 1)
				continue;
			sr.y = bottom - y;
			dr.y = eff->dst_rect.y + bottom - y;
			render_effect_texture(eff->tx_new, &sr, &dr);
		}
	} else {
		int w = eff->dst_rect.w * progress;
		SDL_Rect sr = { 0, 0, 1, eff->dst_rect.h };
		SDL_Rect dr = eff->dst_rect;
		dr.w = 1;

		int right = (eff->dst_rect.w - 1) | 1;
		for (int x = 0; x < w; x += 2) {
			sr.x = x;
			dr.x = eff->dst_rect.x + x;
			render_effect_texture(eff->tx_new, &sr, &dr);

			if (x == 0 && eff->dst_rect.w & 1)
				continue;
			sr.x = right - x;
			dr.x = eff->dst_rect.x + right - x;
			render_effect_texture(eff->tx_new, &sr, &dr);
		}
	}

	SDL_RenderPresent(gfx_renderer);
}

static void skip_line_free(struct effect *eff) {
	eff_finish(eff, true);
	free(eff);
}

// EFFECT_WIPE_*

static void wipe_step(struct effect *eff, float progress);
static void wipe_free(struct effect *eff);

static struct effect *wipe_new(SDL_Rect *rect, EffectTexture *old, EffectTexture *new, enum effect_type type) {
	struct effect *eff = calloc(1, sizeof(struct effect));
	if (!eff)
		NOMEMERR();
	eff_init(eff, rect, old, new, type);
	eff->step = wipe_step;
	eff->finish = wipe_free;
	return eff;
}

static void wipe_step(struct effect *eff, float progress) {
	EffectTexture *bg = eff->tx_old;
	EffectTexture *fg = eff->tx_new;
	if (eff->type == EFFECT_WIPE_RL ||
		eff->type == EFFECT_WIPE_UP ||
		eff->type == EFFECT_WIPE_IN ||
		eff->type == EFFECT_WIPE_IN_V ||
		eff->type == EFFECT_WIPE_IN_H) {
		bg = eff->tx_new;
		fg = eff->tx_old;
		progress = 1.f - progress;
	}

	render_effect_texture(bg, NULL, &eff->dst_rect);

	SDL_Rect sr = { 0, 0, eff->dst_rect.w, eff->dst_rect.h };
	SDL_Rect dr = eff->dst_rect;
	switch (eff->type) {
	case EFFECT_WIPE_LR:
	case EFFECT_WIPE_RL:
		sr.w *= progress;
		dr.w = sr.w;
		break;
	case EFFECT_WIPE_DOWN:
	case EFFECT_WIPE_UP:
		sr.h *= progress;
		dr.h = sr.h;
		break;
	case EFFECT_WIPE_OUT:
	case EFFECT_WIPE_IN:
	case EFFECT_WIPE_OUT_V:
	case EFFECT_WIPE_IN_V:
		sr.w *= progress;
		sr.x = (dr.w - sr.w) / 2;
		dr.w = sr.w;
		dr.x += sr.x;
		if (eff->type == EFFECT_WIPE_OUT_V || eff->type == EFFECT_WIPE_IN_V)
			break;
		// fallthrough
	case EFFECT_WIPE_OUT_H:
	case EFFECT_WIPE_IN_H:
		sr.h *= progress;
		sr.y = (dr.h - sr.h) / 2;
		dr.h = sr.h;
		dr.y += sr.y;
		break;
	default:
		assert(!"Cannot happen");
	}
	render_effect_texture(fg, &sr, &dr);

	SDL_RenderPresent(gfx_renderer);
}

static void wipe_free(struct effect *eff) {
	eff_finish(eff, true);
	free(eff);
}

// EFFECT_CIRCLE_WIPE_*

static void circle_wipe_step(struct effect *eff, float progress);
static void circle_wipe_free(struct effect *eff);

static struct effect *circle_wipe_new(SDL_Rect *rect, EffectTexture *old, EffectTexture *new, enum effect_type type) {
	struct effect *eff = calloc(1, sizeof(struct effect));
	if (!eff)
		NOMEMERR();
	eff_init(eff, rect, old, new, type);
	eff->step = circle_wipe_step;
	eff->finish = circle_wipe_free;
	return eff;
}

static void circle_wipe_step(struct effect *eff, float progress) {
	EffectTexture *bg = eff->tx_old;
	EffectTexture *fg = eff->tx_new;
	if (eff->type == EFFECT_CIRCLE_WIPE_IN) {
		bg = eff->tx_new;
		fg = eff->tx_old;
		progress = 1 - progress;
	}
	render_effect_texture(bg, NULL, &eff->dst_rect);

	int hw = eff->dst_rect.w / 2;
	int hh = eff->dst_rect.h / 2;
	int r = sqrtf(hw*hw + hh*hh) * progress;

	for (int y = -r; y < r; y++) {
		if (y < -hh || y >= hh)
			continue;
		for (int x = max(-r, -hw); x < 0; x++) {
			if (y*y + x*x <= r*r) {
				SDL_Rect sr = {hw + x, hh + y, -2*x, 1};
				SDL_Rect dr = {eff->dst_rect.x + hw + x, eff->dst_rect.y + hh + y, -2*x, 1};
				render_effect_texture(fg, &sr, &dr);
				break;
			}
		}
	}

	SDL_RenderPresent(gfx_renderer);
}

static void circle_wipe_free(struct effect *eff) {
	eff_finish(eff, true);
	free(eff);
}

// EFFECT_BLIND_*

static void blind_step(struct effect *eff, float progress);
static void blind_free(struct effect *eff);

static struct effect *blind_new(SDL_Rect *rect, EffectTexture *old, EffectTexture *new, enum effect_type type) {
	struct effect *eff = calloc(1, sizeof(struct effect));
	if (!eff)
		NOMEMERR();
	eff_init(eff, rect, old, new, type);
	eff->step = blind_step;
	eff->finish = blind_free;
	return eff;
}

static void blind_vstep(float progress, SDL_Rect *rect, EffectTexture *tx_new, int offset_y, bool flip) {
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
		render_effect_texture(tx_new, &sr, &dr);
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
		render_effect_texture(tx_new, &sr, &dr);
	}
}

static void blind_step(struct effect *eff, float progress) {
	const int N = 16;

	render_effect_texture(eff->tx_old, NULL, &eff->dst_rect);

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
			render_effect_texture(eff->tx_new, &sr, &dr);
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
			render_effect_texture(eff->tx_new, &sr, &dr);
		}
	}

	SDL_RenderPresent(gfx_renderer);
}

static void blind_free(struct effect *eff) {
	eff_finish(eff, false);
	free(eff);
}

// EFFECT_BLEND_*

static void blend_animation_step(struct effect *eff, float progress);
static void blend_animation_free(struct effect *eff);

static struct effect *blend_animation_new(SDL_Rect *rect, EffectTexture *old, EffectTexture *new, enum effect_type type) {
	struct effect *eff = calloc(1, sizeof(struct effect));
	if (!eff)
		NOMEMERR();
	eff_init(eff, rect, old, new, type);
	eff->step = blend_animation_step;
	eff->finish = blend_animation_free;
	return eff;
}

static void blend_animation_step(struct effect *eff, float progress) {
	const bool lr = eff->type == EFFECT_BLEND_LR_RL;

	EffectTexture *bg, *fg;
	if (progress < 0.5f) {
		bg = eff->tx_old;
		fg = eff->tx_new;
	} else {
		bg = eff->tx_new;
		fg = eff->tx_old;
		progress = 1.f - progress;
	}

	render_effect_texture(bg, NULL, &eff->dst_rect);
	int k = (lr ? eff->dst_rect.w : eff->dst_rect.h) * progress;
	SDL_SetTextureBlendMode(fg->tx, SDL_BLENDMODE_BLEND);
	SDL_SetTextureAlphaMod(fg->tx, 127);

	SDL_Rect sr = {0, 0, eff->dst_rect.w, eff->dst_rect.h};
	SDL_Rect dr = eff->dst_rect;
	if (lr)
		sr.w = dr.w = k;
	else
		sr.h = dr.h = k;
	render_effect_texture(fg, &sr, &dr);

	if (lr) {
		sr.x = eff->dst_rect.w - k;
		dr.x += sr.x;
	} else {
		sr.y = eff->dst_rect.h - k;
		dr.y += sr.y;
	}
	render_effect_texture(fg, &sr, &dr);

	SDL_SetTextureAlphaMod(fg->tx, 255);
	SDL_SetTextureBlendMode(fg->tx, SDL_BLENDMODE_NONE);
	SDL_RenderPresent(gfx_renderer);
}

static void blend_animation_free(struct effect *eff) {
	eff_finish(eff, false);
	free(eff);
}

// EFFECT_ZOOM_BLEND_BLUR

#define ZOOM_BLEND_BLUR_STEPS 6

struct zoom_blend_blur_effect {
	struct effect eff;
	SDL_Texture *tx[ZOOM_BLEND_BLUR_STEPS];
	int index;
};

static void zoom_blend_blur_step(struct effect *eff, float progress);
static void zoom_blend_blur_free(struct effect *eff);

static struct effect *zoom_blend_blur_new(SDL_Rect *rect, EffectTexture *old, EffectTexture *new) {
	if (!SDL_RenderTargetSupported(gfx_renderer))
		return fallback_effect_new(rect, old, new, EFFECT_ZOOM_BLEND_BLUR);

	struct zoom_blend_blur_effect *eff = calloc(1, sizeof(struct zoom_blend_blur_effect));
	if (!eff)
		NOMEMERR();
	eff_init(&eff->eff, rect, old, new, EFFECT_ZOOM_BLEND_BLUR);
	eff->eff.step = zoom_blend_blur_step;
	eff->eff.finish = zoom_blend_blur_free;
	return &eff->eff;
}

static void zoom_blend_blur_step(struct effect *eff, float progress) {
	struct zoom_blend_blur_effect *this = (struct zoom_blend_blur_effect *)eff;

	if (!this->tx[this->index]) {
		this->tx[this->index] = SDL_CreateTexture(gfx_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, eff->dst_rect.w, eff->dst_rect.h);
	}

	float scale = (progress < 0.5f ? 1 - progress * 2 : (progress - 0.5f) * 2) * 0.9f + 0.1f;  // 0.1 - 1.0
	int w = eff->dst_rect.w * scale;
	int h = eff->dst_rect.h * scale;
	SDL_Rect r = {
		.x = (eff->dst_rect.w - w) / 2,
		.y = (eff->dst_rect.h - h) / 2,
		.w = w,
		.h = h
	};
	SDL_SetRenderTarget(gfx_renderer, this->tx[this->index]);
	render_effect_texture(eff->tx_new, &r, NULL);
	SDL_SetRenderTarget(gfx_renderer, NULL);

	this->index = (this->index + 1) % ZOOM_BLEND_BLUR_STEPS;

	SDL_RenderFillRect(gfx_renderer, &eff->dst_rect);
	EffectTexture texture_rect = { NULL, { 0, 0, eff->dst_rect.w, eff->dst_rect.h } };
	for (int i = 0; i < ZOOM_BLEND_BLUR_STEPS; i++) {
		texture_rect.tx = this->tx[i];
		EffectTexture *t = this->tx[i] ? &texture_rect : eff->tx_new;
		SDL_SetTextureBlendMode(t->tx, SDL_BLENDMODE_ADD);
		SDL_SetTextureAlphaMod(t->tx, 255 / ZOOM_BLEND_BLUR_STEPS);
		render_effect_texture(t, NULL, &eff->dst_rect);
		SDL_SetTextureBlendMode(t->tx, SDL_BLENDMODE_NONE);
		SDL_SetTextureAlphaMod(t->tx, 255);
	}
	SDL_RenderPresent(gfx_renderer);
}

static void zoom_blend_blur_free(struct effect *eff) {
	struct zoom_blend_blur_effect *this = (struct zoom_blend_blur_effect *)eff;
	for (int i = 0; i < ZOOM_BLEND_BLUR_STEPS; i++) {
		if (this->tx[i])
			SDL_DestroyTexture(this->tx[i]);
	}
	eff_finish(&this->eff, true);
	free(this);
}

// EFFECT_LINEAR_BLUR, EFFECT_LINEAR_BLUR_VERT

#define LINEAR_BLUR_STEPS 10

struct linear_blur_effect {
	struct effect eff;
	EffectTexture *blurred_old[LINEAR_BLUR_STEPS];
	EffectTexture *blurred_new[LINEAR_BLUR_STEPS];
};

static void linear_blur_step(struct effect *eff, float progress);
static void linear_blur_free(struct effect *eff);

static struct effect *linear_blur_new(SDL_Rect *rect, EffectTexture *old, EffectTexture *new, enum effect_type type) {
	if (!SDL_RenderTargetSupported(gfx_renderer))
		return fallback_effect_new(rect, old, new, type);

	struct linear_blur_effect *lbe = calloc(1, sizeof(struct linear_blur_effect));
	if (!lbe)
		NOMEMERR();
	eff_init(&lbe->eff, rect, old, new, type);
	lbe->eff.step = linear_blur_step;
	lbe->eff.finish = linear_blur_free;
	return &lbe->eff;
}

static EffectTexture *blur(struct linear_blur_effect *this, EffectTexture *src, int stride) {
	bool vertical = this->eff.type == EFFECT_LINEAR_BLUR_VERT;
	int w = this->eff.dst_rect.w;
	int h = this->eff.dst_rect.h;
	EffectTexture *dst = calloc(1, sizeof(EffectTexture));
	dst->tx = SDL_CreateTexture(gfx_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, w, h);
	dst->rect = (SDL_Rect){0, 0, w, h};

	SDL_SetRenderTarget(gfx_renderer, dst->tx);
	SDL_RenderClear(gfx_renderer);
	SDL_SetTextureBlendMode(src->tx, SDL_BLENDMODE_ADD);
	SDL_SetTextureAlphaMod(src->tx, 127);

	SDL_Rect dr = {0, 0, w, h};
	*(vertical ? &dr. y : &dr.x) = -stride / 2;
	render_effect_texture(src, NULL, &dr);
	*(vertical ? &dr. y : &dr.x) = max(1, stride / 2);
	render_effect_texture(src, NULL, &dr);

	// Copy the edge pixels to emulate texture clamping.
	SDL_Rect sr = {0, 0, vertical ? w : 1, vertical ? 1 : h};
	*(vertical ? &dr.h : &dr.w) = max(1, stride / 2);
	*(vertical ? &dr.y : &dr.x) = 0;
	render_effect_texture(src, &sr, &dr);
	if (stride > 1) {
		if (vertical) {
			sr.y = h - 1;
			dr.y = h - stride / 2;
		} else {
			sr.x = w - 1;
			dr.x = w - stride / 2;
		}
		render_effect_texture(src, &sr, &dr);
	}

	SDL_SetTextureAlphaMod(src->tx, 255);
	SDL_SetTextureBlendMode(src->tx, SDL_BLENDMODE_NONE);
	SDL_SetRenderTarget(gfx_renderer, NULL);

	return dst;
}

static void linear_blur_step(struct effect *eff, float progress) {
	struct linear_blur_effect *this = (struct linear_blur_effect *)eff;

	int step = progress * LINEAR_BLUR_STEPS * 2;
	if (step == LINEAR_BLUR_STEPS * 2) {
		render_effect_texture(eff->tx_new, NULL, NULL);
		SDL_RenderPresent(gfx_renderer);
		return;
	}

	for (int i = 0; i <= step && i < LINEAR_BLUR_STEPS; i++) {
		if (this->blurred_old[i])
			continue;
		this->blurred_old[i] = blur(this, i ? this->blurred_old[i - 1] : eff->tx_old, 1 << i);
		this->blurred_new[i] = blur(this, i ? this->blurred_new[i - 1] : eff->tx_new, 1 << i);
	}

	int i = step < LINEAR_BLUR_STEPS ? step : LINEAR_BLUR_STEPS * 2 - 1 - step;
	EffectTexture *old = this->blurred_old[i];
	EffectTexture *new = this->blurred_new[i];
	render_effect_texture(old, NULL, &eff->dst_rect);
	SDL_SetTextureBlendMode(new->tx, SDL_BLENDMODE_BLEND);
	SDL_SetTextureAlphaMod(new->tx, 255 * progress);
	render_effect_texture(new, NULL, &eff->dst_rect);
	SDL_RenderPresent(gfx_renderer);
}

static void linear_blur_free(struct effect *eff) {
	struct linear_blur_effect *this = (struct linear_blur_effect *)eff;
	for (int i = 0; i < LINEAR_BLUR_STEPS; i++) {
		if (this->blurred_old[i])
			destroy_effect_texture(this->blurred_old[i]);
		if (this->blurred_new[i])
			destroy_effect_texture(this->blurred_new[i]);
	}
	eff_finish(&this->eff, true);
	free(this);
}

// EFFECT_PENTAGRAM_*, EFFECT_HEXAGRAM_*, EFFECT_WINDMILL*

struct polygon_mask_effect {
	struct effect f;
	SDL_Texture *tx_tmp;
};

static void polygon_mask_step(struct effect *eff, float progress);
static void polygon_mask_free(struct effect *eff);

static struct effect *polygon_mask_new(SDL_Rect *rect, EffectTexture *old, EffectTexture *new, enum effect_type type) {
#if HAS_SDL_RenderGeometry
	if (SDL_RenderTargetSupported(gfx_renderer)) {
		struct polygon_mask_effect *pmf = calloc(1, sizeof(struct polygon_mask_effect));
		if (!pmf)
			NOMEMERR();
		eff_init(&pmf->f, rect, old, new, type);
		pmf->tx_tmp = SDL_CreateTexture(gfx_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, rect->w, rect->h);
		pmf->f.step = polygon_mask_step;
		pmf->f.finish = polygon_mask_free;
		return &pmf->f;
	}
#endif // HAS_SDL_RenderGeometry

	return fallback_effect_new(rect, old, new, type);
}

#if HAS_SDL_RenderGeometry

static void draw_pentagram(int center_x, int center_y, float radius, float rotate) {
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
	float sin_r = sinf(rotate);
	float cos_r = cosf(rotate);
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
	SDL_RenderGeometry(gfx_renderer, NULL, v, 10, indices, 15);
}

static void draw_hexagram(int center_x, int center_y, float radius, float rotate) {
	const SDL_FPoint p[6] = {
		{ 0,     -1.0},
		{-0.866,  0.5},
		{ 0.866,  0.5},

		{-0.866, -0.5},
		{ 0,      1.0},
		{ 0.866, -0.5}
	};
	SDL_Vertex v[6];
	float sin_r = sinf(rotate);
	float cos_r = cosf(rotate);
	for (int i = 0; i < 6; i++) {
		v[i].position.x = radius * (p[i].x * cos_r - p[i].y * sin_r) + center_x;
		v[i].position.y = radius * (p[i].x * sin_r + p[i].y * cos_r) + center_y;
		v[i].color = (SDL_Color){0, 0, 0, 0};
	}
	SDL_RenderGeometry(gfx_renderer, NULL, v, 6, NULL, 0);
}

static void draw_windmill_90(int w, int h, float theta) {
	float cx = w / 2.f;
	float cy = h / 2.f;
	float r = max(cx, cy) * sqrtf(2);
	float r_sin = r * sinf(theta);
	float r_cos = r * cosf(theta);

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

	if (theta <= M_PIf / 4) {
		const int indices[12] = {
			0, 1, 3,
			0, 4, 6,
			0, 7, 9,
			0, 10, 12
		};
		SDL_RenderGeometry(gfx_renderer, NULL, v, 13, indices, 12);
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
		SDL_RenderGeometry(gfx_renderer, NULL, v, 13, indices, 24);
	}
}

static void draw_windmill_180(int w, int h, float theta) {
	float cx = w / 2.f;
	float cy = h / 2.f;
	float r = max(cx, cy) * sqrtf(2);
	float r_sin = r * sinf(theta);
	float r_cos = r * cosf(theta);

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
		{.position = {cx + r_cos, cy + r_sin}},  // theta+M_PIf
	};

	int indices[24];
	int i = 0;

	indices[i++] = 0;  // center
	indices[i++] = 9;  // left
	for (int j = 1; theta > M_PIf / 4 * j; j++) {
		indices[i++] = j;
		indices[i++] = 0;  // center
		indices[i++] = j;
	}
	indices[i++] = 5;  // theta

	indices[i++] = 0;  // center
	indices[i++] = 4;  // right
	for (int j = 1; theta > M_PIf / 4 * j; j++) {
		indices[i++] = j + 5;
		indices[i++] = 0;  // center
		indices[i++] = j + 5;
	}
	indices[i++] = 10;  // theta+M_PIf

	SDL_RenderGeometry(gfx_renderer, NULL, v, 11, indices, i);
}

static void draw_windmill_360(int w, int h, float theta) {
	float cx = w / 2.f;
	float cy = h / 2.f;
	float r = max(cx, cy) * sqrtf(2);
	float r_sin = r * sinf(theta);
	float r_cos = r * cosf(theta);

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
	for (int j = 1; theta > M_PIf / 4 * j; j++) {
		indices[i++] = j;
		indices[i++] = 0;  // center
		indices[i++] = j;
	}
	indices[i++] = 9;  // theta
	SDL_RenderGeometry(gfx_renderer, NULL, v, 10, indices, i);
}

static void polygon_mask_step(struct effect *eff, float progress) {
	struct polygon_mask_effect *this = (struct polygon_mask_effect *)eff;
	int w = eff->dst_rect.w;
	int h = eff->dst_rect.h;
	SDL_SetRenderTarget(gfx_renderer, this->tx_tmp);
	render_effect_texture(eff->tx_old, NULL, NULL);
	switch (eff->type) {
	case EFFECT_PENTAGRAM_IN_OUT:
		draw_pentagram(w / 2, h / 2, max(w, h) * progress, M_PIf * progress);
		break;
	case EFFECT_PENTAGRAM_OUT_IN:
		draw_pentagram(w / 2, h / 2, max(w, h) * (1 - progress), M_PIf * progress);
		break;
	case EFFECT_HEXAGRAM_IN_OUT:
		draw_hexagram(w / 2, h / 2, max(w, h) * progress, M_PIf * progress);
		break;
	case EFFECT_HEXAGRAM_OUT_IN:
		draw_hexagram(w / 2, h / 2, max(w, h) * (1 - progress), M_PIf * progress);
		break;
	case EFFECT_WINDMILL:
		draw_windmill_90(w, h, M_PIf / 2 * progress);
		break;
	case EFFECT_WINDMILL_180:
		draw_windmill_180(w, h, M_PIf * progress);
		break;
	case EFFECT_WINDMILL_360:
		draw_windmill_360(w, h, M_PIf * 2 * progress);
		break;
	default:
		assert(!"Cannot happen");
	}
	SDL_SetRenderTarget(gfx_renderer, NULL);
	render_effect_texture(eff->tx_new, NULL, &eff->dst_rect);
	SDL_SetTextureBlendMode(this->tx_tmp, SDL_BLENDMODE_BLEND);
	SDL_RenderCopy(gfx_renderer, this->tx_tmp, NULL, &eff->dst_rect);
	SDL_RenderPresent(gfx_renderer);
}

static void polygon_mask_free(struct effect *eff) {
	struct polygon_mask_effect *this = (struct polygon_mask_effect *)eff;
	SDL_DestroyTexture(this->tx_tmp);
	eff_finish(&this->f, true);
	free(this);
}

#endif // HAS_SDL_RenderGeometry

// EFFECT_ROTATE_*

static void rotate_step(struct effect *eff, float progress);
static void rotate_free(struct effect *eff);

static struct effect *rotate_new(SDL_Rect *rect, EffectTexture *old, EffectTexture *new, enum effect_type type) {
	struct effect *eff = calloc(1, sizeof(struct effect));
	if (!eff)
		NOMEMERR();
	eff_init(eff, rect, old, new, type);
	eff->step = rotate_step;
	eff->finish = rotate_free;
	return eff;
}

static void rotate_step(struct effect *eff, float progress) {
	EffectTexture *bg_texture, *fg_texture;
	float angle = progress * 360;
	float scale = progress;
	switch (eff->type) {
	case EFFECT_ROTATE_OUT:
		scale = 1.f - scale;
		bg_texture = eff->tx_new;
		fg_texture = eff->tx_old;
		angle *= -1.f;
		break;
	case EFFECT_ROTATE_IN:
		bg_texture = eff->tx_old;
		fg_texture = eff->tx_new;
		angle *= -1.f;
		break;
	case EFFECT_ROTATE_OUT_CW:
		scale = 1.f - scale;
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
	r.x += (1.f - scale) * r.w / 2;
	r.y += (1.f - scale) * r.h / 2;
	r.w *= scale;
	r.h *= scale;
	render_effect_texture(bg_texture, NULL, &eff->dst_rect);
	SDL_RenderCopyEx(gfx_renderer, fg_texture->tx, &fg_texture->rect, &r, angle, NULL, SDL_FLIP_NONE);
	SDL_RenderPresent(gfx_renderer);
}

static void rotate_free(struct effect *eff) {
	eff_finish(eff, true);
	free(eff);
}

// EFFECT_POLYGON_ROTATE_*

static void polygon_rotate_step(struct effect *eff, float progress);
static void polygon_rotate_free(struct effect *eff);

static struct effect *polygon_rotate_new(SDL_Rect *rect, EffectTexture *old, EffectTexture *new, enum effect_type type) {
	struct effect *eff = calloc(1, sizeof(struct effect));
	if (!eff)
		NOMEMERR();
	eff_init(eff, rect, old, new, type);
	eff->step = polygon_rotate_step;
	eff->finish = polygon_rotate_free;
	return eff;
}

static void polygon_rotate_step(struct effect *eff, float progress) {
	bool vertical = eff->type == EFFECT_POLYGON_ROTATE_Y || eff->type == EFFECT_POLYGON_ROTATE_Y_CW;
	EffectTexture *texture = progress < 0.5f ? eff->tx_old : eff->tx_new;
	float angle = progress < 0.5f ? progress * M_PIf : (progress - 1.f) * M_PIf;
	if (eff->type == EFFECT_POLYGON_ROTATE_Y_CW || eff->type == EFFECT_POLYGON_ROTATE_X_CW)
		angle *= -1;

	const float distance = 2.f;
	float cos_a = cosf(angle);
	float sin_a = sinf(angle);
	float lz = -0.5f * sin_a + distance;
	float rz = 0.5f * sin_a + distance;
	float lx = -0.5f * cos_a / lz;
	float rx = 0.5f * cos_a / rz;

	int w = vertical ? eff->dst_rect.w : eff->dst_rect.h;
	int h = vertical ? eff->dst_rect.h : eff->dst_rect.w;
	float dlx = w * (lx * distance + 0.5f);
	float drx = w * (rx * distance + 0.5f);

	// Perspective-correct interpolation.
	for (int x = dlx; x <= drx; x++) {
		if (x < 0 || x >= w)
			continue;
		float t = (x - dlx) / (drx - dlx);
		float recip_z = (1 - t) * (1 / lz) + t * (1 / rz);
		float tx = t * (w / rz) / recip_z;
		SDL_Rect sr = {tx, 0, 1, h};
		float dh = h * recip_z * distance;
		SDL_Rect dr = {x, (h - dh) / 2, 1, dh};
		if (!vertical) {
			transpose_rect(&sr);
			transpose_rect(&dr);
		}
		render_effect_texture(texture, &sr, &dr);
	}

	SDL_RenderPresent(gfx_renderer);
}

static void polygon_rotate_free(struct effect *eff) {
	eff_finish(eff, true);
	free(eff);
}

// EFFECT_ZIGZAG_CROSSFADE

struct zigzag_crossfade_effect {
	struct effect eff;
	SDL_Texture *tmp1, *tmp2;
};

static void zigzag_crossfade_step(struct effect *eff, float progress);
static void zigzag_crossfade_free(struct effect *eff);

static struct effect *zigzag_crossfade_new(SDL_Rect *rect, EffectTexture *old, EffectTexture *new) {
	if (!SDL_RenderTargetSupported(gfx_renderer))
		return fallback_effect_new(rect, old, new, EFFECT_ZIGZAG_CROSSFADE);

	struct zigzag_crossfade_effect *eff = calloc(1, sizeof(struct zigzag_crossfade_effect));
	if (!eff)
		NOMEMERR();
	eff_init(&eff->eff, rect, old, new, EFFECT_ZIGZAG_CROSSFADE);
	eff->tmp1 = SDL_CreateTexture(gfx_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, rect->w, rect->h);
	eff->tmp2 = SDL_CreateTexture(gfx_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, rect->w, rect->h);
	eff->eff.step = zigzag_crossfade_step;
	eff->eff.finish = zigzag_crossfade_free;
	return &eff->eff;
}

static void wave_warp_h(EffectTexture *src, SDL_Texture *dst, int w, int h, float amplitude, float length, float phase) {
	SDL_SetRenderTarget(gfx_renderer, dst);
	SDL_RenderClear(gfx_renderer);
	SDL_Rect sr = {0, 0, w, 1};
	SDL_Rect dr = {0, 0, w, 1};
	for (int y = 0; y < h; y++) {
		sr.y = dr.y = y;
		dr.x = sinf(phase + y * (2 * M_PIf / 360 * length)) * amplitude;
		render_effect_texture(src, &sr, &dr);
	}
	SDL_SetRenderTarget(gfx_renderer, NULL);
}

static void wave_warp_v(SDL_Texture *src, SDL_Texture *dst, int w, int h, float amplitude, float length, float phase) {
	SDL_SetRenderTarget(gfx_renderer, dst);
	SDL_RenderClear(gfx_renderer);
	SDL_Rect sr = {0, 0, 1, h};
	SDL_Rect dr = {0, 0, 1, h};
	for (int x = 0; x < w; x++) {
		sr.x = dr.x = x;
		dr.y = sinf(phase + x * (2 * M_PIf / 360 * length)) * amplitude;
		SDL_RenderCopy(gfx_renderer, src, &sr, &dr);
	}
	SDL_SetRenderTarget(gfx_renderer, NULL);
}

static void zigzag_crossfade_step(struct effect *eff, float progress) {
	struct zigzag_crossfade_effect *this = (struct zigzag_crossfade_effect *)eff;

	float amp = (progress < 0.5f ? progress : 1.f - progress) * 80;
	float len = (progress < 0.5f ? progress : 1.f - progress) * 20;
	float phase = progress * 10;

	wave_warp_h(eff->tx_old, this->tmp1, eff->dst_rect.w, eff->dst_rect.h, amp, len, phase);
	wave_warp_v(this->tmp1, this->tmp2, eff->dst_rect.w, eff->dst_rect.h, amp, len, phase);
	SDL_RenderCopy(gfx_renderer, this->tmp2, NULL, &eff->dst_rect);

	wave_warp_h(eff->tx_new, this->tmp1, eff->dst_rect.w, eff->dst_rect.h, amp, len, phase);
	wave_warp_v(this->tmp1, this->tmp2, eff->dst_rect.w, eff->dst_rect.h, amp, len, phase);
	SDL_SetTextureBlendMode(this->tmp2, SDL_BLENDMODE_BLEND);
	SDL_SetTextureAlphaMod(this->tmp2, progress * 255);
	SDL_RenderCopy(gfx_renderer, this->tmp2, NULL, &eff->dst_rect);
	SDL_SetTextureBlendMode(this->tmp2, SDL_BLENDMODE_NONE);

	SDL_RenderPresent(gfx_renderer);
}

static void zigzag_crossfade_free(struct effect *eff) {
	struct zigzag_crossfade_effect *this = (struct zigzag_crossfade_effect *)eff;
	SDL_DestroyTexture(this->tmp1);
	SDL_DestroyTexture(this->tmp2);
	eff_finish(&this->eff, true);
	free(this);
}

// EFFECT_MAGNIFY

struct magnify_effect {
	struct effect eff;
	SDL_Rect new_rect;
};

static void magnify_step(struct effect *eff, float progress);
static void magnify_free(struct effect *eff);

static struct effect *magnify_new(EffectTexture *tx, SDL_Rect *old_rect, SDL_Rect *new_rect) {
	struct magnify_effect *eff = calloc(1, sizeof(struct magnify_effect));
	if (!eff)
		NOMEMERR();
	eff_init(&eff->eff, old_rect, tx, NULL, EFFECT_MAGNIFY);
	eff->new_rect = *new_rect;
	eff->eff.step = magnify_step;
	eff->eff.finish = magnify_free;
	return &eff->eff;
}

static void magnify_step(struct effect *eff, float progress) {
	struct magnify_effect *this = (struct magnify_effect *)eff;
	SDL_Rect *or = &eff->dst_rect;
	SDL_Rect *nr = &this->new_rect;
	SDL_Rect r = {
		.x = or->x + (nr->x - or->x) * progress,
		.y = or->y + (nr->y - or->y) * progress,
		.w = or->w + (nr->w - or->w) * progress,
		.h = or->h + (nr->h - or->h) * progress,
	};
	render_effect_texture(eff->tx_old, &r, NULL);
	SDL_RenderPresent(gfx_renderer);
}

static void magnify_free(struct effect *eff) {
	eff_finish(eff, false);
	free(eff);
}

// EFFECT_RASTER_BLEND

static void raster_blend_step(struct effect *eff, float progress);
static void raster_blend_free(struct effect *eff);

static struct effect *raster_blend_new(SDL_Rect *rect, int sx, int sy) {
	SDL_Surface *sprite = gfx_dib_to_surface_with_alpha(sx, sy, rect->w, rect->h);
	EffectTexture *texture = create_effect_texture_from_surface(sprite);
	SDL_FreeSurface(sprite);
	struct effect *eff = calloc(1, sizeof(struct effect));
	if (!eff)
		NOMEMERR();
	eff_init(eff, rect, NULL, texture, EFFECT_RASTER_BLEND);
	eff->step = raster_blend_step;
	eff->finish = raster_blend_free;
	return eff;
}

static void raster_blend_step(struct effect *eff, float progress) {
	SDL_Rect s_rect = { 0, 0, eff->dst_rect.w, 1 };
	SDL_Rect d_rect = eff->dst_rect;
	d_rect.h = 1;
	for (int y = 0; y < eff->dst_rect.h; y++) {
		float t = 1.f - 4.f * progress + 3.f * y / eff->dst_rect.h;
		d_rect.x = eff->dst_rect.x + (t < 0 ? 0 : sinf(2 * M_PIf * t) * 50);
		render_effect_texture(eff->tx_new, &s_rect, &d_rect);
		s_rect.y++;
		d_rect.y++;
	}
	SDL_RenderPresent(gfx_renderer);
}

static void raster_blend_free(struct effect *eff) {
	eff_finish(eff, false);
	free(eff);
}

// SACTAMASK

struct sactamask_effect {
	struct effect eff;
	SDL_Surface *sf_new;
	SDL_Surface *mask;
};

static void sactamask_step(struct effect *eff, float progress);
static void sactamask_free(struct effect *eff);

static struct effect *sactamask_new(EffectTexture *tx_old, SDL_Surface *sf_new, SDL_Surface *mask) {
	struct sactamask_effect *eff = calloc(1, sizeof(struct sactamask_effect));
	if (!eff)
		NOMEMERR();

	EffectTexture *tx_new = calloc(1, sizeof(EffectTexture));
	tx_new->tx = SDL_CreateTexture(gfx_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, sf_new->w, sf_new->h);
	tx_new->rect = (SDL_Rect){ 0, 0, sf_new->w, sf_new->h };
	SDL_SetTextureBlendMode(tx_new->tx, SDL_BLENDMODE_BLEND);

	SDL_Rect rect = {0, 0, mask->w, mask->h};
	eff_init(&eff->eff, &rect, tx_old, tx_new, EFFECT_SACTAMASK);
	eff->sf_new = sf_new;
	eff->mask = mask;
	eff->eff.step = sactamask_step;
	eff->eff.finish = sactamask_free;
	return &eff->eff;
}

static void sactamask_step(struct effect *eff, float progress) {
	struct sactamask_effect *this = (struct sactamask_effect *)eff;

	SDL_Surface *sf;
	SDL_LockTextureToSurface(eff->tx_new->tx, NULL, &sf);
	SDL_BlitSurface(this->sf_new, NULL, sf, NULL);

	// Scale the alpha value in mask and write it to the alpha channel of sf.
	int val = 255 * progress;
	for (int y = 0; y < this->mask->h; y++) {
		uint8_t *src = PIXEL_AT(this->mask, 0, y);
		uint8_t *dst = ALPHA_AT(sf, 0, y);
		for (int x = 0; x < this->mask->w; x++) {
			int i = 255 - (*src - val) * 16;
			*dst = min(255, max(0, i));
			src++; dst += 4;
		}
	}

	SDL_UnlockTexture(eff->tx_new->tx);
	render_effect_texture(eff->tx_old, NULL, &eff->dst_rect);
	render_effect_texture(eff->tx_new, NULL, &eff->dst_rect);
	SDL_RenderPresent(gfx_renderer);
}

static void sactamask_free(struct effect *eff) {
	eff_finish(eff, true);
	free(eff);
}

// -----------------

struct effect *effect_init(SDL_Rect *rect, surface_t *old, int ox, int oy, surface_t *new, int nx, int ny, enum effect_type type) {
	EffectTexture *tx_old = create_effect_texture(old, ox, oy, rect->w, rect->h);
	EffectTexture *tx_new = create_effect_texture(new, nx, ny, rect->w, rect->h);

	switch (type) {
	case EFFECT_CROSSFADE:
		return crossfade_new(rect, tx_old, tx_new);
	case EFFECT_CROSSFADE_DOWN:
	case EFFECT_CROSSFADE_UP:
	case EFFECT_CROSSFADE_LR:
	case EFFECT_CROSSFADE_RL:
	case EFFECT_CROSSFADE_LR_RL:
	case EFFECT_CROSSFADE_UP_DOWN:
		return crossfade_animation_new(rect, tx_old, tx_new, type);
	case EFFECT_MOSAIC:
	case EFFECT_CROSSFADE_MOSAIC:
		return mosaic_new(rect, tx_old, tx_new, type);
	case EFFECT_FADEOUT:
	case EFFECT_FADEOUT_FROM_NEW:
	case EFFECT_FADEIN:
	case EFFECT_WHITEOUT:
	case EFFECT_WHITEOUT_FROM_NEW:
	case EFFECT_WHITEIN:
		return brightness_new(rect, tx_old, tx_new, type);
	case EFFECT_DITHERING_FADEOUT:
	case EFFECT_DITHERING_FADEIN:
	case EFFECT_DITHERING_WHITEOUT:
	case EFFECT_DITHERING_WHITEIN:
		return dithering_fade_new(rect, tx_old, tx_new, type);
	case EFFECT_PAN_IN_DOWN:
	case EFFECT_PAN_IN_UP:
		return pan_in_new(rect, tx_old, tx_new, type);
	case EFFECT_SKIP_LINE_UP_DOWN:
	case EFFECT_SKIP_LINE_LR_RL:
		return skip_line_new(rect, tx_old, tx_new, type);
	case EFFECT_WIPE_IN:
	case EFFECT_WIPE_OUT:
	case EFFECT_WIPE_LR:
	case EFFECT_WIPE_RL:
	case EFFECT_WIPE_DOWN:
	case EFFECT_WIPE_UP:
	case EFFECT_WIPE_OUT_V:
	case EFFECT_WIPE_IN_V:
	case EFFECT_WIPE_OUT_H:
	case EFFECT_WIPE_IN_H:
		return wipe_new(rect, tx_old, tx_new, type);
	case EFFECT_CIRCLE_WIPE_OUT:
	case EFFECT_CIRCLE_WIPE_IN:
		return circle_wipe_new(rect, tx_old, tx_new, type);
	case EFFECT_BLIND_DOWN:
	case EFFECT_BLIND_UP:
	case EFFECT_BLIND_LR:
	case EFFECT_BLIND_RL:
	case EFFECT_BLIND_UP_DOWN:
	case EFFECT_BLIND_DOWN_LR:
		return blind_new(rect, tx_old, tx_new, type);
	case EFFECT_BLEND_UP_DOWN:
	case EFFECT_BLEND_LR_RL:
		return blend_animation_new(rect, tx_old, tx_new, type);
	case EFFECT_ZOOM_BLEND_BLUR:
		return zoom_blend_blur_new(rect, tx_old, tx_new);
	case EFFECT_LINEAR_BLUR:
	case EFFECT_LINEAR_BLUR_VERT:
		return linear_blur_new(rect, tx_old, tx_new, type);
	case EFFECT_PENTAGRAM_IN_OUT:
	case EFFECT_PENTAGRAM_OUT_IN:
	case EFFECT_HEXAGRAM_IN_OUT:
	case EFFECT_HEXAGRAM_OUT_IN:
	case EFFECT_WINDMILL:
	case EFFECT_WINDMILL_180:
	case EFFECT_WINDMILL_360:
		return polygon_mask_new(rect, tx_old, tx_new, type);
	case EFFECT_ZOOM_IN:
	case EFFECT_ROTATE_OUT:
	case EFFECT_ROTATE_IN:
	case EFFECT_ROTATE_OUT_CW:
	case EFFECT_ROTATE_IN_CW:
		return rotate_new(rect, tx_old, tx_new, type);
	case EFFECT_POLYGON_ROTATE_Y:
	case EFFECT_POLYGON_ROTATE_Y_CW:
	case EFFECT_POLYGON_ROTATE_X:
	case EFFECT_POLYGON_ROTATE_X_CW:
		return polygon_rotate_new(rect, tx_old, tx_new, type);
	case EFFECT_ZIGZAG_CROSSFADE:
		return zigzag_crossfade_new(rect, tx_old, tx_new);
	default:
		WARNING("Unknown effect %d", type);
		return crossfade_new(rect, tx_old, tx_new);
	}
}

struct effect *sprite_effect_init(SDL_Rect *rect, int dx, int dy, int sx, int sy, int col, enum effect_type type) {
	EffectTexture *tx_old = create_effect_texture(gfx_dibinfo, dx, dy, rect->w, rect->h);
	SDL_Surface *sprite = gfx_dib_to_surface_colorkey(sx, sy, rect->w, rect->h, col);
	EffectTexture *tx_new = create_effect_texture_from_surface(sprite);
	SDL_FreeSurface(sprite);

	switch (type) {
	case EFFECT_PAN_IN_DOWN:
	case EFFECT_PAN_IN_UP:
		return pan_in_new(rect, tx_old, tx_new, type);
	case EFFECT_SKIP_LINE_UP_DOWN:
	case EFFECT_SKIP_LINE_LR_RL:
		return skip_line_new(rect, tx_old, tx_new, type);
	case EFFECT_RASTER_BLEND:
		return raster_blend_new(rect, sx, sy);
	default:
		SYSERROR("Unknown sprite effect %d", type);
		return NULL;
	}
}

struct effect *effect_magnify_init(surface_t *surface, SDL_Rect *view_rect, SDL_Rect *target_rect) {
	EffectTexture *tx = create_effect_texture(surface, view_rect->x, view_rect->y, view_rect->w, view_rect->h);
	return magnify_new(tx, view_rect, target_rect);
}

struct effect *effect_sactamask_init(SDL_Surface *mask) {
	EffectTexture *tx_old = create_effect_texture(NULL, 0, 0, main_surface->w, main_surface->h);
	return sactamask_new(tx_old, main_surface, mask);
}

void effect_step(struct effect *eff, float progress) {
	SDL_RenderClear(gfx_renderer);
	if (!eff->is_fullscreen)
		SDL_RenderCopy(gfx_renderer, gfx_texture, NULL, NULL);
	eff->step(eff, progress);
}

void effect_finish(struct effect *eff) {
	eff->finish(eff);
}
