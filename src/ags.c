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
#include "sdl_core.h"
#include "alpha_plane.h"
#include "utfsjis.h"
#include "input.h"
#include "font.h"
#include "cursor.h"
#include "image.h"

static Palette256 pal_256;
static boolean need_update = TRUE;
static boolean fade_outed = FALSE;
static int cursor_move_time = 50; /* カーソル移動にかかる時間(ms) */

static void initPal(Palette256 *pal) {
	int i;
	for (i = 0; i < 256; i++) {
		pal->red[i]   =   0; pal->green[i]   =   0; pal->blue[i]   =   0;
	}
	pal->red[0]   =   0; pal->green[0]   =   0; pal->blue[0]   =   0;
	pal->red[7]   = 255; pal->green[7]   = 255; pal->blue[7]   = 255;
	pal->red[15]  = 255; pal->green[15]  = 255; pal->blue[15]  = 255;
	pal->red[255] = 255; pal->green[255] = 255; pal->blue[255] = 255;
	sdl_setPalette(pal, 0, 256);
	nact->ags.pal_changed = TRUE;
}

boolean ags_check_param(int *x, int *y, int *w, int *h) {
	if (*x >= nact->ags.world_size.width) {
		WARNING("Illegal Param x = %d (max=%d)(@%03x:%05x)", *x, nact->ags.world_size.width, sl_getPage(), sl_getIndex());
		return FALSE;
	}
	if (*y >= nact->ags.world_size.height) {
		WARNING("Illegal Param y = %d (max=%d)", *y, nact->ags.world_size.height);
		return FALSE;
	}
	
	if (*x < 0) { *w += *x; *x = 0; }
	if (*y < 0) { *h += *y; *y = 0; }
	
	if ((*x + *w) > nact->ags.world_size.width)  { *w = nact->ags.world_size.width  - *x;}
	if ((*y + *h) > nact->ags.world_size.height) { *h = nact->ags.world_size.height - *y;}
	
	if (*w <= 0) return FALSE;
	if (*h <= 0) return FALSE;
	
	return TRUE;
}

boolean ags_check_param_xy(int *x, int *y) {
	if (*x >= nact->ags.world_size.width) {
		WARNING("Illegal Param x = %d", *x);
		return FALSE;
	}
	if (*y >= nact->ags.world_size.height) {
		WARNING("Illegal Param y = %d", *y);
		return FALSE;
	}
	
	if (*x < 0) { *x = 0; }
	if (*y < 0) { *y = 0; }
	
	return TRUE;
}

void ags_init(const char *render_driver) {
	nact->ags.mouse_movesw = MOUSE_WARP_SMOOTH;
	nact->ags.pal = &pal_256;
	nact->ags.world_size.width  =  SYS35_DEFAULT_WIDTH;
	nact->ags.world_size.height =  SYS35_DEFAULT_HEIGHT;
	nact->ags.world_depth =  SYS35_DEFAULT_DEPTH;
	nact->ags.view_area.x = 0;
	nact->ags.view_area.y = 0;
	nact->ags.view_area.w = SYS35_DEFAULT_WIDTH;
	nact->ags.view_area.h = SYS35_DEFAULT_HEIGHT;
	
	sdl_Initialize(render_driver);
	font_init();

	initPal(&pal_256);
	cg_init();
}

void ags_remove(void) {
	ags_autorepeat(TRUE);
	sdl_Remove();
}

void ags_reset(void) {
	nact->ags.mouse_movesw = MOUSE_WARP_SMOOTH;
	nact->ags.eventcb = NULL;
	initPal(&pal_256);
	cg_reset();
}

void ags_setWorldSize(int width, int height, int depth) {
	nact->ags.world_size.width  = width;
	nact->ags.world_size.height = height;
	nact->ags.world_depth       = depth;

	if (nact->ags.dib && nact->ags.dib->alpha) {
		free(nact->ags.dib->alpha);
		nact->ags.dib->alpha = NULL;
	}

	sdl_setWorldSize(width, height, depth);
	
	nact->ags.dib = sdl_getDIB();
	
	/* DIBが8以上の場合は、alpha plane を用意 */
	if (depth > 8) {
		nact->ags.dib->alpha = malloc(width * height);
		memset(nact->ags.dib->alpha, 255, width * height);
	}
	
	fade_outed = FALSE;  /* thanx tajiri@wizard */
	
	nact->ags.pal_changed = TRUE;
}

