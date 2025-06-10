/*
 * ags.c  system35のグラフィックブリッジ
 *
 * Copyright (C) 1997-1998 Masaki Chikama (Wren) <chikama@kasumi.ipl.mech.nagoya-u.ac.jp>
 *               1998-                           <masaki-c@is.aist-nara.ac.jp>
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
/* $Id: ags.c,v 1.34 2004/10/31 04:18:05 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "portab.h"
#include "xsystem35.h"
#include "scenario.h"
#include "ags.h"
#include "gfx.h"
#include "sdl_core.h"
#include "alpha_plane.h"
#include "utfsjis.h"
#include "input.h"
#include "font.h"
#include "cursor.h"
#include "image.h"
#include "debugger.h"

static bool need_update = true;
static bool fade_outed = false;
static int cursor_move_time = 50; /* カーソル移動にかかる時間(ms) */

static void palette_changed(void) {
	nact->ags.pal_changed = true;
	dbg_on_palette_change();
}

static void initPal(void) {
	static const SDL_Color initial_palette[256] = {
		[  0] = {  0,   0,   0},
		[  1] = {128,   0,   0},
		[  2] = {  0, 128,   0},
		[  3] = {128, 128,   0},
		[  4] = {  0,   0, 128},
		[  5] = {128,   0, 128},
		[  6] = {  0, 128, 128},
		[  7] = {192, 192, 192},
		[  8] = {192, 220, 192},
		[  9] = {166, 202, 240},
		[ 15] = {255, 255, 255},
		[246] = {255, 251, 240},
		[247] = {160, 160, 164},
		[248] = {128, 128, 128},
		[249] = {255,   0,   0},
		[250] = {  0, 255,   0},
		[251] = {255, 255,   0},
		[252] = {  0,   0, 255},
		[253] = {255,   0, 255},
		[254] = {  0, 255, 255},
		[255] = {255, 255, 255},
	};
	memcpy(nact->ags.pal, initial_palette, sizeof(initial_palette));
	for (int i = 0; i < 256; i++) {
		nact->ags.pal[i].a = 255;
	}
	gfx_setPalette(nact->ags.pal, 0, 256);
	palette_changed();
}

bool ags_check_param(int *x, int *y, int *w, int *h) {
	if (*x >= nact->ags.world_width) {
		WARNING("Illegal Param x = %d (max=%d)(@%03x:%05x)", *x, nact->ags.world_width, sl_getPage(), sl_getIndex());
		return false;
	}
	if (*y >= nact->ags.world_height) {
		WARNING("Illegal Param y = %d (max=%d)", *y, nact->ags.world_height);
		return false;
	}
	
	if (*x < 0) { *w += *x; *x = 0; }
	if (*y < 0) { *h += *y; *y = 0; }
	
	if ((*x + *w) > nact->ags.world_width)  { *w = nact->ags.world_width  - *x;}
	if ((*y + *h) > nact->ags.world_height) { *h = nact->ags.world_height - *y;}
	
	if (*w <= 0) return false;
	if (*h <= 0) return false;
	
	return true;
}

bool ags_check_param_xy(int *x, int *y) {
	if (*x >= nact->ags.world_width) {
		WARNING("Illegal Param x = %d", *x);
		return false;
	}
	if (*y >= nact->ags.world_height) {
		WARNING("Illegal Param y = %d", *y);
		return false;
	}
	
	if (*x < 0) { *x = 0; }
	if (*y < 0) { *y = 0; }
	
	return true;
}

void ags_init(const char *render_driver, bool enable_zb) {
	nact->ags.mouse_warp_enabled = true;
	nact->ags.world_width  =  SYS35_DEFAULT_WIDTH;
	nact->ags.world_height =  SYS35_DEFAULT_HEIGHT;
	nact->ags.world_depth =  SYS35_DEFAULT_DEPTH;
	nact->ags.view_area.x = 0;
	nact->ags.view_area.y = 0;
	nact->ags.view_area.w = SYS35_DEFAULT_WIDTH;
	nact->ags.view_area.h = SYS35_DEFAULT_HEIGHT;

	nact->ags.font_type = FONT_GOTHIC;
	nact->ags.text_decoration_type = 0;
	nact->ags.text_decoration_color = 0;
	nact->ags.enable_zb = enable_zb;
	nact->ags.font_weight = enable_zb ? FONT_WEIGHT_BOLD : FONT_WEIGHT_NORMAL;
	
	gfx_Initialize(render_driver);
	font_init();

	initPal();
	cg_init();
}

