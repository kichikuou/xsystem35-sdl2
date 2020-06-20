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
#include "portab.h"
#include "ags.h"
#include "cursor.h"

/* 初期化関係 */
extern int  sdl_Initilize(void);
extern void sdl_Remove(void);

/* ウィンド関係 */
extern void sdl_setWorldSize(int width, int height, int depth);
extern void sdl_setWindowSize(int x, int y, int w, int h);
extern void sdl_setWindowTitle(char *name);
extern void sdl_getWindowInfo(DispInfo *info);
extern void sdl_FullScreen(boolean on);
extern agsurface_t *sdl_getDIB(void);

/* 画面更新 */
extern void sdl_updateArea(MyRectangle *src, MyPoint *dst);
extern void sdl_fullScreen(boolean on);
extern void sdl_updateScreen(void);

/* パレット関係 */
extern void sdl_setPallet(Pallet256 *pal, int src, int cnt);

/* 描画関係 */
extern void sdl_drawRectangle(int x, int y, int w, int h, int cl);
extern void sdl_fillRectangle(int x, int y, int w, int h, unsigned long c);
extern void sdl_drawLine(int x1, int y1, int x2, int y2, unsigned long col);
extern int  sdl_drawString(int x, int y, const char *msg, unsigned long col);
extern void sdl_copyArea(int sx,int sy, int w, int h, int dx, int dy);
extern void sdl_drawTT(int x,int y,int w,int h,const char *bitmap,int ww, boolean antialiased);
extern void sdl_copyAreaSP(int sx, int sy, int w, int h, int dx, int dy, int sp);
extern void sdl_drawImage8_fromData(cgdata *cg, int x, int y, int w, int h);
extern void sdl_Mosaic(int sx, int sy, int w, int h, int dx, int dy, int slice);
extern void sdl_wrapColor(int sx, int sy, int w, int h, int cl, int rate);
extern void sdl_scaledCopyArea(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, int mirror);
extern void sdl_zoom(int x, int y, int w, int h);
extern void sdl_drawImage16_fromData(cgdata *cg, int x, int y, int w, int h);
extern void sdl_copyAreaSP16_shadow(int sx, int sy, int w, int h, int dx, int dy, int lv);
extern void sdl_copyAreaSP16_alphaBlend(int sx, int sy, int w, int h, int dx, int dy, int lv);
extern void sdl_copyAreaSP16_alphaLevel(int sx, int sy, int w, int h, int dx, int dy, int lv);
extern void sdl_copyAreaSP16_whiteLevel(int sx, int sy, int w, int h, int dx, int dy, int lv);
extern void sdl_copy_from_alpha(int sx, int sy, int w, int h, int dx, int dy, ALPHA_DIB_COPY_TYPE flag);
extern void sdl_copy_to_alpha(int sx, int sy, int w, int h, int dx, int dy, ALPHA_DIB_COPY_TYPE flag);
extern void sdl_getPixel(int x, int y, Pallet *cell);
extern void sdl_putRegion(void *src, int x, int y);
extern void sdl_CopyRegion(void *src, int sx, int sy, int w, int h, int dx, int dy);
extern void sdl_restoreRegion(void *src, int x, int y);
extern void* sdl_saveRegion(int x, int y, int w, int h);
extern void sdl_delRegion(void *src);
extern void sdl_maskupdate(int sx, int sy, int w, int h, int dx, int dy, int func, int step);

/* フォント関連 */
extern void sdl_setFontDevice(FONT *f);

/* fader */
extern void sdl_fadeIn(int step);
extern void sdl_fadeOut(int step);
extern void sdl_whiteIn(int step);
extern void sdl_whiteOut(int step);

/* key/pointer 関係 */
extern int  sdl_keywait(int msec, boolean cancel);
extern void sdl_keywait_post(void);
extern void sdl_setCursorLocation(int x, int y);
extern void sdl_setCursorType(int type);
extern boolean sdl_cursorNew(BYTE* data, int no, CursorImage *cursorImage,  TCursorDirEntry *cursordirentry);
extern int  sdl_getKeyInfo();
extern int  sdl_getMouseInfo(MyPoint *p);
extern int  sdl_getjoyinfo(void);
extern void sdl_setAutoRepeat(boolean bool);

/* misc */
extern void sdl_mainIteration();
extern boolean RawKeyInfo[];
extern void sdl_sleep(int msec);
extern void sdl_wait_vsync();

/* 初期化関係 */
#define GraphicsInitilize() sdl_Initilize()
#define GraphicsRemove() sdl_Remove()

/* ウィンド関係 */
#define GetWindowInfo(info) sdl_getWindowInfo(info)
#define SetWindowSize(x,y,w,h) sdl_setWindowSize((x),(y),(w),(h))
#define SetWindowTitle(size) sdl_setWindowTitle((size))
#define SetWorldSize(w,h,d) sdl_setWorldSize((w),(h),(d))
#define GetDIB() sdl_getDIB()