void ags_setViewArea(int x, int y, int width, int height) {
	nact->ags.view_area.x = x;
	nact->ags.view_area.y = y;
	nact->ags.view_area.w = width;
	nact->ags.view_area.h = height;
	sdl_setWindowSize(width, height);
}

void ags_setWindowTitle(const char *title_utf8) {
	char buf[256];
	snprintf(buf, sizeof(buf), "XSystem35 Version %s: %s", VERSION, title_utf8);
	sdl_setWindowTitle(buf);
}

void ags_getDIBInfo(DispInfo *info) {
	info->width  = nact->ags.world_size.width;
	info->height = nact->ags.world_size.height;
	info->depth  = nact->ags.world_depth;
}

void ags_getViewAreaInfo(DispInfo *info) {
	sdl_getWindowInfo(info);
	info->width  = nact->ags.view_area.w;
	info->height = nact->ags.view_area.h;
}

void ags_getWindowInfo(DispInfo *info) {
	sdl_getWindowInfo(info);
}

void ags_setExposeSwitch(boolean expose) {
	need_update = expose;
}

void ags_updateArea(int x, int y, int w, int h) {
	if (fade_outed || !need_update)
		return;

	MyRectangle r = {x, y, w, h}, update;
	if (SDL_IntersectRect(&nact->ags.view_area, &r, &update)) {
		MyPoint p = {
			update.x - nact->ags.view_area.x,
			update.y - nact->ags.view_area.y
		};
		sdl_updateArea(&update, &p);
	}
}

void ags_updateFull() {
	if (fade_outed || !need_update)
		return;

	MyRectangle r = {
		nact->ags.view_area.x,
		nact->ags.view_area.y,
		min(nact->ags.view_area.w, nact->ags.world_size.width),
		min(nact->ags.view_area.h, nact->ags.world_size.height)
	};
	MyPoint p = {0, 0};
	sdl_updateArea(&r, &p);
}

void ags_setPalettes(Palette256 *src_pal, int src, int dst, int cnt) {
	int i;
	for (i = 0; i < cnt; i++) {
		nact->ags.pal->red  [dst + i] = src_pal->red  [src + i];
		nact->ags.pal->green[dst + i] = src_pal->green[src + i];
		nact->ags.pal->blue [dst + i] = src_pal->blue [src + i];
	}
	nact->ags.pal_changed = TRUE;
}

void ags_setPalette(int no, int red, int green, int blue) {
	nact->ags.pal->red[no]   = red;
	nact->ags.pal->green[no] = green;
	nact->ags.pal->blue[no]  = blue;
	nact->ags.pal_changed = TRUE;
}

void ags_setPaletteToSystem(int src, int cnt) {
	if (!fade_outed) 
		sdl_setPalette(nact->ags.pal, src, cnt);
}

void ags_drawRectangle(int x, int y, int w, int h, int col) {
	if (!ags_check_param(&x, &y, &w, &h)) return;
	
	sdl_drawRectangle(x, y, w, h, col);
}

void ags_fillRectangle(int x, int y, int w, int h, int col) {
	if (!ags_check_param(&x, &y, &w, &h)) return;

	sdl_fillRectangle(x, y, w, h, col);
}

void ags_drawLine(int x0, int y0, int x1, int y1, int col) {
	if (!ags_check_param_xy(&x0, &y0)) return;
	if (!ags_check_param_xy(&x1, &y1)) return;

	sdl_drawLine(x0, y0, x1, y1, col);
}

void ags_copyArea(int sx, int sy, int w, int h, int dx, int dy) {
	if (!ags_check_param(&sx, &sy, &w, &h)) return;
	if (!ags_check_param(&dx, &dy, &w, &h)) return;
	
	sdl_copyArea(sx, sy, w, h, dx, dy);
}

