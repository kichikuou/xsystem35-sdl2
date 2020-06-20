/*
 * xcore_draw.c  X11 draw to DIB
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
/* $Id: xcore_draw.c,v 1.7 2004/10/31 04:18:06 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef X_LOCALE
#include <X11/Xlocale.h>
#else
#include <locale.h>
#endif

#include <X11/extensions/XShm.h>

#include "portab.h"
#include "font.h"
#include "ags.h"
#include "image.h"
#include "nact.h"
#include "xcore.h"
#include "xcore_private.h"

static void Xcore_updateAll();
static void rect_comimg(agsurface_t *src, agsurface_t *dst, int sx, int sy, int sw, int sh);
static void workImageToWindow();
static void DIBToWorkImage(int sx, int sy, int w, int h, int dx, int dy);


/* 全画面更新 */
static void Xcore_updateAll() {
	MyRectangle src;
	MyPoint     dst;
	
	src.x      = view_x;
	src.y      = view_y;
	src.width  = min(DIB->width, view_w);
	src.height = min(DIB->height, view_h);
	dst.x = dst.y = 0;
	Xcore_updateArea(&src, &dst);
}

/* offscreenの指定領域をWindowへ転送 */
void Xcore_updateArea(MyRectangle *src, MyPoint *dst) {
	agsurface_t _dib, _work;
	
	if (x11_dibinfo->shared) {
		XCopyArea(x11_display, x11_pixmap, x11_window, x11_gc_pix,
			  src->x, src->y, src->width, src->height,
			  winoffset_x + dst->x, winoffset_y + dst->y);
	} else {
		if (DIB_DEPTH == 8) {
			if (WIN_DEPTH == 8) {
				XPutImage(x11_display, x11_window,
					  x11_gc_pix, x11_dibinfo->ximg,
					  src->x, src->y,
					  winoffset_x + dst->x, winoffset_y + dst->y,
					  src->width, src->height);
			} else {
				rect_comimg(DIB, &_dib,
					    src->x, src->y, src->width, src->height);
				rect_comimg(WORK, &_work,
					    dst->x, dst->y, src->width, src->height);
				
				image_expandPixel(&_dib, &_work, 255);
				
				if (x11_workinfo->shared) {
					XShmPutImage(x11_display, x11_window,
						     x11_gc_win, x11_workinfo->ximg,
						     dst->x, dst->y,
						     winoffset_x + dst->x, winoffset_y + dst->y,
						     src->width, src->height, False);
				} else {
					XPutImage(x11_display, x11_window,
						  x11_gc_win, x11_workinfo->ximg,
						  dst->x, dst->y,
						  winoffset_x + dst->x, winoffset_y + dst->y,
						  src->width, src->height);
				}
			}
		} else {
			if (packed24bpp) {
				rect_comimg(DIB, &_dib,
					    src->x, src->y, src->width, src->height);
				rect_comimg(WORK, &_work,
					    dst->x, dst->y, src->width, src->height);
				
				image_trans_pixel_24to24p(&_dib, &_work);
				
				XPutImage(x11_display, x11_window,
					  x11_gc_win, x11_workinfo->ximg,
					  dst->x, dst->y,
					  winoffset_x + dst->x, winoffset_y + dst->y,
					  src->width, src->height);
			} else {
				XPutImage(x11_display, x11_window, 
					  x11_gc_pix, x11_dibinfo->ximg,
					  src->x, src->y,
					  winoffset_x + dst->x, winoffset_y + dst->y,
					  src->width, src->height);
			}
		}
	}
	
	XSync(x11_display, False); x11_needSync = False;
}

/* draw rectangle */
void Xcore_drawRectangle(int x, int y, int w, int h, unsigned long col) {
	if (!x11_dibinfo->shared) {
		image_drawRectangle(DIB, x, y, w, h, PAL2PIC(col));
	} else {
		Xcore_setForeground(col);
		XDrawRectangle(x11_display, x11_pixmap, x11_gc_pix, x, y, w - 1, h - 1);
		x11_needSync = TRUE;
	}
}