void ags_remove(void) {
	ags_autorepeat(true);
	gfx_Remove();
}

void ags_reset(void) {
	nact->ags.mouse_warp_enabled = true;
	nact->ags.eventcb = NULL;
	initPal();
	cg_reset();
}

void ags_setWorldSize(int width, int height, int depth) {
	nact->ags.world_width  = width;
	nact->ags.world_height = height;
	nact->ags.world_depth       = depth;

	if (nact->ags.dib && nact->ags.dib->alpha) {
		free(nact->ags.dib->alpha);
		nact->ags.dib->alpha = NULL;
	}

	gfx_setWorldSize(width, height, depth);
	
	nact->ags.dib = gfx_getDIB();
	
	/* DIBが8以上の場合は、alpha plane を用意 */
	if (depth > 8) {
		nact->ags.dib->alpha = malloc(width * height);
		memset(nact->ags.dib->alpha, 255, width * height);
	}
	
	fade_outed = false;  /* thanx tajiri@wizard */
	
	palette_changed();
}

void ags_setViewArea(int x, int y, int width, int height) {
	nact->ags.view_area.x = x;
	nact->ags.view_area.y = y;
	nact->ags.view_area.w = width;
	nact->ags.view_area.h = height;
	gfx_setWindowSize(width, height);
}

void ags_setWindowTitle(const char *title_utf8) {
	char buf[256];
	snprintf(buf, sizeof(buf), "XSystem35 Version %s: %s", VERSION, title_utf8);
	gfx_setWindowTitle(buf);
}

void ags_getDIBInfo(DispInfo *info) {
	info->width  = nact->ags.world_width;
	info->height = nact->ags.world_height;
	info->depth  = nact->ags.world_depth;
}

void ags_getViewAreaInfo(DispInfo *info) {
	gfx_getWindowInfo(NULL, NULL, &info->depth);
	info->width  = nact->ags.view_area.w;
	info->height = nact->ags.view_area.h;
}

void ags_getWindowInfo(DispInfo *info) {
	gfx_getWindowInfo(&info->width, &info->height, &info->depth);
}

void ags_setExposeSwitch(bool expose) {
	need_update = expose;
}

void ags_updateArea(int x, int y, int w, int h) {
	if (fade_outed || !need_update)
		return;

	SDL_Rect r = {x, y, w, h}, update;
	if (SDL_IntersectRect(&nact->ags.view_area, &r, &update)) {
		SDL_Point p = {
			update.x - nact->ags.view_area.x,
			update.y - nact->ags.view_area.y
		};
		gfx_updateArea(&update, &p);
	}
}

void ags_updateFull() {
	if (fade_outed || !need_update)
		return;

	SDL_Rect r = {
		nact->ags.view_area.x,
		nact->ags.view_area.y,
		min(nact->ags.view_area.w, nact->ags.world_width),
		min(nact->ags.view_area.h, nact->ags.world_height)
	};
	SDL_Point p = {0, 0};
	gfx_updateArea(&r, &p);
}

void ags_setPalettes(SDL_Color *src, int dst, int cnt) {
	for (int i = 0; i < cnt; i++) {
		nact->ags.pal[dst + i] = src[i];
	}
	palette_changed();
}

void ags_setPalette(int no, int red, int green, int blue) {
	nact->ags.pal[no].r = red;
	nact->ags.pal[no].g = green;
	nact->ags.pal[no].b = blue;
	palette_changed();
}

void ags_setPaletteToSystem(int src, int cnt) {
	if (!fade_outed) 
		gfx_setPalette(nact->ags.pal, src, cnt);
}

void ags_drawRectangle(int x, int y, int w, int h, int col) {
	if (!ags_check_param(&x, &y, &w, &h)) return;
	
	gfx_drawRectangle(x, y, w, h, col);
}

