/*
 * Copyright (C) 2000-     Fumihiko Murata       <fmurata@p1.tcnet.ne.jp>
 *               2025 kichikuou <KichikuouChrome@gmail.com>
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
#ifndef __GFX_H__
#define __GFX_H__

#include "config.h"
#include <SDL_surface.h>
#include "portab.h"
#include "ags.h"

/* 初期化関係 */
int gfx_Initialize(const char *render_driver);
void gfx_Remove(void);

/* ウィンド関係 */
void gfx_setWorldSize(int width, int height, int depth);
void gfx_setWindowSize(int w, int h);
void gfx_setWindowTitle(char *name);
void gfx_getWindowInfo(int *width, int *height, int *depth);
void gfx_setFullscreen(bool on);
bool gfx_isFullscreen(void);
void gfx_raiseWindow(void);
surface_t *gfx_getDIB(void);
void gfx_setIntegerScaling(bool enable);
SDL_Surface *gfx_createSurfaceView(SDL_Surface *sf, int x, int y, int w, int h);

/* 画面更新 */
void gfx_updateArea(SDL_Rect *src, SDL_Point *dst);
void gfx_updateAll(SDL_Rect *view_rect);
void gfx_updateScreen(void);

/* パレット関係 */
void gfx_setPalette(SDL_Color *pal, int first, int count);

/* 描画関係 */
void gfx_drawRectangle(int x, int y, int w, int h, uint8_t c);
void gfx_fillRectangle(int x, int y, int w, int h, uint8_t c);
void gfx_fillRectangleRGB(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b);
void gfx_fillCircle(int left, int top, int diameter, uint8_t c);
void gfx_drawLine(int x1, int y1, int x2, int y2, uint8_t c);
SDL_Rect gfx_floodFill(int x, int y, int col);
SDL_Rect gfx_drawString(int x, int y, const char *str_utf8, uint8_t col);
void gfx_copyArea(int sx,int sy, int w, int h, int dx, int dy);
void gfx_copyAreaSP(int sx, int sy, int w, int h, int dx, int dy, uint8_t sp);
void gfx_wrapColor(int sx, int sy, int w, int h, uint8_t cl, int rate);
void gfx_scaledCopyArea(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, int mirror);
void gfx_drawImage8(cgdata *cg, int x, int y, int sprite_color);
void gfx_drawImage16(cgdata *cg, surface_t *sf, int x, int y, int brightness, bool alpha_blend);
void gfx_drawImage24(cgdata *cg, surface_t *sf, int x, int y, int brightness);
void gfx_drawImageAlphaMap(cgdata *cg, surface_t *sf, int x, int y);
void gfx_copyAreaSP16_shadow(int sx, int sy, int w, int h, int dx, int dy, int lv);
void gfx_copyAreaSP16_alphaBlend(int sx, int sy, int w, int h, int dx, int dy, int lv);
void gfx_copyAreaSP16_alphaLevel(int sx, int sy, int w, int h, int dx, int dy, int lv);
void gfx_copy_from_alpha(int sx, int sy, int w, int h, int dx, int dy, ALPHA_DIB_COPY_TYPE flag);
void gfx_copy_to_alpha(int sx, int sy, int w, int h, int dx, int dy, ALPHA_DIB_COPY_TYPE flag);
uint32_t gfx_getPixel(int x, int y);
void gfx_putRegion(void *src, int x, int y);
void gfx_restoreRegion(void *src, int x, int y);
void* gfx_saveRegion(int x, int y, int w, int h);
void gfx_delRegion(void *src);
void gfx_FlipSurfaceHorizontal(SDL_Surface *s);
void gfx_FlipSurfaceVertical(SDL_Surface *s);

bool save_screenshot(const char* path);

#endif /* __GFX_H__ */