void ags_scaledCopyArea(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, int mirror_sw) {
	if (!ags_check_param(&sx, &sy, &sw, &sh)) return;
	if (!ags_check_param(&dx, &dy, &dw, &dh)) return;
	
	sdl_scaledCopyArea(sx, sy, sw, sh, dx, dy, dw, dh, mirror_sw);
}

void ags_copyAreaSP(int sx, int sy, int w, int h, int dx, int dy, int col) {
	if (!ags_check_param(&sx, &sy, &w, &h)) return;
	if (!ags_check_param(&dx, &dy, &w, &h)) return;

	sdl_copyAreaSP(sx, sy, w, h, dx, dy, col);
}

void ags_wrapColor(int x, int y, int w, int h, int p1, int p2) {
	if (nact->ags.world_depth == 8) return;
	
	if (!ags_check_param(&x, &y, &w, &h)) return;
	
	sdl_wrapColor(x, y, w, h, p1, p2);
}

void ags_getPixel(int x, int y, Palette *cell) {
	if (!ags_check_param_xy(&x, &y)) return;

	sdl_getPixel(x, y, cell);
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
		agsurface_t *dib = nact->ags.dib;
		int   x, y;
		int   src_last = src + cnt,dif = dst - src;
		uint8_t *yl;
		uint8_t *sdata = GETOFFSET_PIXEL(dib, sx, sy);
		
		for (y = 0; y < h; y++) {
			yl = sdata + y * dib->bytes_per_line;
			for (x = 0; x < w; x++) {
				if (*yl >= src && *yl < src_last) *yl += dif;
				yl++;
			}
		}
	}
}

void* ags_saveRegion(int x, int y, int w, int h) {
	if (!ags_check_param(&x, &y, &w, &h)) return NULL;

	return (void *)sdl_saveRegion(x, y, w, h);
}

void ags_restoreRegion(void *region, int x, int y) {
	if (region == NULL) return;
	
	if (!ags_check_param_xy(&x, &y)) return;
	
	sdl_restoreRegion(region, x, y);
}

void ags_putRegion(void *region, int x, int y) {
	if (region == NULL) return;
	
	if (!ags_check_param_xy(&x, &y)) return;
	
	sdl_putRegion(region, x, y);
}

void ags_copyRegion(void *region, int sx, int sy , int w, int h, int dx, int dy) {
	if (region == NULL) return;
	
	if (!ags_check_param_xy(&dx, &dy)) return;
	if (!ags_check_param(&dx, &dy, &w, &h)) return;
	
	sdl_CopyRegion(region, sx, sy, w, h, dx, dy);
}

void ags_delRegion(void *region) {
	if (region == NULL) return;
	
	sdl_delRegion(region);
}

MyRectangle ags_drawString(int x, int y, const char *src, int col) {
	if (!ags_check_param_xy(&x, &y)) return (MyRectangle){};
	
	char *utf8 = toUTF8(src);
	SDL_Rect r = sdl_drawString(x, y, utf8, col);
	free(utf8);

	return (MyRectangle){r.x, r.y, r.w, r.h};
}

agsurface_t *ags_drawStringToSurface(const char *str) {
	static SDL_Surface *fs;
	static agsurface_t result;
	if (fs) {
		SDL_FreeSurface(fs);
		fs = NULL;
	}

	char *utf8 = toUTF8(str);
	fs = font_get_glyph(utf8);
	free(utf8);

	result.depth = fs->format->BitsPerPixel;
	result.bytes_per_pixel = fs->format->BytesPerPixel;
	result.bytes_per_line = fs->pitch;
	result.pixel = fs->pixels;
	result.width = fs->w;
	result.height = fs->h;
	return &result;
}

void ags_drawCg(cgdata *cg, int x, int y, int brightness, int sprite_color, bool alpha_blend) {
	switch (cg->depth) {
	case 8:
		sdl_drawImage8_fromData(cg, x, y, sprite_color);
		break;
	case 16:
		sdl_drawImage16_fromData(cg, x, y, brightness, alpha_blend);
		break;
	case 24:
		sdl_drawImage24_fromData(cg, x, y, brightness);
		break;
	}
}