void ags_fillRectangle(int x, int y, int w, int h, int col) {
	if (!ags_check_param(&x, &y, &w, &h)) return;

	gfx_fillRectangle(x, y, w, h, col);
}

void ags_drawLine(int x0, int y0, int x1, int y1, int col) {
	if (!ags_check_param_xy(&x0, &y0)) return;
	if (!ags_check_param_xy(&x1, &y1)) return;

	gfx_drawLine(x0, y0, x1, y1, col);
}

void ags_copyArea(int sx, int sy, int w, int h, int dx, int dy) {
	if (!ags_check_param(&sx, &sy, &w, &h)) return;
	if (!ags_check_param(&dx, &dy, &w, &h)) return;
	
	gfx_copyArea(sx, sy, w, h, dx, dy);
}

void ags_scaledCopyArea(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, int mirror_sw) {
	if (!ags_check_param(&sx, &sy, &sw, &sh)) return;
	if (!ags_check_param(&dx, &dy, &dw, &dh)) return;
	
	gfx_scaledCopyArea(sx, sy, sw, sh, dx, dy, dw, dh, mirror_sw);
}

void ags_copyAreaSP(int sx, int sy, int w, int h, int dx, int dy, int col) {
	if (!ags_check_param(&sx, &sy, &w, &h)) return;
	if (!ags_check_param(&dx, &dy, &w, &h)) return;

	gfx_copyAreaSP(sx, sy, w, h, dx, dy, col);
}

void ags_wrapColor(int x, int y, int w, int h, int p1, int p2) {
	if (nact->ags.world_depth == 8) return;
	
	if (!ags_check_param(&x, &y, &w, &h)) return;
	
	gfx_wrapColor(x, y, w, h, p1, p2);
}

void ags_getPixel(int x, int y, PixelColor *cell) {
	if (!ags_check_param_xy(&x, &y)) return;

	uint32_t pixel = gfx_getPixel(x, y);
	if (main_surface->format->BitsPerPixel == 8) {
		cell->index = pixel;
	} else {
		SDL_GetRGB(pixel, main_surface->format, &cell->r, &cell->g, &cell->b);
	}
}

void ags_copyPaletteShift(int sx, int sy, int w, int h, int dx, int dy, uint8_t sprite) {
	if (!ags_check_param(&sx, &sy, &w, &h)) return;
	if (!ags_check_param(&dx, &dy, &w, &h)) return;

	for (int y = 0; y < h; y++) {
		uint8_t *src = GETOFFSET_PIXEL(nact->ags.dib, sx, sy + y);
		uint8_t *dst = GETOFFSET_PIXEL(nact->ags.dib, dx, dy + y);
		for (int x = 0; x < w; x++, src++, dst++) {
			if (*src != sprite)
				*dst = (*src & 0xf0) | (*dst & 0x0f);
		}
	}
}

void ags_changeColorArea(int sx, int sy, int w, int h, int dst, int src, int cnt) {
	if (nact->ags.world_depth != 8) return;
	
	if (!ags_check_param(&sx, &sy, &w, &h)) return;
	
	{
		surface_t *dib = nact->ags.dib;
		int   x, y;
		int   src_last = src + cnt,dif = dst - src;
		uint8_t *yl;
		uint8_t *sdata = GETOFFSET_PIXEL(dib, sx, sy);
		
		for (y = 0; y < h; y++) {
			yl = sdata + y * dib->sdl_surface->pitch;
			for (x = 0; x < w; x++) {
				if (*yl >= src && *yl < src_last) *yl += dif;
				yl++;
			}
		}
	}
}

void* ags_saveRegion(int x, int y, int w, int h) {
	if (!ags_check_param(&x, &y, &w, &h)) return NULL;

	return (void *)gfx_saveRegion(x, y, w, h);
}

void ags_restoreRegion(void *region, int x, int y) {
	if (region == NULL) return;
	
	if (!ags_check_param_xy(&x, &y)) return;
	
	gfx_restoreRegion(region, x, y);
}

void ags_putRegion(void *region, int x, int y) {
	if (region == NULL) return;
	
	if (!ags_check_param_xy(&x, &y)) return;
	
	gfx_putRegion(region, x, y);
}

