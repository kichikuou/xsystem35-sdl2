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

#include <SDL_surface.h>
#include "portab.h"
#include "cg.h"
#include "font.h"

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
	int width;
	int height;
	
	SDL_Surface *sdl_surface; // pixel data (can be NULL)
	uint8_t *alpha; // alpha data (can be NULL)
};
typedef struct agsurface surface_t;

// for surface_t
#define GETOFFSET_PIXEL(suf, x, y) PIXEL_AT((suf)->sdl_surface, x, y)
#define GETOFFSET_ALPHA(suf, x, y) ((suf)->alpha + (y) * (suf)->width + (x))
// for SDL_surface
#define PIXEL_AT(suf, x, y) ((suf)->pixels + (y) * (suf)->pitch + (x) * (suf)->format->BytesPerPixel)
#define ALPHA_AT(suf, x, y) (PIXEL_AT(suf, x, y) + (SDL_BYTEORDER == SDL_LIL_ENDIAN ? 3 : 0))

typedef struct {
	int type;
	int code;
	int mousex, mousey;
} agsevent_t;

enum agsevent_type {
	AGSEVENT_MOUSE_MOTION,
	AGSEVENT_BUTTON_PRESS,
	AGSEVENT_BUTTON_RELEASE,
	AGSEVENT_KEY_PRESS,
	AGSEVENT_KEY_RELEASE,
	AGSEVENT_TIMER,
	AGSEVENT_MOUSE_WHEEL,
};

enum agsevent_button {
	AGSEVENT_BUTTON_LEFT,
	AGSEVENT_BUTTON_MID,
	AGSEVENT_BUTTON_RIGHT,
};

typedef enum {
	TEXT_DECORATION_NONE = 0,
	TEXT_DECORATION_DROP_SHADOW_BOTTOM = 1,
	TEXT_DECORATION_DROP_SHADOW_RIGHT = 2,
	TEXT_DECORATION_DROP_SHADOW_BOTTOM_RIGHT = 3,
	TEXT_DECORATION_OUTLINE = 4,
	TEXT_DECORATION_BOLD_HORIZONTAL = 6,
	TEXT_DECORATION_BOLD_VERTICAL = 7,
	TEXT_DECORATION_BOLD_HORIZONTAL_VERTICAL = 8,
	TEXT_DECORATION_UNDERLINE = 9,
	TEXT_DECORATION_DROP_SHADOW_OUTLINE = 10,
} TextDecorationType;


typedef struct {
	uint8_t r, g, b;
	uint32_t index;
} PixelColor;

typedef struct {
	int width;
	int height;
	int depth;
} DispInfo;

struct _ags {
	SDL_Color pal[256];         /* system palette */
	bool   pal_changed;      /* system palette has changed */

	int world_width;
	int world_height;

	SDL_Rect view_area;      /* view region in off-screen */
	
	int world_depth;            /* depth of off-screen (bits per pixel) */

	surface_t *dib;           /* main surface */
	void (*eventcb)(agsevent_t *e); /* deliver event */

	FontType font_type;
	int font_weight;
	TextDecorationType text_decoration_type;
	int text_decoration_color;

	bool mouse_warp_enabled;
	bool enable_zb;
	bool noantialias; /* antialias を使用しない */
	bool noimagecursor; /* リソースファイルのカーソルを読みこまない */
};
typedef struct _ags ags_t;

extern SDL_Surface *main_surface;

/* 初期化関係 */
void ags_init(const char *render_driver, bool enable_zb);
void ags_remove(void);
void ags_reset(void);

/* ウィンド関係 */
extern void ags_setWorldSize(int width, int height, int depth);
extern void ags_setViewArea(int x, int y, int width, int height);
extern void ags_setWindowTitle(const char *title_utf8);
extern void ags_getDIBInfo(DispInfo *info);
extern void ags_getWindowInfo(DispInfo *info);
extern void ags_getViewAreaInfo(DispInfo *info);
extern bool ags_check_param(int *x, int *y, int *w, int *h);
extern bool ags_check_param_xy(int *x, int *y);
extern surface_t *ags_getDIB();

/* 画面更新 */
extern void ags_setExposeSwitch(bool expose);
extern void ags_updateFull(void);
extern void ags_updateArea(int x, int y, int width, int height);

/* パレット関係 */
extern void ags_setPalettes(SDL_Color *src, int dst, int cnt);
extern void ags_setPalette(int no, int red, int green, int blue);
extern void ags_setPaletteToSystem(int src, int cnt);

/* 描画関係 */
extern void ags_drawRectangle(int x, int y, int w, int h, int col);
extern void ags_fillRectangle(int x, int y, int w, int h, int col);
extern void ags_drawLine(int x0, int y0, int x1, int y1, int col);
extern void ags_copyArea(int sx, int sy, int w, int h, int dx, int dy);
extern void ags_scaledCopyArea(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, int mirror_sw);
extern void ags_copyAreaSP(int sx, int sy, int w, int h, int dx, int dy, int col);
extern void ags_copyArea_shadow_withrate(int sx, int sy, int w, int h, int dx, int dy, int lv);

extern void ags_wrapColor(int x, int y, int w, int h, int p1, int p2);
extern void ags_getPixel(int x, int y, PixelColor *cell);
extern void ags_copyPaletteShift(int sx, int sy, int w, int h, int dx, int dy, uint8_t sprite);
extern void ags_changeColorArea(int x, int y, int w, int h, int dst, int src, int cnt);