/* fill rectangle */
void Xcore_fillRectangle(int x, int y, int w, int h, unsigned long col) {
	if (!x11_dibinfo->shared) {
		image_fillRectangle(DIB, x, y, w, h, PAL2PIC(col));
	} else {
		Xcore_setForeground(col);
		XFillRectangle(x11_display, x11_pixmap, x11_gc_pix, x, y, w, h);
		x11_needSync = TRUE;
	}
}	

/* copy area */
void Xcore_copyArea(int sx, int sy, int w, int h, int dx, int dy) {
	if (!x11_dibinfo->shared) {
		image_copyArea(DIB, sx, sy, w, h, dx, dy);
	} else {
		XCopyArea(x11_display, x11_pixmap, x11_pixmap, x11_gc_pix,
			  sx, sy, w, h, dx, dy);
		x11_needSync = TRUE;
	}
}

/* 直線描画 */
void Xcore_drawLine(int x1, int y1, int x2, int y2, unsigned long col) {
	if (!x11_dibinfo->shared) {
		image_drawLine(DIB, x1, y1, x2, y2, PAL2PIC(col));
	} else {
		Xcore_setForeground(col);
		XDrawLine(x11_display, x11_pixmap, x11_gc_pix, x1, y1, x2, y2);
		x11_needSync = TRUE;
	}
} 

int Xcore_drawString(int x, int y, const char *msg, unsigned long col) {
	int w;
	
	if (x11_font->self_drawable()) {
		w = x11_font->draw_glyph(x, y, msg, col);
		x11_needSync = TRUE;
	} else {
		agsurface_t *glyph = x11_font->get_glyph(msg);
		
		if (glyph == NULL) return 0;
		
		if (x11_font->antialiase_on && DIB_DEPTH != 8) {
			if (x > 0) x -= 1;
			if (y > 0) y -= 1;
			image_draw_antialiased_pattern(DIB, glyph,
						       x, y,
						       glyph->width +2, glyph->height +2,
						       PAL2PIC(col));
		} else {
			image_draw_pattern(DIB, glyph,
					   x, y,
					   glyph->width, glyph->height,
					   PAL2PIC(col));
		}
		w = glyph->width;
	}
	
	return w;
}

void Xcore_zoom(int x, int y, int w, int h) {
	if (DIB_DEPTH == WIN_DEPTH) {
		agsurface_t _dib, _work;
		rect_comimg(WORK, &_work, 0, 0, view_w, view_h);
		rect_comimg(DIB, &_dib, x, y, w, h);
		image_scaledCopyArea(DIB, &_work, x, y, w, h,
				     0, 0, view_w, view_h, 0);
		workImageToWindow();
	} else {
		/* TODO */
	}
}

/* Color の複数個指定 */
void Xcore_setPallet(Pallet256 *pal, int src, int cnt) {
	int i;
	
	if (x11_visual->class == PseudoColor) {
		for (i = 0; i < cnt; i++) {
			x11_col[src + i].pixel = src + i;
                        x11_col[src + i].flags = DoRed | DoGreen | DoBlue;
 			x11_col[src + i].red   = pal->red  [src + i] * 257;
			x11_col[src + i].green = pal->green[src + i] * 257;
			x11_col[src + i].blue  = pal->blue [src + i] * 257;
		}
		XStoreColors(x11_display, x11_cm, &x11_col[src], cnt);
	}
}

/* foreground のセット */
void Xcore_setForeground(unsigned long col) {
	static unsigned long fc = 0;
	
	if (fc == col && x11_visual->class == PseudoColor) return;
	
	fc = col;
	
	if (x11_visual->class != PseudoColor) {
		XSetForeground(x11_display, x11_gc_pix, PAL2PIC(col));
	} else {
		XSetForeground(x11_display, x11_gc_pix, col);
	}
}

/* background のセット */
void Xcore_setBackground(unsigned long col) {
	static unsigned long bc = 0;
	
	if (bc == col) return;
	
	bc = col;
	
	if (x11_visual->class != PseudoColor) {
		XSetBackground(x11_display, x11_gc_win, PAL2PIC(col));
	} else {
		XSetBackground(x11_display, x11_gc_win, col);
	}
}

