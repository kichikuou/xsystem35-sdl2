/*
 * sdl_image.c  image操作 for SDL
 *
 * Copyright (C) 2000-   Fumihiko Murata <fmurata@p1.tcnet.ne.jp>
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
/* $Id: sdl_image.c,v 1.23 2003/07/20 19:30:16 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <SDL.h>

#include "portab.h"
#include "system.h"
#include "sdl_private.h"
#include "cg.h"
#include "nact.h"
#include "alpha_plane.h"
#include "image.h"

static unsigned char shlv_tbl[256*256];

/*
 * dib内での拡大・縮小コピー
 */
void sdl_scaledCopyArea(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, int mirror) {
	float    a1, a2, xd, yd;
	int      *row, *col;
	int      x, y;
	SDL_Surface *ss;
	SDL_Surface *src = sdl_dib;
	SDL_Surface *dst = sdl_dib;
	
	ss = SDL_CreateRGBSurface(0, dw, dh, dst->format->BitsPerPixel, 0, 0, 0, 0);
	
	SDL_LockSurface(ss);

	if (dst->format->BitsPerPixel == 8) {
		memcpy(ss->format->palette->colors, dst->format->palette->colors,
		       sizeof(SDL_Color) * 256);
	}
	
	a1  = (float)sw / (float)dw;
	a2  = (float)sh / (float)dh;
	// src width と dst width が同じときに問題があるので+1
	row = calloc(dw+1, sizeof(int));
	// 1おおきくして初期化しないと col[dw-1]とcol[dw]が同じになる
	// 可能性がある。
	col = calloc(dh+1, sizeof(int));
	
	if(mirror & 1){
		/*上下反転 added by  tajiri@wizard*/
		for (yd = sh-a2, y = 0; y<dh; y++) {
			col[y] = yd; yd -= a2;
		}
	} else {
		for (yd = 0.0, y = 0; y < dh; y++) {
			col[y] = yd; yd += a2;
		}
	}
	if(mirror & 2){
		/*左右反転 added by  tajiri@wizard*/
		for (xd = sw-a1, x = 0; x <dw; x++) {
			row[x] = xd; xd -= a1;
		}
	} else {
		for (xd = 0.0, x = 0; x < dw; x++) {
			row[x] = xd; xd += a1;
		}
	}

#define sccp(type) \
{ \
	type *p_ss = src->pixels + sx * src->format->BytesPerPixel + sy * src->pitch;\
	type *p_dd = ss->pixels, *p_src, *p_dst;\
	int l=src->pitch/src->format->BytesPerPixel; \
	int m=ss->pitch/ss->format->BytesPerPixel; \
	for (y = 0; y < dh; y++) { \
		p_src = p_ss + col[y] * l; \
		p_dst = p_dd + y * m; \
		for (x = 0; x < dw ; x++) { \
			*(p_dst++) = *(p_src + *(row + x)); \
		} \
	} \
}
	switch(dst->format->BytesPerPixel) {
	case 1:
		sccp(BYTE);
		break;
	case 2:
		sccp(WORD);
		break;
	case 3: {
		BYTE *p_ss=(BYTE *)(src->pixels+sx*src->format->BytesPerPixel+sy*src->pitch);
		BYTE *p_src=p_ss,*p_dd=ss->pixels, *p_dst;
		for (y = 0; y < dh; y++) {
			p_src = p_ss + col[y] * src->pitch;
			p_dst = p_dd + y      * ss->pitch;
			for (x = 0; x < dw ; x++) {
				*(p_dst)   = *(p_src + (*(row + x))*3    );
				*(p_dst+1) = *(p_src + (*(row + x))*3 + 1);
				*(p_dst+2) = *(p_src + (*(row + x))*3 + 2);
				p_dst+=3;
			}
		}
		break;
	}
	case 4:
		sccp(Uint32);
		break;
	}
	
	SDL_UnlockSurface(ss);
	
	SDL_Rect r_src = { 0,  0, dw, dh};
	SDL_Rect r_dst = {dx, dy, dw, dh};
	SDL_BlitSurface(ss, &r_src, dst, &r_dst);
	
	SDL_FreeSurface(ss);
	
	free(row);
	free(col);
}


/*
 * dibに16bitCGの描画
 */
