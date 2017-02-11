/*
 * sdl_darw.c  SDL draw to surface
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
/* $Id: sdl_draw.c,v 1.13 2003/01/25 01:34:50 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
#include <glib.h>

#include "portab.h"
#include "system.h"
#include "sdl_core.h"
#include "sdl_private.h"
#include "font.h"
#include "ags.h"
#include "image.h"
#include "nact.h"

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

static SDL_Surface *s_fader;  /* fade in /out 用 work surface */

static void sdl_pal_check(void) {
	if (nact->sys_pal_changed) {
		nact->sys_pal_changed = FALSE;
		sdl_setPallet(nact->sys_pal, 0, 256);
	}
}

/* off-screen の指定領域を Main Window へ転送 */
void sdl_updateArea(MyRectangle *src, MyPoint *dst) {
	SDL_Rect rect_s, rect_d;
	
	setRect(rect_s, src->x, src->y, src->width, src->height);
	setRect(rect_d, winoffset_x + dst->x, winoffset_y + dst->y, src->width, src->height);
	
	SDL_BlitSurface(sdl_dib, &rect_s, sdl_display, &rect_d);
	
	SDL_UpdateRect(sdl_display, winoffset_x + dst->x, winoffset_y + dst->y,
		       src->width, src->height);
}

/* 全画面更新 */
static void sdl_updateAll() {
	SDL_Rect rect;

	setRect(rect, winoffset_x, winoffset_y, view_w, view_h);
	
	SDL_BlitSurface(sdl_dib, &sdl_view, sdl_display, &rect);
	
	SDL_UpdateRect(sdl_display, 0, 0, 0, 0);

}

/* Color の複数個指定 */
void sdl_setPallet(Pallet256 *pal, int src, int cnt) {
	int i;
	
	for (i = 0; i < cnt; i++) {
		sdl_col[src + i].r = pal->red  [src + i];
		sdl_col[src + i].g = pal->green[src + i];
		sdl_col[src + i].b = pal->blue [src + i];
	}
	
	if (sdl_dib->format->BitsPerPixel == 8) {
		SDL_SetColors(sdl_dib, sdl_col, src, cnt);
	}
}

/* 矩形の描画 */
void sdl_drawRectangle(int x, int y, int w, int h, int c) {
	SDL_Rect rect;
	
	sdl_pal_check();
	
	if (c < 256 && sdl_dib->format->BitsPerPixel > 8)
		c = SDL_MapRGB(sdl_dib->format, sdl_col[c].r, sdl_col[c].g, sdl_col[c].b);

	setRect(rect,x,y,w,1);
	SDL_FillRect(sdl_dib, &rect, c);
	
	setRect(rect,x,y,1,h);
	SDL_FillRect(sdl_dib, &rect, c);
	
	setRect(rect,x,y+h-1,w,1);
	SDL_FillRect(sdl_dib, &rect, c);
	
	setRect(rect,x+w-1,y,1,h);
	SDL_FillRect(sdl_dib, &rect, c);
}

/* 矩形塗りつぶし */
void sdl_fillRectangle(int x, int y, int w, int h, u_long c) {
	SDL_Rect rect;
	
	sdl_pal_check();
	
	setRect(rect,x,y,w,h);
	
	if (c < 256 && sdl_dib->format->BitsPerPixel > 8)
		c = SDL_MapRGB(sdl_dib->format, sdl_col[c].r, sdl_col[c].g, sdl_col[c].b);
	
	SDL_FillRect(sdl_dib, &rect, c);
}

/* 領域コピー */
void sdl_copyArea(int sx,int sy, int w, int h, int dx, int dy) {
	SDL_Rect r_src, r_dst;
	
	setRect(r_src, sx, sy, w, h);
	setRect(r_dst, dx, dy, w, h);

	SDL_BlitSurface(sdl_dib, &r_src, sdl_dib, &r_dst);
}

/*
 * dib に指定のパレット sp を抜いてコピー
 */
void sdl_copyAreaSP(int sx, int sy, int w, int h, int dx, int dy, int sp) {
	SDL_Rect r_src, r_dst;

	sdl_pal_check();
	
	if (sdl_dib->format->BitsPerPixel > 8 && sp < 256) {
		sp = SDL_MapRGB(sdl_dib->format,
				sdl_col[sp].r & 0xf8,
				sdl_col[sp].g & 0xfc,
				sdl_col[sp].b & 0xf8);
	}
	
	SDL_SetColorKey(sdl_dib, SDL_SRCCOLORKEY, sp);
	
	setRect(r_src, sx, sy, w, h);
	setRect(r_dst, dx, dy, w, h);
	
	SDL_BlitSurface(sdl_dib, &r_src, sdl_dib, &r_dst);
	SDL_SetColorKey(sdl_dib, 0, 0);
}