/* X-Serverとの同期を取る */
void Xcore_sync() {
	if (x11_needSync) { XSync(x11_display, False); x11_needSync = FALSE; }
}

static void rect_comimg(agsurface_t *src, agsurface_t *dst, int sx, int sy, int sw, int sh) {
	memcpy(dst, src, sizeof(agsurface_t));
	
	//dst->x      = sx;
	//dst->y      = sy;
	dst->width  = sw;
	dst->height = sh;
	dst->pixel  = GETOFFSET_PIXEL(src, sx, sy);
}

/* fader で workImage の内容をWindowに描画 */
static void workImageToWindow() {
	if (x11_workinfo->shared) {
		XSync(x11_display, False);
		XShmPutImage(x11_display, x11_window,
			     x11_gc_win, x11_workinfo->ximg,
			     0, 0,
			     winoffset_x, winoffset_y,
			     view_w, view_h, False);
	} else {
		XPutImage(x11_display, x11_window,
			  x11_gc_win, x11_workinfo->ximg,
			  0, 0,
			  winoffset_x, winoffset_y,
			  view_w, view_h);
	}
}

/* DIB から work image への展開 */
static void DIBToWorkImage(int sx, int sy, int w, int h, int dx, int dy) {
	if (DIB_DEPTH == dib_depth_candidate) {
		int y;
		BYTE *src = GETOFFSET_PIXEL(&(x11_dibinfo->cimg), sx, sy);
		BYTE *dst = GETOFFSET_PIXEL(&(x11_workinfo->cimg), dx, dy);
		for (y = 0; y < h; y++) {
			memcpy(dst, src, w * x11_dibinfo->cimg.bytes_per_pixel);
			src += x11_dibinfo->cimg.bytes_per_line;
			dst += x11_workinfo->cimg.bytes_per_line;
		}
	} else {
		agsurface_t _dib, _work;
		rect_comimg(DIB, &_dib, sx, sy, w, h);
		rect_comimg(WORK, &_work, dx, dy, w, h);
		image_expandPixel(&_dib, &_work, 255);
	}
}

/* 指定の明度以上に明るさを設定 */
/* (255-col)/255 * val + col */
static void setWhiteness8(int val) {
	int i;
	Pallet256 *pal = nact->sys_pal;

	for (i = 0; i < 256; i++) {
		x11_col[i].pixel = i;
		x11_col[i].flags = DoRed | DoGreen | DoBlue;
 		x11_col[i].red   = ((((255 - pal->red[i])   * val) / 256) + pal->red[i])   * 257;
		x11_col[i].green = ((((255 - pal->green[i]) * val) / 256) + pal->green[i]) * 257;
		x11_col[i].blue  = ((((255 - pal->blue[i])  * val) / 256) + pal->blue[i])  * 257;
	}
	XStoreColors(x11_display, x11_cm, x11_col, 256);
}

/* 指定の明度以下に明るさを抑える */
static void setBlightness8(int val) {
	int i;
	Pallet256 *pal = nact->sys_pal;
	
	for (i =0; i < 256; i++) {
		x11_col[i].pixel = i;
		x11_col[i].flags = DoRed | DoGreen | DoBlue;
 		x11_col[i].red   = (val * pal->red[i]   / 255) * 257;
		x11_col[i].green = (val * pal->green[i] / 255) * 257;
		x11_col[i].blue  = (val * pal->blue[i]  / 255) * 257;
	}
	XStoreColors(x11_display, x11_cm, x11_col, 256);
}