/* 画面更新 */
#define DspDeviceSync()
#define UpdateArea(src,dst) sdl_updateArea((src),(dst))
#define FullScreen(on) sdl_FullScreen(on)

/* パレット関係 */
#define SetPallet(pal,src,cnt) sdl_setPallet((pal),(src),(cnt))

/* 描画関係 */
#define DrawString(x,y,str,col) sdl_drawString((x),(y),(str),(col))
#define FillRectangle(x,y,w,h,col) sdl_fillRectangle((x),(y),(w),(h),(col))
#define CopyArea(sx,sy,w,h,dx,dy) sdl_copyArea((sx),(sy),(w),(h),(dx),(dy))
#define CopyAreaSP(sx,sy,w,h,dx,dy,col) sdl_copyAreaSP((sx),(sy),(w),(h),(dx),(dy),(col)) 
#define DrawLine(x0,y0,x1,y1,col) sdl_drawLine((x0),(y0),(x1),(y1),(col))
#define ScaledCopyArea(sx, sy, sw, sh, dx, dy, dw, dh, mirror_sw) sdl_scaledCopyArea(sx, sy, sw, sh, dx, dy, dw, dh, mirror_sw)
#define Zoom(x,y,w,h) sdl_zoom(x,y,w,h)
#define WrapColor(x,y,w,h,col,rate) sdl_wrapColor((x),(y),(w),(h),(col),(rate))
#define GetPixel(x, y, cell) sdl_getPixel(x, y, cell)
#define DrawImage8_fromData(info,x,y,w,h) sdl_drawImage8_fromData(info,x,y,w,h)
#define DrawImage16_fromData(info,x,y,w,h) sdl_drawImage16_fromData(info,x,y,w,h)
#define CopyAreaSP16_shadow(sx,sy,w,h,dx,dy) sdl_copyAreaSP16_shadow(sx,sy,w,h,dx,dy,255)
#define CopyAreaSP16_shadow_withRate(sx,sy,w,h,dx,dy,lv) sdl_copyAreaSP16_shadow(sx,sy,w,h,dx,dy,lv)
#define CopyAreaSP16_alphaLevel(sx, sy, w, h, dx, dy, lv) sdl_copyAreaSP16_alphaLevel(sx, sy, w, h, dx, dy, lv)
#define CopyAreaSP16_alphaBlend(sx, sy, w, h, dx, dy, lv) sdl_copyAreaSP16_alphaBlend(sx, sy, w, h, dx, dy, lv)
#define CopyAreaSP16_whiteLevel(sx, sy, w, h, dx, dy, lv) sdl_copyAreaSP16_whiteLevel(sx, sy, w, h, dx, dy, lv)
#define Copy_from_alpha(sx, sy, w, h, dx, dy, flg) sdl_copy_from_alpha(sx, sy, w, h, dx, dy, flg)
#define Copy_to_alpha(sx, sy, w, h, dx, dy, flg) sdl_copy_to_alpha(sx, sy, w, h, dx, dy, flg)
#define DrawRectangle(x,y,w,h,col) sdl_drawRectangle((x),(y),(w),(h),(col))
#define Mosaic(sx,sy,w,h,dx,dy,sl) sdl_Mosaic((sx),(sy),(w),(h),(dx),(dy),(sl))
#define RestoreRegion(img,x,y) sdl_restoreRegion((img),(x),(y));
#define SaveRegion(x,y,w,h) sdl_saveRegion(x,y,w,h)
#define DelRegion(i) sdl_delRegion(i)
#define PutRegion(i,x,y) sdl_putRegion(i,x,y)
#define CopyRegion(i,sx,sy,w,h,dx,dy) sdl_CopyRegion(i,sx,sy,w,h,dx,dy)
#define Maskupdate(sx,sy,w,h,dx,dy,f,st) sdl_maskupdate(sx,sy,w,h,dx,dy,f,st)

/* フォント関連 */
#define SetFontDevice(f) sdl_setFontDevice(f)

/* fader 関連 */
#define FadeOut sdl_fadeOut
#define FadeIn sdl_fadeIn
#define WhiteIn sdl_whiteIn
#define WhiteOut sdl_whiteOut

/* key/pointer 関係 */
#define GetKeyInfo() sdl_getKeyInfo()
#define SetCursorType(t) sdl_setCursorType((t))
#define GetMouseInfo(info) sdl_getMouseInfo(info)
#define Keywait(r,flg) sdl_keywait(r,flg)
#define SetCursorLocation(x,y) sdl_setCursorLocation((x),(y))
#define CursorNew(d,no,i,dir) sdl_cursorNew((d),(no),(i),(dir))
#define SetAutoRepeat(b) sdl_setAutoRepeat(b)

/* misc */
#define ResourceInit(c,v) sdl_ResourceInit((c),(v))
#define SetNoShmMode() /* NO */
#define Sleep(ms) sdl_sleep(ms)
#define WaitVsync() sdl_wait_vsync()

#endif /* !__SDL_CORE__ */