void ags_delRegion(void *region) {
	if (region == NULL) return;
	
	gfx_delRegion(region);
}

int ags_drawString(int x, int y, const char *src, int col, SDL_Rect *rect_out) {
	if (!ags_check_param_xy(&x, &y)) {
		if (rect_out) {
			rect_out->x = x;
			rect_out->y = y;
			rect_out->w = 0;
			rect_out->h = 0;
		}
		return 0;
	}
	
	SDL_Rect adj;
	char *utf8 = toUTF8(src);
	switch(nact->ags.text_decoration_type) {
	case TEXT_DECORATION_NONE:
	default:
		adj.x = 0; adj.y = 0; adj.w = 0; adj.h = 0;
		break;
	case TEXT_DECORATION_DROP_SHADOW_BOTTOM:
		gfx_drawString(x, y +1, utf8, nact->ags.text_decoration_color);
		adj.x = 0; adj.y = 0; adj.w = 0; adj.h = 1;
		break;
	case TEXT_DECORATION_DROP_SHADOW_RIGHT:
		gfx_drawString(x +1, y, utf8, nact->ags.text_decoration_color);
		adj.x = 0; adj.y = 0; adj.w = 1; adj.h = 0;
		break;
	case TEXT_DECORATION_DROP_SHADOW_BOTTOM_RIGHT:
		gfx_drawString(x +1, y +1, utf8, nact->ags.text_decoration_color);
		adj.x = 0; adj.y = 0; adj.w = 1; adj.h = 1;
		break;
	case TEXT_DECORATION_OUTLINE:
		gfx_drawString(x -1, y, utf8, nact->ags.text_decoration_color);
		gfx_drawString(x +1, y, utf8, nact->ags.text_decoration_color);
		gfx_drawString(x, y -1, utf8, nact->ags.text_decoration_color);
		gfx_drawString(x, y +1, utf8, nact->ags.text_decoration_color);
		adj.x = -1; adj.y = -1; adj.w = 2; adj.h = 2;
		break;
	case TEXT_DECORATION_BOLD_HORIZONTAL:
		gfx_drawString(x +1, y, utf8, col);
		adj.x = 0; adj.y = 0; adj.w = 1; adj.h = 0;
		break;
	case TEXT_DECORATION_BOLD_VERTICAL:
		gfx_drawString(x, y +1, utf8, col);
		adj.x = 0; adj.y = 0; adj.w = 0; adj.h = 1;
		break;
	case TEXT_DECORATION_BOLD_HORIZONTAL_VERTICAL:
		gfx_drawString(x +1, y +1, utf8, col);
		adj.x = 0; adj.y = 0; adj.w = 1; adj.h = 1;
		break;
	case TEXT_DECORATION_DROP_SHADOW_OUTLINE:
		gfx_drawString(x -1, y   , utf8, nact->ags.text_decoration_color);
		gfx_drawString(x +1, y   , utf8, nact->ags.text_decoration_color);
		gfx_drawString(x   , y -1, utf8, nact->ags.text_decoration_color);
		gfx_drawString(x   , y +1, utf8, nact->ags.text_decoration_color);
		gfx_drawString(x +2, y +2, utf8, nact->ags.text_decoration_color);
		adj.x = -1; adj.y = -1; adj.w = 3; adj.h = 3;
		break;
	}
	SDL_Rect r = gfx_drawString(x, y, utf8, col);
	if (rect_out) {
		rect_out->x = r.x + adj.x;
		rect_out->y = r.y + adj.y;
		rect_out->w = r.w + adj.w;
		rect_out->h = r.h + adj.h;
	}
	free(utf8);
	return r.w;
}

SDL_Surface *ags_drawStringToSurface(const char *str, int r, int g, int b) {
	char *utf8 = toUTF8(str);
	SDL_Surface *sf = font_get_glyph(utf8, r, g, b);
	free(utf8);
	return sf;
}

void ags_drawCg(cgdata *cg, int x, int y, int brightness, int sprite_color, bool alpha_blend) {
	switch (cg->depth) {
	case 8:
		gfx_drawImage8(cg, x, y, sprite_color);
		break;
	case 16:
		gfx_drawImage16(cg, nact->ags.dib, x, y, brightness, alpha_blend);
		break;
	case 24:
		gfx_drawImage24(cg, nact->ags.dib, x, y, brightness);
		break;
	}
}