static int fadestep[256] =
{0,1,3,4,6,7,9,10,12,14,15,17,18,20,21,23,25,26,28,29,31,32,34,36,37,39,40,
 42,43,45,46,48,49,51,53,54,56,57,59,60,62,63,65,66,68,69,71,72,74,75,77,78,
 80,81,83,84,86,87,89,90,92,93,95,96,97,99,100,102,103,105,106,108,109,110,
 112,113,115,116,117,119,120,122,123,124,126,127,128,130,131,132,134,135,136,
 138,139,140,142,143,144,146,147,148,149,151,152,153,155,156,157,158,159,161,
 162,163,164,166,167,168,169,170,171,173,174,175,176,177,178,179,181,182,183,
 184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,
 203,204,205,206,207,208,209,210,211,211,212,213,214,215,216,217,217,218,219,
 220,221,221,222,223,224,225,225,226,227,227,228,229,230,230,231,232,232,233,
 234,234,235,235,236,237,237,238,238,239,239,240,241,241,242,242,243,243,244,
 244,244,245,245,246,246,247,247,247,248,248,249,249,249,250,250,250,251,251,
 251,251,252,252,252,252,253,253,253,253,254,254,254,254,254,254,255,255,255,
 255,255,255,255,255,255,255,255,255,255,255};

static void fadein_8_n8(int step) {
	static agsurface_t _dib, _work;
	
	if (step == 0) {
		rect_comimg(DIB, &_dib, view_x, view_y, view_w, view_h);
		rect_comimg(WORK, &_work, 0, 0, view_w, view_h);
	}
	image_expandPixel(&_dib, &_work, fadestep[step]);
	workImageToWindow();
}

static void fadein_n8(int step) {
	static agsurface_t _dib, _work;
	
	if (step == 0) {
		rect_comimg(DIB, &_dib, view_x, view_y, view_w, view_h);
		rect_comimg(WORK, &_work, 0, 0, view_w, view_h);
		memset(_work.pixel, 0, x11_workinfo->cimg.bytes_per_pixel * view_w * view_h);
	}
	if (step == 255) {
		ags_updateFull();
		return;
	}
	image_fadeIn(&_dib, &_work, step /16);
	workImageToWindow();
}

static void fadeout_8_n8(int step) {
	static agsurface_t _dib, _work;
	
	if (step == 0) {
		rect_comimg(DIB, &_dib, view_x, view_y, view_w, view_h);
		rect_comimg(WORK, &_work, 0, 0, view_w, view_h);
	}
	if (step == 255) {
		Xcore_setBackground(0);
		XClearWindow(x11_display, x11_window);
		return;
	}
	image_expandPixel(&_dib, &_work, fadestep[255 - step]);
	workImageToWindow();
}

static void fadeout_n8(int step) {
	static agsurface_t _work;
	
	if (step == 0) {
		DIBToWorkImage(view_x, view_y, view_w, view_h, 0, 0);
		rect_comimg(WORK, &_work, 0, 0, view_w, view_h);
	}
	if (step == 255) {
		Xcore_setBackground(0);
		XClearWindow(x11_display, x11_window);
		return;
	}
	image_fadeOut(&_work, (255-step)/16, 0);
	workImageToWindow();
}

static void whitein_8_n8(int step) {
	static agsurface_t _dib, _work;
	
	if (step == 0) {
		rect_comimg(DIB, &_dib, view_x, view_y, view_w, view_h);
		rect_comimg(WORK, &_work, 0, 0, view_w, view_h);
	}
	image_expandPixel(&_dib, &_work, fadestep[255-step]+255);
	workImageToWindow();
}

static void whitein_n8(int step) {
	static agsurface_t _dib, _work;
	
	if (step == 0) {
		rect_comimg(DIB, &_dib, view_x, view_y, view_w, view_h);
		rect_comimg(WORK, &_work, 0, 0, view_w, view_h);
	}
	if (step == 255) {
		ags_updateFull();
		return;
	}
	image_fadeIn(&_dib, &_work, (255-step)/16);
	workImageToWindow();
}

static void whiteout_8_n8(int step) {
	static agsurface_t _dib, _work;
	
	if (step == 0) {
		rect_comimg(DIB, &_dib, view_x, view_y, view_w, view_h);
		rect_comimg(WORK, &_work, 0, 0, view_w, view_h);
	}
	image_expandPixel(&_dib, &_work, fadestep[step]+255);
	workImageToWindow();
}