void sdl_drawImage16_fromData(cgdata *cg, int dx, int dy, int brightness, bool alpha_blend) {
	int w = cg->width;
	int h = cg->height;

	SDL_Surface *s;
	if (cg->alpha && alpha_blend) {
		unsigned short *p_src = (WORD *)cg->pic;
		DWORD *p_ds, *p_dst;
		BYTE *a_src = cg->alpha;
		BYTE *adata = GETOFFSET_ALPHA(sdl_dibinfo, dx, dy);
		int x, y, l;
		
		s = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_ARGB8888);
		SDL_LockSurface(s);
		p_ds = s->pixels;
		l = s->pitch/4;
		for (y = 0; y < h; y++) {
			p_dst = p_ds;
			memcpy(adata, a_src, w);
			for (x = 0; x < w; x++) {
				*p_dst++ = *a_src++ << 24 | PIX24(PIXR16(*p_src), PIXG16(*p_src), PIXB16(*p_src));
				p_src++;
			}
			p_ds  += l;
			adata += sdl_dibinfo->width;
		}
		SDL_UnlockSurface(s);
	} else {
		s = SDL_CreateRGBSurfaceWithFormatFrom(
			cg->pic, cg->width, cg->height, 16, cg->width * 2, SDL_PIXELFORMAT_RGB565);
		if (cg->alpha) {
			BYTE *a_src = cg->alpha;
			BYTE *adata = GETOFFSET_ALPHA(sdl_dibinfo, dx, dy);
			
			for (int i = 0; i < h; i++) {
				memcpy(adata, a_src, w);
				adata += sdl_dibinfo->width;
				a_src += cg->width;
			}
		}
	}

	if (brightness != 255)
		SDL_SetSurfaceColorMod(s, brightness, brightness, brightness);

	SDL_Rect r_dst = {dx, dy, w, h};
	SDL_BlitSurface(s, NULL, sdl_dib, &r_dst);
	SDL_FreeSurface(s);
}

void sdl_drawImage24_fromData(cgdata *cg, int x, int y, int brightness) {
	if (cg->alpha) {
		// This function is called only for JPEG images, so this shouldn't happen.
		WARNING("sdl_drawImage24_fromData: unsupported format");
		return;
	}

	SDL_Surface *s = SDL_CreateRGBSurfaceWithFormatFrom(
		cg->pic, cg->width, cg->height, 24, cg->width * 3, SDL_PIXELFORMAT_RGB24);
	if (brightness != 255)
		SDL_SetSurfaceColorMod(s, brightness, brightness, brightness);

	SDL_Rect r_dst = {x, y, cg->width, cg->height};
	SDL_BlitSurface(s, NULL, sdl_dib, &r_dst);
	SDL_FreeSurface(s);
}

/*
 * 16bit専用の dib の指定領域コピー alphaつき
 */
void sdl_shadow_init(void) {
	int i, j;
	unsigned char *c = shlv_tbl;

	for (i = 0; i < 255; i++) {
		for (j = 0; j < 256; j++) {
			*(c++) = (unsigned char)(i * j / 255);
		}
	}
}

void sdl_copyAreaSP16_shadow(int sx, int sy, int w, int h, int dx, int dy, int lv) {
	BYTE *adata = GETOFFSET_ALPHA(sdl_dibinfo, sx, sy);
	BYTE *p_src, *p_dst, *p_ds;
	
	SDL_Surface *s = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32, SDL_PIXELFORMAT_ARGB8888);
	int x, y;

	SDL_Rect r_src = {sx, sy, w, h};
	SDL_Rect r_tmp = { 0,  0, w, h};
	SDL_BlitSurface(sdl_dib, &r_src, s, &r_tmp);

	SDL_LockSurface(s);

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	p_ds = s->pixels + 3;
#else
	p_ds = s->pixels;
#endif
	switch(lv) {
	case 255:
		for (y = 0; y < h; y++) {
			p_src = adata;
			p_dst = p_ds;
			for (x = 0; x < w; x++) {
				*p_dst = *(p_src++);
				p_dst += 4;
			}
			adata += sdl_dibinfo->width;
			p_ds  += s->pitch;
		}
		break;
	default:
		{
			unsigned char *lvtbl = shlv_tbl + lv * 256;
			for (y = 0; y < h; y++) {
				p_src = adata;
				p_dst = p_ds;
				for (x = 0; x < w; x++) {
					*p_dst = lvtbl[(int)(*(p_src++))];
					p_dst += 4;
				}
				adata += sdl_dibinfo->width;
				p_ds += s->pitch;
			}
		}
		break;
	}
	SDL_UnlockSurface(s);

	SDL_Rect r_dst = {dx, dy, w, h};
	SDL_BlitSurface(s, &r_tmp, sdl_dib, &r_dst);
	SDL_FreeSurface(s);
}

void sdl_copyAreaSP16_alphaBlend(int sx, int sy, int w, int h, int dx, int dy, int lv) {
	SDL_Rect r_src = {sx, sy, w, h};
	SDL_Rect r_dst = {dx, dy, w, h};
	SDL_SetSurfaceBlendMode(sdl_dib, SDL_BLENDMODE_BLEND);
	SDL_SetSurfaceAlphaMod(sdl_dib, lv);
	SDL_BlitSurface(sdl_dib, &r_src, sdl_dib, &r_dst);
	SDL_SetSurfaceBlendMode(sdl_dib, SDL_BLENDMODE_NONE);
}