extern void* ags_saveRegion(int x, int y, int w, int h);
extern void ags_restoreRegion(void *region, int x, int y);
extern void ags_putRegion(void *region, int x, int y);
extern void ags_delRegion(void *region);

extern int ags_drawString(int x, int y, const char *src, int col, SDL_Rect *rect_out);
extern void ags_drawCg(cgdata *cg, int x, int y, int brightness, int sprite_color, bool alpha_blend);

extern void ags_copyArea_shadow(int sx, int sy, int w, int h, int dx, int dy);
extern void ags_copyArea_transparent(int sx, int sy, int w, int h, int dx, int dy, int col);
extern void ags_copyArea_alphaLevel(int sx, int sy, int w, int h, int dx, int dy, int lv);
extern void ags_copyArea_alphaBlend(int sx, int sy, int w, int h, int dx, int dy, int lv);
extern SDL_Rect ags_floodFill(int x, int y, int col);
extern void ags_eCopyArea(int sx, int sy, int w, int h, int dx, int dy, int sw, int opt, bool cancel);
extern void ags_eSpriteCopyArea(int sx, int sy, int w, int h, int dx, int dy, int sw, int opt, bool cancel, int spCol);

/* alpha channel 操作 */
extern void ags_copyFromAlpha(int sx, int sy, int w, int h, int dx, int dy, ALPHA_DIB_COPY_TYPE flg);
extern void ags_copyToAlpha(int sx, int sy, int w, int h, int dx, int dy, ALPHA_DIB_COPY_TYPE flg);
extern void ags_alpha_uppercut(int sx, int sy, int w, int h, int s, int d);
extern void ags_alpha_lowercut(int sx, int sy, int w, int h, int s, int d);
extern void ags_alpha_setLevel(int x, int y, int w, int h, int lv);
extern void ags_alpha_copyArea(int sx, int sy, int w, int h, int dx, int dy);
extern void ags_alpha_getPixel(int x, int y, int *pic);
extern void ags_alpha_setPixel(int x, int y, int w, int h, uint8_t *b);

/* fader */
extern void ags_fadeIn(int rate, bool flg);
extern void ags_fadeOut(int rate, bool flg);
extern void ags_whiteIn(int rate, bool flg);
extern void ags_whiteOut(int rate, bool flg);

/* フォント関連 */
extern void ags_setFont(FontType type, int size);
extern void ags_setFontWithWeight(FontType type, int size, int weight);
extern void ags_setTextDecorationType(TextDecorationType type);
extern void ags_setTextDecorationColor(int color);
extern SDL_Surface *ags_drawStringToSurface(const char *str, int r, int g, int b);

/* カーソル関係 */
extern void ags_setCursorType(int type);
extern void ags_loadCursor(int ,int);
extern void ags_setCursorLocation(int x, int y, bool is_dibgeo, bool for_selection);
extern void ags_setCursorMoveTime(int msec);
extern int  ags_getCursorMoveTime();

/* misc */
extern void    ags_setAntialiasedStringMode(bool mode);
extern bool ags_getAntialiasedStringMode();
extern void    ags_autorepeat(bool enable);
extern bool ags_clipCopyRect(const SDL_Rect *sr, const SDL_Rect *dr, int *sx, int *sy, int *dx, int *dy, int *w, int *h);

typedef void (*ags_EffectStepFunc)(void *, float);
void ags_runEffect(int duration_ms, bool cancelable, ags_EffectStepFunc step, void *arg);

#define RMASK16 0xf800
#define GMASK16 0x07e0
#define BMASK16 0x001f
#define RMASK24 0x00ff0000
#define GMASK24 0x0000ff00
#define BMASK24 0x000000ff

#define PIXR16(pic) (uint8_t)(((pic) & RMASK16) >> 8)
#define PIXG16(pic) (uint8_t)(((pic) & GMASK16) >> 3)
#define PIXB16(pic) (uint8_t)(((pic) & BMASK16) << 3)
#define PIX16(r,g,b) (uint16_t)((((r) & 0xf8) << 8) | (((g) & 0xfc) << 3) | ((b       ) >> 3))

#define PIXR24(pic) (uint8_t)(((pic) & RMASK24) >> 16)
#define PIXG24(pic) (uint8_t)(((pic) & GMASK24) >>  8)
#define PIXB24(pic) (uint8_t)(((pic) & BMASK24)      )
#define PIX24(r,g,b) (uint32_t)((((r) << 16) | ((g) << 8) | (b)))

#define ALPHABLEND16(f, b, a)  (PIX16((((PIXR16((f)) - PIXR16((b))) * (a)) >> 8)+ PIXR16((b)),\
                                      (((PIXG16((f)) - PIXG16((b))) * (a)) >> 8)+ PIXG16((b)),\
                                      (((PIXB16((f)) - PIXB16((b))) * (a)) >> 8)+ PIXB16((b))))

#define ALPHABLEND24(f, b, a)  (PIX24((((PIXR24((f)) - PIXR24((b))) * (a)) >> 8) + PIXR24((b)),\
                                      (((PIXG24((f)) - PIXG24((b))) * (a)) >> 8) + PIXG24((b)),\
                                      (((PIXB24((f)) - PIXB24((b))) * (a)) >> 8) + PIXB24((b))))

#define SUTURADD24(pa, pb) PIX24(min(255,PIXR24(pa)+PIXR24(pb)), min(255, PIXG24(pa)+PIXG24(pb)), min(255, PIXB24(pa)+PIXB24(pb)));

#endif /* !__AGS_H__ */
