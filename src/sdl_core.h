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
#include <SDL_surface.h>
#include "portab.h"
#include "ags.h"
#include "cursor.h"

struct inputstring_param;

/* 初期化関係 */
extern int  sdl_Initilize(void);
extern void sdl_Remove(void);

/* ウィンド関係 */
extern void sdl_setWorldSize(int width, int height, int depth);
extern void sdl_setWindowSize(int x, int y, int w, int h);
extern void sdl_setWindowTitle(char *name);
extern void sdl_getWindowInfo(DispInfo *info);
extern void sdl_setFullscreen(boolean on);
extern boolean sdl_isFullscreen(void);
extern agsurface_t *sdl_getDIB(void);
extern void sdl_setIntegerScaling(boolean enable);

/* 画面更新 */
extern void sdl_updateArea(MyRectangle *src, MyPoint *dst);
extern void sdl_updateScreen(void);

/* パレット関係 */
extern void sdl_setPalette(Palette256 *pal, int src, int cnt);

/* 描画関係 */
extern void sdl_drawRectangle(int x, int y, int w, int h, BYTE c);
extern void sdl_fillRectangle(int x, int y, int w, int h, BYTE c);
extern void sdl_drawLine(int x1, int y1, int x2, int y2, BYTE c);
extern SDL_Rect sdl_floodFill(int x, int y, int col);
extern SDL_Rect sdl_drawString(int x, int y, const char *str_utf8, BYTE col);
extern void sdl_copyArea(int sx,int sy, int w, int h, int dx, int dy);
extern void sdl_drawTT(int x,int y,int w,int h,const char *bitmap,int ww, boolean antialiased);
extern void sdl_copyAreaSP(int sx, int sy, int w, int h, int dx, int dy, BYTE sp);
extern void sdl_drawImage8_fromData(cgdata *cg, int x, int y, int w, int h);
extern void sdl_Mosaic(int sx, int sy, int w, int h, int dx, int dy, int slice);
extern void sdl_wrapColor(int sx, int sy, int w, int h, BYTE cl, int rate);
extern void sdl_scaledCopyArea(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, int mirror);
extern void sdl_zoom(int x, int y, int w, int h);
extern void sdl_drawImage16_fromData(cgdata *cg, int x, int y, int w, int h);
extern void sdl_copyAreaSP16_shadow(int sx, int sy, int w, int h, int dx, int dy, int lv);
extern void sdl_copyAreaSP16_alphaBlend(int sx, int sy, int w, int h, int dx, int dy, int lv);
extern void sdl_copyAreaSP16_alphaLevel(int sx, int sy, int w, int h, int dx, int dy, int lv);
extern void sdl_copyAreaSP16_whiteLevel(int sx, int sy, int w, int h, int dx, int dy, int lv);
extern void sdl_copy_from_alpha(int sx, int sy, int w, int h, int dx, int dy, ALPHA_DIB_COPY_TYPE flag);
extern void sdl_copy_to_alpha(int sx, int sy, int w, int h, int dx, int dy, ALPHA_DIB_COPY_TYPE flag);
extern void sdl_getPixel(int x, int y, Palette *cell);
extern void sdl_putRegion(void *src, int x, int y);
extern void sdl_CopyRegion(void *src, int sx, int sy, int w, int h, int dx, int dy);
extern void sdl_restoreRegion(void *src, int x, int y);
extern void* sdl_saveRegion(int x, int y, int w, int h);
extern void sdl_delRegion(void *src);
extern void sdl_maskupdate(int sx, int sy, int w, int h, int dx, int dy, int func, int step);
extern SDL_Surface *com2surface(agsurface_t *s);

/* fader */
extern void sdl_fadeIn(int step);
extern void sdl_fadeOut(int step);
extern void sdl_whiteIn(int step);
extern void sdl_whiteOut(int step);

/* key/pointer 関係 */
extern void sdl_setJoyDeviceIndex(int index);
extern void sdl_setCursorLocation(int x, int y);
extern void sdl_setCursorType(int type);
extern boolean sdl_cursorNew(BYTE* data, int no, CursorImage *cursorImage,  TCursorDirEntry *cursordirentry);
extern int  sdl_getKeyInfo();
extern int  sdl_getMouseInfo(MyPoint *p);
extern int  sdl_getJoyInfo(void);
extern void sdl_setAutoRepeat(boolean enable);
extern MyPoint sdl_translateMouseCoords(int x, int y);

/* misc */
extern void sdl_mainIteration();
extern boolean RawKeyInfo[];
extern uint32_t sdl_getTicks(void);
extern void sdl_sleep(int msec);
extern void sdl_wait_vsync();
extern boolean sdl_inputString(struct inputstring_param *);

#endif /* !__SDL_CORE__ */