void sdl_copyAreaSP16_alphaLevel(int sx, int sy, int w, int h, int dx, int dy, int lv) {
	SDL_Rect r_src = {sx, sy, w, h};
	SDL_Rect r_dst = {dx, dy, w, h};
	SDL_SetSurfaceBlendMode(sdl_dib, SDL_BLENDMODE_BLEND);
	SDL_SetSurfaceAlphaMod(sdl_dib, lv);
	SDL_FillRect(sdl_dib, &r_dst, 0);
	SDL_BlitSurface(sdl_dib, &r_src, sdl_dib, &r_dst);
	SDL_SetSurfaceBlendMode(sdl_dib, SDL_BLENDMODE_NONE);
}

void sdl_copy_from_alpha(int sx, int sy, int w, int h, int dx, int dy, ALPHA_DIB_COPY_TYPE flag) {
	SDL_LockSurface(sdl_dib);
	image_copy_from_alpha(nact->ags.dib, sx, sy, w, h, dx, dy, flag);
	SDL_UnlockSurface(sdl_dib);
}

void sdl_copy_to_alpha(int sx, int sy, int w, int h, int dx, int dy, ALPHA_DIB_COPY_TYPE flag) {
	SDL_LockSurface(sdl_dib);
	image_copy_to_alpha(nact->ags.dib, sx, sy, w, h, dx, dy, flag);
	SDL_UnlockSurface(sdl_dib);
}

/*
 * dib のピクセル情報を取得
 */
void sdl_getPixel(int x, int y, Palette *cell) {
	SDL_LockSurface(sdl_dib);

	BYTE *p = PIXEL_AT(sdl_dib, x, y);
	if (sdl_dib->format->BitsPerPixel == 8) {
		cell->pixel = *p;
	} else {
		Uint32 cl=0;
		
		switch (sdl_dib->format->BytesPerPixel) {
		case 2:
			{
				unsigned short *pp = (unsigned short *)p;
				cl = *pp;
			}
			break;
		case 3:
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
			cl = (p[2]<<16)+(p[1]<<8) + p[0];
#else
			cl = (p[0]<<16)+(p[1]<<8) + p[2];
#endif
			break;
		case 4:
			{
				Uint32 *pp = (Uint32 *)p;
				cl = *pp;
			}
			break;
		}
		SDL_GetRGB(cl, sdl_dib->format, &cell->r, &cell->g, &cell->b);
	}
	
	SDL_UnlockSurface(sdl_dib);
}

/*
 * dib から領域の切り出し
 */
void* sdl_saveRegion(int x, int y, int w, int h) {
	SDL_Surface *s = SDL_CreateRGBSurface(0, w, h, sdl_dib->format->BitsPerPixel,
			     sdl_dib->format->Rmask, sdl_dib->format->Gmask,
			     sdl_dib->format->Bmask, sdl_dib->format->Amask);
	if (sdl_dib->format->BitsPerPixel == 8)
		memcpy(s->format->palette->colors, sdl_dib->format->palette->colors,
		       sizeof(SDL_Color) * sdl_dib->format->palette->ncolors);
	SDL_Rect r_src = {x, y, w, h};
	SDL_Rect r_dst = {0, 0, w, h};
	SDL_BlitSurface(sdl_dib, &r_src, s, &r_dst);
	return s;
}

/*
 * セーブした領域を破棄
 */
void sdl_delRegion(void *psrc) {
	SDL_FreeSurface((SDL_Surface *)psrc);
}

/*
 * dib にセーブした領域を回復
 */
void sdl_putRegion(void *psrc, int x, int y) {
	SDL_Surface *src = (SDL_Surface *)psrc;
	
	SDL_Rect r_src = {0, 0, src->w, src->h};
	SDL_Rect r_dst = {x, y, src->w, src->h};

	SDL_BlitSurface(src, &r_src, sdl_dib, &r_dst);
}

/*
 * dib にセーブした領域からコピー
 */
void sdl_CopyRegion(void *psrc, int sx, int sy, int w,int h, int dx, int dy) {
	SDL_Surface *src = (SDL_Surface *)psrc;

	SDL_Rect r_src = {sx, sy, w, h};
	SDL_Rect r_dst = {dx, dy, w, h};

	SDL_BlitSurface(src, &r_src, sdl_dib, &r_dst);
}

/*
 * dib に dstを描画後、後始末
 */
void sdl_restoreRegion(void *psrc, int x, int y) {
	SDL_Surface *src = (SDL_Surface *)psrc;
	sdl_putRegion(src, x ,y);
	SDL_FreeSurface(src);
}