void ags_copyArea_shadow(int sx, int sy, int w, int h, int dx, int dy) {
	if (nact->ags.world_depth == 8) return;
	
	if (!ags_check_param(&sx, &sy, &w, &h)) return;
	if (!ags_check_param(&dx, &dy, &w, &h)) return;
	
	gfx_copyAreaSP16_shadow(sx, sy, w, h, dx, dy, 255);
}

void ags_copyArea_transparent(int sx, int sy, int w, int h, int dx, int dy, int col) {
	if (nact->ags.world_depth == 8) return;

	if (!ags_check_param(&sx, &sy, &w, &h)) return;
	if (!ags_check_param(&dx, &dy, &w, &h)) return;
	
	gfx_copyAreaSP(sx, sy, w, h, dx, dy, col);
}

void ags_copyArea_alphaLevel(int sx, int sy, int w, int h, int dx, int dy, int lv) {
	if (nact->ags.world_depth == 8) return;

	if (!ags_check_param(&sx, &sy, &w, &h)) return;
	if (!ags_check_param(&dx, &dy, &w, &h)) return;
	
	gfx_copyAreaSP16_alphaLevel(sx, sy, w, h, dx, dy, lv);
}

void ags_copyArea_alphaBlend(int sx, int sy, int w, int h, int dx, int dy, int lv) {
	if (nact->ags.world_depth == 8) return;

	if (!ags_check_param(&sx, &sy, &w, &h)) return;
	if (!ags_check_param(&dx, &dy, &w, &h)) return;
	
	gfx_copyAreaSP16_alphaBlend(sx, sy, w, h, dx, dy, lv);
}

SDL_Rect ags_floodFill(int x, int y, int col) {
	if (!ags_check_param_xy(&x, &y))
		return (SDL_Rect){};

	return gfx_floodFill(x, y, col);
}

void ags_copyFromAlpha(int sx, int sy, int w, int h, int dx, int dy, ALPHA_DIB_COPY_TYPE flg) {
	if (nact->ags.world_depth == 8) return;

	if (!ags_check_param(&sx, &sy, &w, &h)) return;
	if (!ags_check_param(&dx, &dy, &w, &h)) return;
	
	gfx_copy_from_alpha(sx, sy, w, h, dx, dy, flg);
}

void ags_copyToAlpha(int sx, int sy, int w, int h, int dx, int dy, ALPHA_DIB_COPY_TYPE flg) {
	if (nact->ags.world_depth == 8) return;

	if (!ags_check_param(&sx, &sy, &w, &h)) return;
	if (!ags_check_param(&dx, &dy, &w, &h)) return;
	
	gfx_copy_to_alpha(sx, sy, w, h, dx, dy, flg);
}

void ags_alpha_uppercut(int sx, int sy, int w, int h, int s, int d) {
	if (nact->ags.world_depth == 8) return;
	
	if (!ags_check_param(&sx, &sy, &w, &h)) return;
	
	alpha_uppercut(nact->ags.dib, sx, sy, w, h, s, d);
}

void ags_alpha_lowercut(int sx, int sy, int w, int h, int s, int d) {
	if (nact->ags.world_depth == 8) return;
	
	if (!ags_check_param(&sx, &sy, &w, &h)) return;

	alpha_lowercut(nact->ags.dib, sx, sy, w, h, s, d);
}

void ags_alpha_setLevel(int x, int y, int w, int h, int lv) {
	if (nact->ags.world_depth == 8) return;
	
	if (!ags_check_param(&x, &y, &w, &h)) return;

	alpha_set_level(nact->ags.dib, x, y, w, h, lv);
}

void ags_alpha_copyArea(int sx, int sy, int w, int h, int dx, int dy) {
	if (nact->ags.world_depth == 8) return;
	
	if (!ags_check_param(&sx, &sy, &w, &h)) return;
	if (!ags_check_param(&dx, &dy, &w, &h)) return;
	
	alpha_copy_area(nact->ags.dib, sx, sy, w, h, dx, dy);
}

