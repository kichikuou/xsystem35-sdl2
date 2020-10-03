/*
 * ags.h  Alice Graphic System
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
/* $Id: ags.h,v 1.25 2004/10/31 04:18:06 chikama Exp $ */

#ifndef __AGS_H__
#define __AGS_H__

#include "config.h"

#include "portab.h"
#include "cg.h"
#include "graphics.h"

/* マウスカーソルの種類 */
#define CURSOR_ARROW     1
#define CURSOR_CROSS     2
#define CURSOR_IBEAM     3
#define CURSOR_ICON      4
#define CURSOR_NO        5
#define CURSOR_SIZE      6
#define CURSOR_SIZEALL   7
#define CURSOR_SIZENESW  8
#define CURSOR_SIZENS    9
#define CURSOR_SIZENWSE 10
#define CURSOR_SIZEWE   11
#define CURSOR_UPARROW  12
#define CURSOR_WAIT     13

/* RGB <-> alpha plane copy type */
typedef enum {
	FROM_16H,
	FROM_16L,
	FROM_24R,
	FROM_24G,
	FROM_24B,
	TO_16H,
	TO_16L,
	TO_24R,
	TO_24G,
	TO_24B
} ALPHA_DIB_COPY_TYPE;

struct agsurface {
	int width;   /* width of surface  */
	int height;  /* height of surface */
	int depth;   /* depth of surface, 8/16/24/32 is available */
	
	int bytes_per_line;   /* bytes per line  */
	int bytes_per_pixel;  /* bytes per pixel */
	
	BYTE *pixel; /* pixel data (can be NULL) */
	BYTE *alpha; /* alpha pixel data (can be NULL) */
};
typedef struct agsurface agsurface_t;

// for agsurface_t
#define GETOFFSET_PIXEL(suf, x, y) ((suf)->pixel + (y) * (suf)->bytes_per_line + (x) * (suf)->bytes_per_pixel)
#define GETOFFSET_ALPHA(suf, x, y) ((suf)->alpha + (y) * (suf)->width + (x))
// for SDL_surface
#define PIXEL_AT(suf, x, y) ((suf)->pixels + (y) * (suf)->pitch + (x) * (suf)->format->BytesPerPixel)

struct _agsevent {
	int type;
	int d1, d2, d3;
};
typedef struct _agsevent agsevent_t;


#define	AGSEVENT_MOUSE_MOTION 1
#define	AGSEVENT_BUTTON_PRESS 2
#define	AGSEVENT_BUTTON_RELEASE 3
#define AGSEVENT_KEY_PRESS 4
#define AGSEVENT_KEY_RELEASE 5
#define AGSEVENT_TIMER 6

#define AGSEVENT_BUTTON_LEFT  1
#define AGSEVENT_BUTTON_MID   2
#define AGSEVENT_BUTTON_RIGHT 3
#define AGSEVENT_WHEEL_UP 4
#define AGSEVENT_WHEEL_DN 5


/*
 * fader 管理情報
 */
struct ags_faderinfo {
	int step_max;    /* 最大 step 数 */
	int effect_time; /* 全体の処理にかける時間 */
	
	boolean cancel;  /* 途中で key 抜けをうけつけるか */
	void (*callback)(int); /* callback 関数 */
};
typedef struct ags_faderinfo ags_faderinfo_t;


struct _ags {
	Palette256 pal;             /* system palette */
	boolean   pal_changed;      /* system palette has changed */
	
	MyDimension world_size;     /* size of off-screen */

	MyRectangle view_area;      /* view region in off-screen */
	
	int world_depth;            /* depth of off-screen (bits per pixel) */

	int mouse_movesw;           /* mouse cursor move mode
				       0: ignore IZ
				       1: move to the geometory direcly
				       2: move to the geometory smoothly
				    */

	boolean fullscree_is_on;    /* if full-screen mode then true */
	
	agsurface_t *dib;           /* main surface */
	void (*eventcb)(agsevent_t *e); /* deliver event */
};
typedef struct _ags ags_t;



/* 初期化関係 */
extern void ags_init();
extern void ags_remove();