void sdl_drawImage8_fromData(cgdata *cg, int dx, int dy, int w, int h) {
	SDL_Surface *s;
	SDL_Rect r_src, r_dst;
	
	s = SDL_AllocSurface(SDL_SWSURFACE, w, h, 8, 0, 0, 0, 0);

	SDL_LockSurface(s);

#if 0  /* for broken cg */
	if (s->pitch == s->w) {
		memcpy(s->pixels, cg->pic, w * h);
	} else 
#endif
	{
		int i = h;
		BYTE *p_src = (cg->pic + cg->data_offset), *p_dst = s->pixels;
		
		while (i--) {
			memcpy(p_dst, p_src, w);
			p_dst += s->pitch;
			p_src += cg->width;
		}
	}
	
	SDL_UnlockSurface(s);
	
	sdl_pal_check();
	
	if (sdl_dib->format->BitsPerPixel > 8 && cg->pal) {
		int i, i_st = 0, i_end = 256;
		SDL_Color *c = s->format->palette->colors;
		BYTE *r = cg->pal->red, *g = cg->pal->green, *b = cg->pal->blue;
		
		if (cg->type == ALCG_VSP) {
			i_st  = (cg->vsp_bank << 4);
			i_end = i_st + 16;
			c += i_st;
		}
		for (i = i_st; i < i_end; i++) {
			c->r = *(r++);
			c->g = *(g++);
			c->b = *(b++);
			c++;
		}
	} else {
		memcpy(s->format->palette->colors, sdl_col, sizeof(SDL_Color) * 256);
	}
	
	if (cg->spritecolor != -1) {
		SDL_SetColorKey(s, SDL_SRCCOLORKEY, cg->spritecolor);
	}
	
	setRect(r_src,  0,  0, w, h);
	setRect(r_dst, dx, dy, w, h);
	
	SDL_BlitSurface(s, &r_src, sdl_dib, &r_dst);
	SDL_FreeSurface(s);
}

/* 直線描画 */
void sdl_drawLine(int x1, int y1, int x2, int y2, u_long cl) {

	sdl_pal_check();
	
	if (sdl_dib->format->BitsPerPixel > 8 && cl < 256) {
		cl = SDL_MapRGB(sdl_dib->format,
				sdl_col[cl].r, sdl_col[cl].g, sdl_col[cl].b);
	}
	
	SDL_LockSurface(sdl_dib);

	image_drawLine(sdl_dibinfo, x1, y1, x2, y2, cl);
	
	SDL_UnlockSurface(sdl_dib);
	
}

static agsurface_t* surface2com(SDL_Surface *src) {
	agsurface_t *dst = g_new(agsurface_t, 1);
	
	dst->depth           = src->format->BitsPerPixel;
	dst->bytes_per_pixel = src->format->BytesPerPixel;
	dst->bytes_per_line  = src->pitch;
	dst->pixel   = src->pixels;
	dst->width  = src->w;
	dst->height = src->h;
	
	return dst;
}

static SDL_Surface *com2surface(agsurface_t *src) {
	SDL_Surface *s;
	int y;
	BYTE *sp, *dp;
	
	s = SDL_AllocSurface(SDL_SWSURFACE, src->width, src->height, src->depth, 0, 0, 0, 0);
	
	SDL_LockSurface(s);
	
	sp = s->pixels;
	dp = src->pixel;
	
	for (y = 0; y < src->height; y++) {
		memcpy(sp, dp, src->width);
		sp += s->pitch;
		dp += src->bytes_per_line;
	}
	
	SDL_UnlockSurface(s);
	return s;
}