void ags_alpha_getPixel(int x, int y, int *pic) {
	if (nact->ags.world_depth == 8) return;
	
	if (!ags_check_param_xy(&x, &y)) {
		*pic = 0;
	} else {
		alpha_get_pixel(nact->ags.dib, x, y, (uint8_t *)pic);
	}
}

void ags_alpha_setPixel(int x, int y, int w, int h, uint8_t *b) {
	int savex, savey, savew, offset;
	
	savex = x;
	savey = y;
	savew = w;
	
	if (!ags_check_param(&x, &y, &w, &h)) return;
	
	offset = abs(savey - y) * savew + abs(savex - x);
	
	alpha_set_pixels(nact->ags.dib, x, y, w, h, b + offset, savew);
}

void ags_runEffect(int duration_ms, bool cancelable, ags_EffectStepFunc step, void *arg) {
	unsigned wflags = cancelable ? KEYWAIT_CANCELABLE : KEYWAIT_NONCANCELABLE;
	int start = sdl_getTicks();
	for (int t = 0; t < duration_ms; t = sdl_getTicks() - start) {
		step(arg, (float)t / duration_ms);
		int key = sys_keywait(start + t + 16 - sdl_getTicks(), wflags);
		if (cancelable && key) {
			nact->waitcancel_key = key;
			break;
		}
	}
	step(arg, 1.0);
}

static void fade(int duration, bool cancelable, enum sdl_effect_type type) {
	nact->waitcancel_key = 0;

	SDL_Rect rect = {0, 0, nact->ags.view_area.w, nact->ags.view_area.h};
	struct sdl_effect *eff = sdl_effect_init(
		&rect, NULL, 0, 0,
		gfx_getDIB(), nact->ags.view_area.x, nact->ags.view_area.y,
		type);
	ags_runEffect(duration, cancelable, (ags_EffectStepFunc)sdl_effect_step, eff);
	sdl_effect_finish(eff);
}

void ags_fadeIn(int rate, bool flag) {
	fade_outed = false;
	if (nact->ags.world_depth == 8)
		gfx_setPalette(nact->ags.pal, 0, 256);

	if (!need_update)
		return;

	int duration = rate * 16 * 1000 / 60;
	fade(duration, flag, nact->ags.world_depth == 8 ? EFFECT_FADEIN : EFFECT_DITHERING_FADEIN);

	gfx_updateAll(&nact->ags.view_area);
}

void ags_fadeOut(int rate, bool flag) {
	if (need_update && !fade_outed) {
		int duration = rate * 16 * 1000 / 60;
		fade(duration, flag, nact->ags.world_depth == 8 ? EFFECT_FADEOUT : EFFECT_DITHERING_FADEOUT);
	}
	fade_outed = true;

	if (nact->ags.world_depth == 8) {
		SDL_Color pal[256];
		for (int i = 0; i < 256; i++) {
			pal[i] = (SDL_Color){0, 0, 0, 255};
		}
		gfx_setPalette(pal, 0, 256);
	}
}

void ags_whiteIn(int rate, bool flag) {	
	fade_outed = false;
	if (nact->ags.world_depth == 8)
		gfx_setPalette(nact->ags.pal, 0, 256);

	if (!need_update)
		return;

	int duration = rate * 16 * 1000 / 60;
	fade(duration, flag, nact->ags.world_depth == 8 ? EFFECT_WHITEIN : EFFECT_DITHERING_WHITEIN);

	gfx_updateAll(&nact->ags.view_area);
}

void ags_whiteOut(int rate, bool flag) {
	if (need_update && !fade_outed) {
		int duration = rate * 16 * 1000 / 60;
		fade(duration, flag, nact->ags.world_depth == 8 ? EFFECT_WHITEIN : EFFECT_DITHERING_WHITEOUT);
	}
	fade_outed = true;

	if (nact->ags.world_depth == 8) {
		SDL_Color pal[256];
		memset(&pal, 255, sizeof(pal));
		gfx_setPalette(pal, 0, 256);
	}
}

void ags_setFont(FontType type, int size) {
	font_select(type, size, FONT_WEIGHT_NORMAL);
}

void ags_setFontWithWeight(FontType type, int size, int weight) {
	font_select(type, size, weight);
}