/* ウィンド関係 */
extern void ags_setWorldSize(int width, int height, int depth);
extern void ags_setViewArea(int x, int y, int width, int height);
extern void ags_setWindowTitle(const char *str);
extern void ags_getDIBInfo(DispInfo *info);
extern void ags_getWindowInfo(DispInfo *info);
extern void ags_getViewAreaInfo(DispInfo *info);
extern boolean ags_regionContains(MyRectangle *r, int x, int y);
extern void    ags_fullscreen(boolean on);
extern boolean ags_check_param(int *x, int *y, int *w, int *h);
extern boolean ags_check_param_xy(int *x, int *y);
extern void    ags_intersection(MyRectangle *r1, MyRectangle *r2, MyRectangle *rst);
extern agsurface_t *ags_getDIB();

/* 画面更新 */
extern void ags_setExposeSwitch(boolean bool);
extern void ags_updateFull(void);
extern void ags_updateArea(int x, int y, int width, int height);

/* パレット関係 */
extern void ags_setPalettes(Palette256 *src_pal, int src, int dst, int cnt);
extern void ags_setPalette(int no, int red, int green, int blue);
extern void ags_setPaletteToSystem(int src, int cnt);

/* 描画関係 */
extern void ags_drawRectangle(int x, int y, int w, int h, int col);
extern void ags_fillRectangle(int x, int y, int w, int h, int col);
extern void ags_drawLine(int x0, int y0, int x1, int y1, int col);
extern void ags_copyArea(int sx, int sy, int w, int h, int dx, int dy);
extern void ags_scaledCopyArea(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, int mirror_sw);
extern void ags_zoom(int x, int y, int w, int h);
extern void ags_copyAreaSP(int sx, int sy, int w, int h, int dx, int dy, int col);
extern void ags_copyArea_shadow_withrate(int sx, int sy, int w, int h, int dx, int dy, int lv);

extern void ags_wrapColor(int x, int y, int w, int h, int p1, int p2);
extern void ags_getPixel(int x, int y, Palette *cell);
extern void ags_changeColorArea(int x, int y, int w, int h, int dst, int src, int cnt);

extern void* ags_saveRegion(int x, int y, int w, int h);
extern void ags_restoreRegion(void *region, int x, int y);
extern void ags_putRegion(void *region, int x, int y);
extern void ags_copyRegion(void *region, int sx, int sy, int w,int h,int dx,int dy);
extern void ags_delRegion(void *region);

extern int  ags_drawString(int x, int y, const char *src, int col);
extern void ags_drawCg8bit(cgdata *cg, int x, int y); 
extern void ags_drawCg16bit(cgdata *cg, int x, int y); 

extern void ags_copyArea_shadow(int sx, int sy, int w, int h, int dx, int dy);
extern void ags_copyArea_transparent(int sx, int sy, int w, int h, int dx, int dy, int col);
extern void ags_copyArea_alphaLevel(int sx, int sy, int w, int h, int dx, int dy, int lv);
extern void ags_copyArea_alphaBlend(int sx, int sy, int w, int h, int dx, int dy, int lv);
extern void ags_copyArea_whiteLevel(int sx, int sy, int w, int h, int dx, int dy, int lv);
extern MyRectangle* ags_imageFlood(int x, int y, int c);
extern void ags_eCopyArea(int sx, int sy, int w, int h, int dx, int dy, int type, int opt, boolean flg, int spCol);

/* alpha channel 操作 */
extern void ags_copyFromAlpha(int sx, int sy, int w, int h, int dx, int dy, ALPHA_DIB_COPY_TYPE flg);
extern void ags_copyToAlpha(int sx, int sy, int w, int h, int dx, int dy, ALPHA_DIB_COPY_TYPE flg);
extern void ags_alpha_uppercut(int sx, int sy, int w, int h, int s, int d);
extern void ags_alpha_lowercut(int sx, int sy, int w, int h, int s, int d);
extern void ags_alpha_setLevel(int x, int y, int w, int h, int lv);
extern void ags_alpha_copyArea(int sx, int sy, int w, int h, int dx, int dy);
extern void ags_alpha_getPixel(int x, int y, int *pic);
extern void ags_alpha_setPixel(int x, int y, int w, int h, BYTE *b);

/* fader */
extern void ags_fadeIn(int rate, boolean flg);
extern void ags_fadeOut(int rate, boolean flg);
extern void ags_whiteIn(int rate, boolean flg);
extern void ags_whiteOut(int rate, boolean flg);
extern void ags_fader_callback();

