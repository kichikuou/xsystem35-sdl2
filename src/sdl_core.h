/*
 * sdl_core.h  SDL acess wrapper
 *
 * Copyright (C) 2000-     Fumihiko Murata       <fmurata@p1.tcnet.ne.jp>
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
/* $Id: sdl_core.h,v 1.18 2003/01/04 17:01:02 chikama Exp $ */

#ifndef __SDL_CORE__
#define __SDL_CORE__

#include "config.h"
#include <sys/types.h>
#include <SDL_events.h>
#include <SDL_surface.h>
#include "portab.h"
#include "ags.h"
#include "cursor.h"
#include "effect.h"

struct inputstring_param;

/* 初期化関係 */
extern int sdl_Initialize(const char *render_driver);
extern void sdl_Remove(void);

/* ウィンド関係 */
extern void sdl_setWorldSize(int width, int height, int depth);
extern void sdl_setWindowSize(int w, int h);
extern void sdl_setWindowTitle(char *name);
extern void sdl_getWindowInfo(DispInfo *info);
extern void sdl_setFullscreen(bool on);
extern bool sdl_isFullscreen(void);
extern void sdl_raiseWindow(void);
extern surface_t *sdl_getDIB(void);
extern void sdl_setIntegerScaling(bool enable);
extern SDL_Surface *sdl_createSurfaceView(SDL_Surface *sf, int x, int y, int w, int h);

/* 画面更新 */
extern void sdl_updateArea(MyRectangle *src, MyPoint *dst);
extern void sdl_updateAll(MyRectangle *view_rect);
extern void sdl_updateScreen(void);

/* パレット関係 */
extern void sdl_setPalette(SDL_Color *pal, int first, int count);

/* 描画関係 */
extern void sdl_drawRectangle(int x, int y, int w, int h, uint8_t c);
extern void sdl_fillRectangle(int x, int y, int w, int h, uint8_t c);
extern void sdl_fillRectangleRGB(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b);
extern void sdl_fillCircle(int left, int top, int diameter, uint8_t c);
extern void sdl_drawLine(int x1, int y1, int x2, int y2, uint8_t c);
extern SDL_Rect sdl_floodFill(int x, int y, int col);
extern SDL_Rect sdl_drawString(int x, int y, const char *str_utf8, uint8_t col);
extern void sdl_copyArea(int sx,int sy, int w, int h, int dx, int dy);
extern void sdl_copyAreaSP(int sx, int sy, int w, int h, int dx, int dy, uint8_t sp);
extern void sdl_wrapColor(int sx, int sy, int w, int h, uint8_t cl, int rate);
extern void sdl_scaledCopyArea(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, int mirror);
extern void sdl_drawImage8(cgdata *cg, int x, int y, int sprite_color);
extern void sdl_drawImage16(cgdata *cg, surface_t *sf, int x, int y, int brightness, bool alpha_blend);
extern void sdl_drawImage24(cgdata *cg, surface_t *sf, int x, int y, int brightness);
extern void sdl_drawImageAlphaMap(cgdata *cg, surface_t *sf, int x, int y);
extern void sdl_copyAreaSP16_shadow(int sx, int sy, int w, int h, int dx, int dy, int lv);
extern void sdl_copyAreaSP16_alphaBlend(int sx, int sy, int w, int h, int dx, int dy, int lv);
extern void sdl_copyAreaSP16_alphaLevel(int sx, int sy, int w, int h, int dx, int dy, int lv);
extern void sdl_copy_from_alpha(int sx, int sy, int w, int h, int dx, int dy, ALPHA_DIB_COPY_TYPE flag);
extern void sdl_copy_to_alpha(int sx, int sy, int w, int h, int dx, int dy, ALPHA_DIB_COPY_TYPE flag);
extern void sdl_getPixel(int x, int y, Palette *cell);
extern void sdl_putRegion(void *src, int x, int y);
extern void sdl_restoreRegion(void *src, int x, int y);
extern void* sdl_saveRegion(int x, int y, int w, int h);
extern void sdl_delRegion(void *src);
void sdl_FlipSurfaceHorizontal(SDL_Surface *s);
void sdl_FlipSurfaceVertical(SDL_Surface *s);

/* Effects */
struct sdl_effect;
struct sdl_effect *sdl_effect_init(SDL_Rect *rect, surface_t *old, int ox, int oy, surface_t *new, int nx, int ny, enum sdl_effect_type effect);
struct sdl_effect *sdl_sprite_effect_init(SDL_Rect *rect, int dx, int dy, int sx, int sy, int col, enum sdl_effect_type type);
struct sdl_effect *sdl_effect_magnify_init(surface_t *surface, SDL_Rect *view_rect, SDL_Rect *target_rect);
struct sdl_effect *sdl_effect_sactamask_init(SDL_Surface *mask);
void sdl_effect_step(struct sdl_effect *eff, float progress);
void sdl_effect_finish(struct sdl_effect *eff);

/* key/pointer 関係 */
extern void sdl_setJoyDeviceIndex(int index);
extern void sdl_setCursorLocation(int x, int y);
extern void sdl_setCursorInternalLocation(int x, int y);
extern void sdl_setCursorType(int type);
extern bool sdl_cursorNew(uint8_t* data, int no, CursorImage *cursorImage,  TCursorDirEntry *cursordirentry);
extern int  sdl_getKeyInfo();
extern int  sdl_getMouseInfo(MyPoint *p);
extern void sdl_getWheelInfo(int *forward, int *back);
extern void sdl_clearWheelInfo(void);
extern int  sdl_getJoyInfo(void);
extern void sdl_setAutoRepeat(bool enable);
extern MyPoint sdl_translateMouseCoords(int x, int y);

/* misc */
enum messagebox_type {
	MESSAGEBOX_ERROR,
	MESSAGEBOX_WARNING,
	MESSAGEBOX_INFO,
};
extern uint32_t sdl_getTicks(void);
extern void sdl_sleep(int msec);
extern void sdl_wait_vsync();
extern void sdl_showMessageBox(enum messagebox_type type, const char* title_utf8, const char* message_utf8);
extern bool sdl_inputString(struct inputstring_param *);
extern void sdl_post_debugger_command(void *data);
extern void sdl_handle_event(SDL_Event *e);

extern bool save_screenshot(const char* path);

#endif /* !__SDL_CORE__ */