void ags_copyArea_shadow(int sx, int sy, int w, int h, int dx, int dy) {
	if (nact->ags.world_depth == 8) return;
	
	if (!ags_check_param(&sx, &sy, &w, &h)) return;
	if (!ags_check_param(&dx, &dy, &w, &h)) return;
	
	sdl_copyAreaSP16_shadow(sx, sy, w, h, dx, dy, 255);
}

void ags_copyArea_transparent(int sx, int sy, int w, int h, int dx, int dy, int col) {
	if (nact->ags.world_depth == 8) return;

	if (!ags_check_param(&sx, &sy, &w, &h)) return;
	if (!ags_check_param(&dx, &dy, &w, &h)) return;
	
	sdl_copyAreaSP(sx, sy, w, h, dx, dy, col);
}

void ags_copyArea_alphaLevel(int sx, int sy, int w, int h, int dx, int dy, int lv) {
	if (nact->ags.world_depth == 8) return;

	if (!ags_check_param(&sx, &sy, &w, &h)) return;
	if (!ags_check_param(&dx, &dy, &w, &h)) return;
	
	sdl_copyAreaSP16_alphaLevel(sx, sy, w, h, dx, dy, lv);
}

void ags_copyArea_alphaBlend(int sx, int sy, int w, int h, int dx, int dy, int lv) {
	if (nact->ags.world_depth == 8) return;

	if (!ags_check_param(&sx, &sy, &w, &h)) return;
	if (!ags_check_param(&dx, &dy, &w, &h)) return;
	
	sdl_copyAreaSP16_alphaBlend(sx, sy, w, h, dx, dy, lv);
}

MyRectangle ags_floodFill(int x, int y, int col) {
	if (!ags_check_param_xy(&x, &y))
		return (MyRectangle){};

	return sdl_floodFill(x, y, col);
}

void ags_copyFromAlpha(int sx, int sy, int w, int h, int dx, int dy, ALPHA_DIB_COPY_TYPE flg) {
	if (nact->ags.world_depth == 8) return;

	if (!ags_check_param(&sx, &sy, &w, &h)) return;
	if (!ags_check_param(&dx, &dy, &w, &h)) return;
	
	sdl_copy_from_alpha(sx, sy, w, h, dx, dy, flg);
}