/* フォント関連 */
enum FontType {
	FONT_GOTHIC,
	FONT_MINCHO,
};
extern void ags_setFont(int type, int size);
extern agsurface_t *ags_drawStringToSurface(const char *str);

/* カーソル関係 */
extern void ags_setCursorType(int type);
extern void ags_loadCursor(int ,int);
extern void ags_setCursorLocation(int x, int y, boolean dibgeo);
extern void ags_setCursorMoveTime(int msec);
extern int  ags_getCursorMoveTime();

/* misc */
extern void    ags_setAntialiasedStringMode(boolean mode);
extern boolean ags_getAntialiasedStringMode();
extern void    ags_fader(ags_faderinfo_t *);
extern void    ags_autorepeat(boolean bool);

#define RMASK16 0xf800
#define GMASK16 0x07e0
#define BMASK16 0x001f
#define RMASK24 0x00ff0000
#define GMASK24 0x0000ff00
#define BMASK24 0x000000ff

#define PIXR16(pic) (BYTE)(((pic) & RMASK16) >> 8)
#define PIXG16(pic) (BYTE)(((pic) & GMASK16) >> 3)
#define PIXB16(pic) (BYTE)(((pic) & BMASK16) << 3)
#define PIX16(r,g,b) (WORD)((((r) & 0xf8) << 8) | (((g) & 0xfc) << 3) | ((b       ) >> 3))

#define PIXR24(pic) (BYTE)(((pic) & RMASK24) >> 16)
#define PIXG24(pic) (BYTE)(((pic) & GMASK24) >>  8)
#define PIXB24(pic) (BYTE)(((pic) & BMASK24)      )
#define PIX24(r,g,b) (DWORD)((((r) << 16) | ((g) << 8) | (b)      ))

#define ALPHABLEND16(f, b, a)  (PIX16((((PIXR16((f)) - PIXR16((b))) * (a)) >> 8)+ PIXR16((b)),\
                                      (((PIXG16((f)) - PIXG16((b))) * (a)) >> 8)+ PIXG16((b)),\
                                      (((PIXB16((f)) - PIXB16((b))) * (a)) >> 8)+ PIXB16((b))))

#define ALPHALEVEL16(p, lv) PIX16(((PIXR16(p) * (lv)) >> 8),\
								  ((PIXG16(p) * (lv)) >> 8),\
								  ((PIXB16(p) * (lv)) >> 8))

#define WHITELEVEL16(p, lv) ALPHABLEND16(0xffff,p,lv)

#define ALPHABLEND24(f, b, a)  (PIX24((((PIXR24((f)) - PIXR24((b))) * (a)) >> 8) + PIXR24((b)),\
                                      (((PIXG24((f)) - PIXG24((b))) * (a)) >> 8) + PIXG24((b)),\
                                      (((PIXB24((f)) - PIXB24((b))) * (a)) >> 8) + PIXB24((b))))

#define ALPHALEVEL24(p, lv) (PIX24(((PIXR24(p) * (lv)) >> 8),\
                                   ((PIXG24(p) * (lv)) >> 8),\
                                   ((PIXB24(p) * (lv)) >> 8)))

#define WHITELEVEL24(p, lv) ALPHABLEND24(0xffffffff, p, lv)

#define SUTURADD16(pa, pb) PIX16(min(255,PIXR16(pa)+PIXR16(pb)), min(255, PIXG16(pa)+PIXG16(pb)), min(255, PIXB16(pa)+PIXB16(pb)));
//#define SUTURADD16(pa, pb) PIX16(min(255,(int)(PIXR16(pa))+(int)(PIXR16(pb))), min(255, (int)(PIXG16(pa))+(int)(PIXG16(pb))), min(255, (int)(PIXB16(pa))+(int)(PIXB16(pb))));
#define SUTURADD24(pa, pb) PIX24(min(255,PIXR24(pa)+PIXR24(pb)), min(255, PIXG24(pa)+PIXG24(pb)), min(255, PIXB24(pa)+PIXB24(pb)));


/* exter methods */
extern void ablend16_dpd(BYTE *, int, BYTE *, int, int, int, int, int);
extern void ablend16_ppd(BYTE *, BYTE *, BYTE *, int, int, int, int, int, int);
extern void ablend16_ppp(BYTE *, BYTE *, BYTE *, BYTE *, int, int, int, int, int, int, int);

#endif /* !__AGS_H__ */