static SDL_Surface *com2alphasurface(agsurface_t *src, int cl) {
	SDL_Surface *s;
	int x,y;
	BYTE *sp, *dp;
	SDL_Rect r_src;
	
	s = SDL_AllocSurface(SDL_SWSURFACE, src->width, src->height, 
			     sdl_dib->format->BitsPerPixel <= 24 ? sdl_dib->format->BitsPerPixel+8:32,
			     sdl_dib->format->Rmask,sdl_dib->format->Gmask,
			     sdl_dib->format->Bmask,
			     sdl_dib->format->BitsPerPixel<24?0xFF0000:0xFF000000);
	
	setRect(r_src, 0, 0, src->width, src->height);
	SDL_FillRect(s, &r_src,
		     SDL_MapRGB(sdl_dib->format, sdl_col[cl].r, sdl_col[cl].g, sdl_col[cl].b));

	SDL_LockSurface(s);
	
	for (y = 0; y < src->height; y++) {
		sp = src->pixel + y * src->bytes_per_line;
		dp = s->pixels + y * s->pitch;
#ifndef WORDS_BIGENDIAN
		dp += s->format->BytesPerPixel -1;
#endif
		
		for (x = 0; x < src->width; x++) {
			*dp =  R_ALPHA(*sp);
			sp++;
			dp += s->format->BytesPerPixel;
		}
	}
	
	SDL_UnlockSurface(s);
	return s;
}



int sdl_drawString(int x, int y, char *msg, u_long col) {
	int w;

	sdl_pal_check();
	
	if (sdl_font->self_drawable()) {
		w = sdl_font->draw_glyph(x, y, msg, col);
	} else {
		agsurface_t *glyph = sdl_font->get_glyph(msg);
		SDL_Rect r_src, r_dst;
		
		if (glyph == NULL) return 0;
		setRect(r_src, 0, 0, glyph->width, glyph->height);
		setRect(r_dst, x, y, glyph->width, glyph->height);
		if (sdl_font->antialiase_on && sdl_dib->format->BitsPerPixel != 8) {
			SDL_Surface *src = com2alphasurface(glyph, col);
			
			SDL_BlitSurface(src, &r_src, sdl_dib, &r_dst);
			SDL_FreeSurface(src);
		} else {
			int i;
			SDL_Surface *src = com2surface(glyph);
			for (i = 1; i < 256; i++) { 
				memcpy(src->format->palette->colors + i, &sdl_col[col],
				       sizeof(SDL_Color));
			}
			SDL_SetColorKey(src, SDL_SRCCOLORKEY, 0);
			SDL_BlitSurface(src, &r_src, sdl_dib, &r_dst);
			SDL_FreeSurface(src);
		}
		w = glyph->width;
	}
	
	return w;
}

void sdl_Mosaic(int sx, int sy, int w, int h, int dx, int dy, int slice) {
	
	SDL_LockSurface(sdl_dib);

	image_Mosaic(sdl_dibinfo, sx, sy, w, h, dx, dy, slice); 

	SDL_UnlockSurface(sdl_dib);
}

static void setBligtness(SDL_Surface *s, int val) {
	int i;
	Pallet256 *pal = nact->sys_pal;
	Uint8 *r = pal->red, *g = pal->green, *b = pal->blue;
	SDL_Color *cl = sdl_col;
	
	for (i = 0; i < 256; i++) {
		cl->r = (val * (*(r++))) / 255;
		cl->g = (val * (*(g++))) / 255;
		cl->b = (val * (*(b++))) / 255;
		cl++;
	}
	SDL_SetColors(s, sdl_col, 0, 256);
}

static void setWhiteness(SDL_Surface *s, int val) {
	int i;
	Pallet256 *pal = nact->sys_pal;
	Uint8 *r = pal->red, *g = pal->green, *b = pal->blue;
	SDL_Color *cl = sdl_col;
	
	for (i = 0; i < 256; i++) {
		cl->r = (((255- *r) * val) / 256) + *r; r++;
		cl->g = (((255- *g) * val) / 256) + *g; g++;
		cl->b = (((255- *b) * val) / 256) + *b; b++;
		cl++;
	}
	SDL_SetColors(s, sdl_col, 0, 256);
}

static void fader_in(int n) {
	static agsurface_t *work, *disp;
	
	if (n == 0) {
		SDL_Rect r_src, r_dst;
		
		s_fader = SDL_AllocSurface(sdl_dib->flags, sdl_display->w, sdl_display->h,
					   sdl_display->format->BitsPerPixel, 0, 0, 0, 0);
		
		if (sdl_display->format->BitsPerPixel == 8) {
			memcpy(s_fader->format->palette->colors,
			       sdl_display->format->palette->colors,
			       sizeof(SDL_Color) * 256);
		}
		setRect(r_src, view_x, view_y, view_w, view_h);
		setRect(r_dst, winoffset_x, winoffset_y, view_w, view_h);
		SDL_BlitSurface(sdl_dib, &r_src, s_fader, &r_dst);
		
		work = surface2com(s_fader);
		disp = surface2com(sdl_display);
	}
	
	if (n == 255) {
		SDL_FreeSurface(s_fader);
		sdl_updateAll();
		g_free(work);
		g_free(disp);
		return;
	}
	
	SDL_LockSurface(s_fader);
	SDL_LockSurface(sdl_display);
	
	image_fadeIn(work, disp, n / 16);
	
	SDL_UnlockSurface(sdl_display);
	SDL_UnlockSurface(s_fader);
	SDL_UpdateRect(sdl_display,0,0,0,0);
}

