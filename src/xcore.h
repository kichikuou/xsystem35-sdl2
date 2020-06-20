/*
 * xcore.h  Xlibとの通信
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
/* $Id: xcore.h,v 1.42 2003/04/22 16:34:28 chikama Exp $ */

#ifndef __XCORE_H__
#define __XCORE_H__

#include "config.h"
#include <sys/types.h>
#include "portab.h"
#include "image.h"
#include "xcore_image.h"
#include "font.h"
#include "cursor.h"
#include "ags.h"

/* 初期化関係 */
extern int  Xcore_Initilize(void);
extern void Xcore_Remove(void);

/* ウィンド関係 */
extern void Xcore_setWorldSize(int width, int height, int depth);
extern void Xcore_setWindowSize(int x, int y, int width, int height);
extern void Xcore_setWindowTitle(char *name);
extern void Xcore_getWindowInfo(DispInfo *info);
extern void Xcore_fullScreen(boolean on);
extern agsurface_t *Xcore_getDIB();

/* 画面更新 */
extern void Xcore_updateArea(MyRectangle *src, MyPoint *dst);
extern void Xcore_sync();

/* パレット関係 */
extern void Xcore_setPallet(Pallet256 *pal, int src, int cnt);
extern void Xcore_setForeground(unsigned long col);

/* 描画関係 */
extern void Xcore_drawRectangle(int x, int y, int w, int h, unsigned long col);
extern void Xcore_fillRectangle(int x, int y, int w, int h, unsigned long col);
extern void Xcore_drawLine(int x0, int y0, int x1, int y1, unsigned long col);
extern int  Xcore_drawString(int x, int y, const char *msg, unsigned long col);
extern void Xcore_copyArea(int sx, int sy, int w, int h, int dx, int dy);
// extern void Xcore_drawTT(int x, int y, int w, int h, const char *bitmap, int ww, boolean antialiased);
extern int  Xcore_eCopyArea(int sx, int sy, int w, int h, int dx, int dy, int t,int opt, boolean flg, int spCol);
extern void Xcore_zoom(int x, int y, int w, int h);
extern void Xcore_restoreRegion(void *img, int x, int y);
extern void Xcore_delRegion(void *img);
extern void Xcore_putRegion(void *img, int x, int y);
extern void *Xcore_saveRegion(int x, int y, int w, int h);
extern void Xcore_copyRegion(void *i, int sx, int sy, int w, int h, int dx, int dy);
extern void Xcore_maskupdate(int sx, int sy, int w, int h, int dx, int dy, int func, int step);

/* フォント関連 */
// extern FONT *Xcore_getFontDevice(void);
extern void Xcore_setFontDevice(FONT *f);

/* fader */
extern void Xcore_fadeIn(int step);
extern void Xcore_fadeOut(int step);
extern void Xcore_whiteIn(int step);
extern void Xcore_whiteOut(int step);

/* key/pointer 関係 */
extern int  Xcore_getMouseInfo(MyPoint *p);
extern int  Xcore_getKeyInfo();
extern void Xcore_setCursorLocation(int x, int y);
extern boolean Xcore_cursorNew(BYTE* data, int no, CursorImage *cursorImage,  TCursorDirEntry *cursordirentry);
extern void Xcore_setCursorType(int type);
extern int  Xcore_keywait(int ms, boolean cancel);
extern void Xcore_setAutoRepeat(boolean bool);

/* misc */
// extern void Xcore_mainloop(int (*idle_func)(void));
extern void Xcore_eventCallnack();
extern void Xcore_setNoShmMode();
extern void Xcore_mainIterarion();
extern boolean   RawKeyInfo[256];

/* 初期化関係 */
#define GraphicsInitilize() Xcore_Initilize()
#define GraphicsRemove() Xcore_Remove()

/* ウィンド関係 */
#define GetWindowInfo(info) Xcore_getWindowInfo(info)
#define SetWorldSize(w,h,d) Xcore_setWorldSize((w),(h),(d))
#define SetWindowSize(x,y,w,h) Xcore_setWindowSize((x),(y),(w),(h))
#define SetWindowTitle(size) Xcore_setWindowTitle((size))
#define GetDIB() Xcore_getDIB()

/* 画面更新 */
#define DspDeviceSync() Xcore_sync()
#define UpdateArea(src,dst) Xcore_updateArea((src),(dst))
#define FullScreen(on) Xcore_fullScreen(on)

/* パレット関係 */
#define SetPallet(pal,src,cnt) Xcore_setPallet((pal),(src),(cnt))
#define SetForeground(col) Xcore_setForeground((col))