void ags_setTextDecorationType(TextDecorationType type) {
	nact->ags.text_decoration_type = type;
}

void ags_setTextDecorationColor(int col) {
	nact->ags.text_decoration_color = col;
}

void ags_setCursorType(int type) {
	if (nact->ags.noimagecursor && type >= 100) return;
	sdl_setCursorType(type);
}

void ags_loadCursor(int p1,int p2) {
	if (!nact->ags.noimagecursor) {
		cursor_load(p1, p2);
	}
}

void ags_setCursorLocation(int x, int y, bool is_dibgeo, bool for_selection) {
	if (!ags_check_param_xy(&x, &y)) return;

	if (is_dibgeo) {
		// DIB coordinates -> Window coordinates
		x -= nact->ags.view_area.x;
		y -= nact->ags.view_area.y;
	}

#ifdef __EMSCRIPTEN__
	if (!for_selection) {
		// We can't move the actual cursor in the browser, but can change the
		// internal mouse coordinates. This can help with keyboard/gamepad
		// navigation.
		sdl_setCursorInternalLocation(x, y);
		EM_ASM({ xsystem35.shell.showMouseMoveEffect($0, $1); }, x, y);
		sdl_sleep(cursor_move_time);
	}
#else
	if (nact->ags.mouse_warp_enabled) {
		SDL_Point p;
		sys_getMouseInfo(&p, is_dibgeo);
		int dx = x - p.x;
		int dy = y - p.y;
		for (int i = 1; i < 8; i++) {
			int xi = ((dx*i*i*i) >> 9) - ((3*dx*i*i)>> 6) + ((3*dx*i) >> 3) + p.x;
			int yi = ((dy*i*i*i) >> 9) - ((3*dy*i*i)>> 6) + ((3*dy*i) >> 3) + p.y;
			sdl_setCursorLocation(xi, yi);
			sdl_sleep(cursor_move_time / 7);
		}
		sdl_setCursorLocation(x, y);
	} else if (!for_selection) {
		sdl_setCursorInternalLocation(x, y);
		sdl_sleep(cursor_move_time);
	}
#endif
}

EMSCRIPTEN_KEEPALIVE
void ags_setAntialiasedStringMode(bool on) {
	if (!nact->ags.noantialias) {
		font_set_antialias(on);
	}
}

bool ags_getAntialiasedStringMode() {
	return font_get_antialias();
}

void ags_copyArea_shadow_withrate(int sx, int sy, int w, int h, int dx, int dy, int lv) {
	if (nact->ags.world_depth == 8) return;
	
	if (lv == 0) return;
	
	if (!ags_check_param(&sx, &sy, &w, &h)) return;
	if (!ags_check_param(&dx, &dy, &w, &h)) return;
	
	gfx_copyAreaSP16_shadow(sx, sy, w, h, dx, dy, lv);
} 

void ags_setCursorMoveTime(int msec) {
	 cursor_move_time = msec;
}

int ags_getCursorMoveTime() {
	 return cursor_move_time;
}

surface_t *ags_getDIB() {
	return nact->ags.dib;
}

void ags_autorepeat(bool enable) {
	sdl_setAutoRepeat(enable);
}

bool ags_clipCopyRect(const SDL_Rect *sw, const SDL_Rect *dw, int *sx, int *sy, int *dx, int *dy, int *w, int *h) {
	// Clip source rectangle to source window
	SDL_Rect sr = {*sx, *sy, *w, *h};
	if (!SDL_IntersectRect(&sr, sw, &sr))
		return false;

	// Shift destination rectangle if source origin has changed
	int dx_ = *dx + (sr.x - *sx);
	int dy_ = *dy + (sr.y - *sy);

	// Clip destination rectangle to destination window
	SDL_Rect dr = {dx_, dy_, sr.w, sr.h};
	if (!SDL_IntersectRect(&dr, dw, &dr))
		return false;

	// Shift source rectangle if destination origin has changed
	sr.x += dr.x - dx_;
	sr.y += dr.y - dy_;

	*sx = sr.x;
	*sy = sr.y;
	*w = dr.w;
	*h = dr.h;
	*dx = dr.x;
	*dy = dr.y;
	return true;
}