static void fader_out(int n,Uint32 c) {
	static agsurface_t *disp;

	if (n == 0) {
		disp = surface2com(sdl_display);
	}
	
	if (n == 255) {
		SDL_FillRect(sdl_display, NULL, c);
		g_free(disp);
		return;
	}
	
	SDL_LockSurface(sdl_display);
	
	image_fadeOut(disp, (255 - n) / 16, c);
	
	SDL_UnlockSurface(sdl_display);
	
	SDL_UpdateRect(sdl_display,0,0,0,0);
}

static __inline void sdl_fade_blit(void) {
	SDL_Rect r_dst;
	setRect(r_dst, winoffset_x, winoffset_y, view_w, view_h);

	SDL_BlitSurface(sdl_dib, &sdl_view, sdl_display, &r_dst);
	SDL_UpdateRect(sdl_display, 0, 0, view_w, view_h);
}

void sdl_fadeIn(int step) {
	if (sdl_display->flags & SDL_HWPALETTE) {
		setBligtness(sdl_display, fadestep[step]);
	} else if (sdl_dib->format->BitsPerPixel == 8) {
		setBligtness(sdl_dib, fadestep[step]);
		sdl_fade_blit();
	} else {
		fader_in(step);
	}
}

void sdl_fadeOut(int step) {
	if (sdl_display->flags & SDL_HWPALETTE) {
		setBligtness(sdl_display, fadestep[255 - step]);
	} else if (sdl_dib->format->BitsPerPixel == 8) {
		setBligtness(sdl_dib, fadestep[255 - step]);
		sdl_fade_blit();
	} else {
		fader_out(step, SDL_MapRGB(sdl_display->format, 0, 0, 0));
	}
}

void sdl_whiteIn(int step) {
	if (sdl_display->flags & SDL_HWPALETTE) {
		setWhiteness(sdl_display, fadestep[step]);
	} else if (sdl_dib->format->BitsPerPixel == 8) {
		setWhiteness(sdl_dib, fadestep[255 - step]); /* ??? */
		sdl_fade_blit();
	} else {
		fader_in(step);
	}
}

void sdl_whiteOut(int step) {
	if (sdl_display->flags & SDL_HWPALETTE) {
		setWhiteness(sdl_display, fadestep[255 - step]);
	} else if (sdl_dib->format->BitsPerPixel == 8) {
		setWhiteness(sdl_dib, fadestep[step]); /* ??? */
		sdl_fade_blit();
	} else {
		fader_out(step, SDL_MapRGB(sdl_display->format, 255, 255, 255));
	}
}

/*
 * 指定範囲にパレット col を rate の割合で重ねる CK1
 */
void sdl_wrapColor(int sx, int sy, int w, int h, int cl, int rate) {
	SDL_Surface *s;
	SDL_Rect r_src,r_dst;

	s = SDL_AllocSurface(SDL_SWSURFACE, w, h,
			     sdl_dib->format->BitsPerPixel, 0, 0, 0, 0);
	
	if (s->format->BitsPerPixel == 8) {
		memcpy(s->format->palette->colors, sdl_dib->format->palette->colors,
		       sizeof(SDL_Color)*256);
	} else {
		cl = (cl < 256) ? SDL_MapRGB(sdl_dib->format, sdl_col[cl].r, sdl_col[cl].g, sdl_col[cl].b) : cl;
	}
	
	setRect(r_src, 0, 0, w, h);
	SDL_FillRect(s, &r_src, cl);
	
	SDL_SetAlpha(s, RLEFLAG(SDL_SRCALPHA), R_ALPHA(rate));
	setRect(r_dst, sx, sy, w, h);
	SDL_BlitSurface(s, &r_src, sdl_dib, &r_dst);
	SDL_FreeSurface(s);
}

/* mask update まだ */
void sdl_maskupdate(int sx, int sy, int w, int h, int dx, int dy, int func, int step) {

	if (step == 256) {
		ags_copyArea(sx, sy, w, h, dx, dy);
		ags_updateArea(dx, dy, w, h);
	}
}