/* 描画関係 */
#define DrawString(x,y,str,col) Xcore_drawString((x),(y),(str),(col))
#define FillRectangle(x,y,w,h,col) Xcore_fillRectangle((x),(y),(w),(h),(col))
#define CopyArea(sx,sy,w,h,dx,dy) Xcore_copyArea((sx),(sy),(w),(h),(dx),(dy)) 
#define CopyAreaSP(sx,sy,w,h,dx,dy,col) image_copyAreaSP(nact->ags.dib, (sx),(sy),(w),(h),(dx),(dy),(col))
#define DrawLine(x0,y0,x1,y1,col) Xcore_drawLine((x0),(y0),(x1),(y1),(col))
#define DrawTT(x,y,w,h,bm,ww,anti) Xcore_drawTT((x),(y),(w),(h),(bm),(ww),(anti))
#define ScaledCopyArea(sx,sy,sw,sh,dx,dy,dw,dh,mirror_sw) image_scaledCopyArea(NULL,nact->ags.dib,sx,sy,sw,sh,dx,dy,dw,dh,mirror_sw)
#define Zoom(x,y,w,h) Xcore_zoom(x,y,w,h)
#define WrapColor(x,y,w,h,p1,p2) image_wrapColor(nact->ags.dib,x,y,w,h,p1,p2)
#define GetPixel(x,y,cell) image_getPixel(nact->ags.dib,x,y,cell)
#define DrawImage8_fromData(info,x,y,w,h) image_drawImage8_fromData(nact->ags.dib, info,x,y,w,h)
#define DrawImage16_fromData(info,x,y,w,h) image_drawImage16_fromData(nact->ags.dib, info,x,y,w,h)
#define CopyAreaSP16_shadow(sx,sy,w,h,dx,dy) image_copyAreaSP16_shadow(nact->ags.dib,sx,sy,w,h,dx,dy,255)
#define CopyAreaSP16_shadow_withRate(sx,sy,w,h,dx,dy,lv) image_copyAreaSP16_shadow(nact->ags.dib,sx,sy,w,h,dx,dy,lv)
#define CopyAreaSP16_alphaLevel(sx,sy,w,h,dx,dy,lv) image_copyAreaSP16_alphaLevel(nact->ags.dib,sx,sy,w,h,dx,dy,lv)
#define CopyAreaSP16_alphaBlend(sx,sy,w,h,dx,dy,lv) image_copyAreaSP16_alphaBlend(nact->ags.dib,sx,sy,w,h,dx,dy,lv)
#define CopyAreaSP16_whiteLevel(sx,sy,w,h,dx,dy,lv) image_copyAreaSP16_whiteLevel(nact->ags.dib,sx,sy,w,h,dx,dy,lv)
#define Copy_from_alpha(sx,sy,w,h,dx,dy,flg) image_copy_from_alpha(nact->ags.dib,sx,sy,w,h,dx,dy,flg)
#define Copy_to_alpha(sx,sy,w,h,dx,dy,flg) image_copy_to_alpha(nact->ags.dib,sx,sy,w,h,dx,dy,flg)
#define ChangeColorArea(x,y,w,h,s,d,c) Xcore_changeColorArea(x,y,w,h,s,d,c)
#define DrawRectangle(x,y,w,h,col) Xcore_drawRectangle((x),(y),(w),(h),(col))
#define Mosaic(sx,sy,w,h,dx,dy,sl) image_Mosaic(nact->ags.dib,(sx),(sy),(w),(h),(dx),(dy),(sl))
#define RestoreRegion(img,x,y) image_restoreRegion(nact->ags.dib,(img),(x),(y))
#define SaveRegion(x,y,w,h) image_saveRegion(nact->ags.dib,(x),(y),(w),(h))
#define DelRegion(i) image_delRegion(i)
#define PutRegion(i,x,y) image_putRegion(nact->ags.dib,i,x,y)
#define CopyRegion(i,sx,sy,w,h,dx,dy) image_copyRegion(i,sx,sy,w,h,nact->ags.dib,dx,dy)
#define Maskupdate(sx,sy,w,h,dx,dy,f,st) Xcore_maskupdate(sx,sy,w,h,dx,dy,f,st)

/* フォント関連 */
#define SetFontDevice(f) Xcore_setFontDevice(f)

/* fader 関連 */
#define FadeOut  Xcore_fadeOut
#define FadeIn   Xcore_fadeIn
#define WhiteIn  Xcore_whiteIn
#define WhiteOut Xcore_whiteOut

/* key/pointer 関係 */
#define GetKeyInfo() Xcore_getKeyInfo()
#define SetCursorType(t) Xcore_setCursorType((t))
#define GetMouseInfo(info) Xcore_getMouseInfo(info)
#define Keywait(r,flg) Xcore_keywait(r,flg)
#define SetCursorLocation(x,y) Xcore_setCursorLocation((x),(y))
#define CursorNew(d,no,i,dir) Xcore_cursorNew((d),(no),(i),(dir))
#define SetAutoRepeat(b) Xcore_setAutoRepeat(b);

/* misc */
// #define Mainloop(ptr) Xcore_mainloop(ptr)
#define SetNoShmMode Xcore_setNoShmMode
// #define MainIteration() Xcore_mainIteration()
#define EventCallback Xcore_eventCallback
#define Sleep(ms) usleep((ms) * 1000)
#define WaitVsync() Sleep(16)

#endif /* !__XCORE_H__ */