static void whiteout_n8(int step) {
	static agsurface_t _work;
	
	if (step == 0) {
		DIBToWorkImage(view_x, view_y, view_w, view_h, 0, 0);
		rect_comimg(WORK, &_work, 0, 0, view_w, view_h);
	}
	if (step == 255) {
		memset(x11_workinfo->cimg.pixel, 255, x11_workinfo->cimg.bytes_per_pixel * view_w * view_h);
		workImageToWindow();
		return;
	}
	image_fadeOut(&_work, step/16, 0xffffffff);
	workImageToWindow();
}

void Xcore_fadeIn(int step) {
	
	if (step == 0) {
		Xcore_sync();
	}
	
	if (DIB_DEPTH == 8) {
		if (WIN_DEPTH == 8) {
			if (step == 0) Xcore_updateAll();
			setBlightness8(fadestep[step]);
		} else {
			fadein_8_n8(step);
		}
	} else {
		fadein_n8(step);
	}
}

void Xcore_fadeOut(int step) {
	
	if (step == 0) {
		Xcore_sync();
	}
	
	if (DIB_DEPTH == 8) {
		if (WIN_DEPTH == 8) {
			setBlightness8(fadestep[255-step]);
		} else {
			fadeout_8_n8(step);
		}
	} else {
		fadeout_n8(step);
	}
}

void Xcore_whiteIn(int step) {
	
	if (step == 0) {
		Xcore_sync();
	}
	
	if (DIB_DEPTH == 8) {
		if (WIN_DEPTH == 8) {
			if (step == 0) Xcore_updateAll();
			setWhiteness8(fadestep[step]);
		} else {
			whitein_8_n8(step);
		}
	} else {
		whitein_n8(step);
	}
}

void Xcore_whiteOut(int step) {
	
	if (step == 0) {
		Xcore_sync();
	}
	
	if (DIB_DEPTH == 8) {
		if (WIN_DEPTH == 8) {
			setWhiteness8(fadestep[255-step]);
		} else {
			whiteout_8_n8(step);
		}
	} else {
		whiteout_n8(step);
	}
}

Pixmap x11_clip_from_DIB(int sx, int sy, int w, int h) {
	agsurface_t _dib, _work;
	Pixmap clippix;

	clippix = XCreatePixmap(x11_display, x11_window, w, h, WIN_DEPTH);
	
	if (x11_dibinfo->shared) {
		XCopyArea(x11_display, x11_pixmap, clippix, x11_gc_pix,
			  sx, sy, w, h, 0, 0);
	} else {
		if (DIB_DEPTH == 8) {
			if (WIN_DEPTH == 8) {
				XPutImage(x11_display, clippix,
					  x11_gc_pix, x11_dibinfo->ximg,
					  sx, sy, 0, 0, w, h);
			} else {
				rect_comimg(DIB, &_dib,
					    sx, sy, w, h);
				rect_comimg(WORK, &_work,
					    0, 0, w, h);
				
				image_expandPixel(&_dib, &_work, 255);
				
				if (x11_workinfo->shared) {
					XShmPutImage(x11_display, clippix,
						     x11_gc_win, x11_workinfo->ximg,
						     0, 0,
						     0, 0,
						     sx, sy, False);
				} else {
					XPutImage(x11_display, clippix,
						  x11_gc_win, x11_workinfo->ximg,
						  0, 0,
						  0, 0,
						  w, h);
				}
			}
		} else {
			if (packed24bpp) {
				rect_comimg(DIB, &_dib,
					    sx, sy, w, h);
				rect_comimg(WORK, &_work,
					    0, 0, w, h);
				
				image_trans_pixel_24to24p(&_dib, &_work);
				
				XPutImage(x11_display, clippix,
					  x11_gc_win, x11_workinfo->ximg,
					  0, 0,
					  0, 0,
					  w, h);
			} else {
				XPutImage(x11_display, clippix, 
					  x11_gc_pix, x11_dibinfo->ximg,
					  sx, sy,
					  0, 0,
					  w, h);
			}
		}
	}
	
	return clippix;
}