void ags_copyToAlpha(int sx, int sy, int w, int h, int dx, int dy, ALPHA_DIB_COPY_TYPE flg) {
	if (nact->ags.world_depth == 8) return;

	if (!ags_check_param(&sx, &sy, &w, &h)) return;
	if (!ags_check_param(&dx, &dy, &w, &h)) return;
	
	sdl_copy_to_alpha(sx, sy, w, h, dx, dy, flg);
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

void ags_runEffect(int duration_ms, boolean cancelable, ags_EffectStepFunc step, void *arg) {
	unsigned wflags = cancelable ? KEYWAIT_CANCELABLE : KEYWAIT_NONCANCELABLE;
	int start = sdl_getTicks();
	for (int t = 0; t < duration_ms; t = sdl_getTicks() - start) {
		step(arg, (double)t / duration_ms);
		int key = sys_keywait(start + t + 16 - sdl_getTicks(), wflags);
		if (cancelable && key) {
			nact->waitcancel_key = key;
			break;
		}
	}
	step(arg, 1.0);
}

static void fade(int duration, boolean cancelable, enum sdl_effect_type type) {
	nact->waitcancel_key = 0;

	SDL_Rect rect = {0, 0, nact->ags.view_area.w, nact->ags.view_area.h};
	struct sdl_effect *eff = sdl_effect_init(
		&rect, NULL, 0, 0,
		sdl_getDIB(), nact->ags.view_area.x, nact->ags.view_area.y,
		type);
	ags_runEffect(duration, cancelable, (ags_EffectStepFunc)sdl_effect_step, eff);
	sdl_effect_finish(eff);
}

void ags_fadeIn(int rate, boolean flag) {
	int duration = rate * 16 * 1000 / 60;
	if (!need_update)
		duration = 0;
	fade_outed = FALSE;

	if (nact->ags.world_depth == 8)
		sdl_setPalette(nact->ags.pal, 0, 256);

	fade(duration, flag, nact->ags.world_depth == 8 ? EFFECT_FADEIN : EFFECT_DITHERING_FADEIN);

	sdl_updateAll(&nact->ags.view_area);
}

void ags_fadeOut(int rate, boolean flag) {
	int duration = rate * 16 * 1000 / 60;
	if (!need_update || fade_outed)
		duration = 0;
	fade_outed = TRUE;

	fade(duration, flag, nact->ags.world_depth == 8 ? EFFECT_FADEOUT : EFFECT_DITHERING_FADEOUT);

	if (nact->ags.world_depth == 8) {
		Palette256 pal;
		memset(&pal, 0, sizeof(pal));
		sdl_setPalette(&pal, 0, 256);
	}
}

void ags_whiteIn(int rate, boolean flag) {	
	int duration = rate * 16 * 1000 / 60;
	if (!need_update)
		duration = 0;
	fade_outed = FALSE;

	if (nact->ags.world_depth == 8)
		sdl_setPalette(nact->ags.pal, 0, 256);

	fade(duration, flag, nact->ags.world_depth == 8 ? EFFECT_WHITEIN : EFFECT_DITHERING_WHITEIN);

	sdl_updateAll(&nact->ags.view_area);
}

void ags_whiteOut(int rate, boolean flag) {
	int duration = rate * 16 * 1000 / 60;
	if (!need_update || fade_outed)
		duration = 0;
	fade_outed = TRUE;

	fade(duration, flag, nact->ags.world_depth == 8 ? EFFECT_WHITEIN : EFFECT_DITHERING_WHITEOUT);

	if (nact->ags.world_depth == 8) {
		Palette256 pal;
		memset(&pal, 255, sizeof(pal));
		sdl_setPalette(&pal, 0, 256);
	}
}

void ags_setFont(int type, int size) {
	font_select(type, size);
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
	}
#else
	switch(nact->ags.mouse_movesw) {
	case MOUSE_WARP_DISABLED:
		if (!for_selection)
			sdl_setCursorInternalLocation(x, y);
		break;
	case MOUSE_WARP_DIRECT:
		sdl_setCursorLocation(x, y);
		break;
	case MOUSE_WARP_SMOOTH:
		{
			MyPoint p;
			sys_getMouseInfo(&p, is_dibgeo);
			int dx = x - p.x;
			int dy = y - p.y;
			for (int i = 1; i < 8; i++) {
				int xi = ((dx*i*i*i) >> 9) - ((3*dx*i*i)>> 6) + ((3*dx*i) >> 3) + p.x;
				int yi = ((dy*i*i*i) >> 9) - ((3*dy*i*i)>> 6) + ((3*dy*i) >> 3) + p.y;
				sdl_setCursorLocation(xi, yi);
				usleep(cursor_move_time * 1000 / 8);
			}
			sdl_setCursorLocation(x, y);
		}
		break;
	}
#endif
}

EMSCRIPTEN_KEEPALIVE
void ags_setAntialiasedStringMode(boolean on) {
	if (!nact->ags.noantialias) {
		font_set_antialias(on);
	}
}

boolean ags_getAntialiasedStringMode() {
	return font_get_antialias();
}

void ags_copyArea_shadow_withrate(int sx, int sy, int w, int h, int dx, int dy, int lv) {
	if (nact->ags.world_depth == 8) return;
	
	if (lv == 0) return;
	
	if (!ags_check_param(&sx, &sy, &w, &h)) return;
	if (!ags_check_param(&dx, &dy, &w, &h)) return;
	
	sdl_copyAreaSP16_shadow(sx, sy, w, h, dx, dy, lv);
} 

void ags_setCursorMoveTime(int msec) {
	 cursor_move_time = msec;
}

int ags_getCursorMoveTime() {
	 return cursor_move_time;
}

agsurface_t *ags_getDIB() {
	return nact->ags.dib;
}

void ags_autorepeat(boolean enable) {
	sdl_setAutoRepeat(enable);
}
