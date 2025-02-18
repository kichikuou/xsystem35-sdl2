#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>

#include "portab.h"
#include "system.h"
#include "surface.h"
#include "graph.h"
#include "ngraph.h"
#include "ags.h"
#include "sdl_core.h"

/*
  gr_xxxx はクリッピングあり
  gre_xxxx はクリッピングなし
*/

/**
 * surface から surface にコピーなどをする際に、転送元と転送先のsurface
 * の大きさや、転送元・転送先座標、転送する領域の大きさなどから、実際に
 * コピーする領域をクリッピングする
 * 
 * @param ss: 転送元surface
 * @param sx: 転送元Ｘ座標
 * @param sy: 転送元Ｙ座標
 * @param sw: 転送幅
 * @param sh: 転送高さ
 * @param ds: 転送先surface
 * @param dx: 転送先Ｘ座標
 * @param dy: 転送先Ｙ座標
 * @return true -> 描画領域がある
 *         false-> 転送領域の高さや幅が０以下になったり、転送元、転送先の座標が
 *                 surfaceの範囲外になり、描画領域がなくなった。
 *         それぞれの引数は適宜変更されている
 */
bool gr_clip(surface_t *ss, int *sx, int *sy, int *sw, int *sh, surface_t *ds, int *dx, int *dy) {
	if (!ss || !ds) return false;
	MyRectangle src_window = { 0, 0, ss->width, ss->height };
	MyRectangle dst_window = { 0, 0, ds->width, ds->height };
	return ags_clipCopyRect(&src_window, &dst_window, sx, sy, dx, dy, sw, sh);
}

/**
 * surface 内に描画を行う際に、surface の大きさで描画領域をクリッピングする
 * 
 * @param ss: 描画surface
 * @param sx: 描画開始Ｘ座標
 * @param sy: 描画開始Ｙ座標
 * @param sw: 描画幅
 * @param sh: 描画高さ
 * @return true -> 描画領域がある
 *         false-> 描画領域の高さや幅が０以下になったり、描画開始座標が
 *                 surfaceの範囲外になり、描画領域がなくなった。
 *         それぞれの引数は適宜変更されている
 */
bool gr_clip_xywh(surface_t *ss, int *sx, int *sy, int *sw, int *sh) {
	if (!ss) return false;
	SDL_Rect rect = {*sx, *sy, *sw, *sh};
	if (!SDL_IntersectRect(&rect, &(SDL_Rect){ 0, 0, ss->width, ss->height }, &rect))
		return false;
	*sx = rect.x;
	*sy = rect.y;
	*sw = rect.w;
	*sh = rect.h;
	return true;
}

void gr_init() {
}

void gr_copy(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int sw, int sh) {
	if (!dst->sdl_surface || !src->sdl_surface) return;
	if (!gr_clip(src, &sx, &sy, &sw, &sh, dst, &dx, &dy)) return;

	SDL_Rect src_rect = {sx, sy, sw, sh};
	SDL_Rect dst_rect = {dx, dy, sw, sh};
	if (dst == src) {
		SDL_Rect r;
		if (SDL_IntersectRect(&src_rect, &dst_rect, &r)) {
			SDL_Surface *view = sdl_createSurfaceView(src->sdl_surface, sx, sy, sw, sh);
			SDL_Surface *tmp = SDL_ConvertSurface(view, dst->sdl_surface->format, 0);
			src_rect.x = 0;
			src_rect.y = 0;
			SDL_LowerBlit(tmp, &src_rect, dst->sdl_surface, &dst_rect);
			SDL_FreeSurface(tmp);
			SDL_FreeSurface(view);
			return;
		}
	}
	SDL_LowerBlit(src->sdl_surface, &src_rect, dst->sdl_surface, &dst_rect);
}

// Copy the specified surface area with brightness lv/255 times
void gr_copy_bright(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int width, int height, int lv) {
	SDL_Rect src_rect = {sx, sy, width, height};
	SDL_Rect dst_rect = {dx, dy, width, height};
	SDL_SetSurfaceColorMod(src->sdl_surface, lv, lv, lv);
	SDL_BlitSurface(src->sdl_surface, &src_rect, dst->sdl_surface, &dst_rect);
	SDL_SetSurfaceColorMod(src->sdl_surface, 255, 255, 255);
}

void gr_fill(surface_t *dst, int dx, int dy, int dw, int dh, int r, int g, int b) {
	SDL_Rect rect = {dx, dy, dw, dh};
	uint32_t color = SDL_MapRGB(dst->sdl_surface->format, r, g, b);
	SDL_FillRect(dst->sdl_surface, &rect, color);
}

void gr_drawrect(surface_t *dst, int x, int y, int w, int h, int r, int g, int b) {
	SDL_Rect rects[4] = {
		{x, y, w, 1},
		{x, y + h - 1, w, 1},
		{x, y, 1, h},
		{x + w - 1, y, 1, h}
	};
	uint32_t color = SDL_MapRGB(dst->sdl_surface->format, r, g, b);
	SDL_FillRects(dst->sdl_surface, rects, 4, color);
}

void gr_copy_stretch(surface_t *dst, int dx, int dy, int dw, int dh, surface_t *src, int sx, int sy, int sw, int sh) {
	SDL_Rect srcrect = {sx, sy, sw, sh};
	SDL_Rect dstrect = {dx, dy, dw, dh};
	SDL_BlitScaled(src->sdl_surface, &srcrect, dst->sdl_surface, &dstrect);
}

void gr_blend(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int width, int height, int lv) {
	if (!gr_clip(src, &sx, &sy, &width, &height, dst, &dx, &dy)) return;

	uint8_t *sp = GETOFFSET_PIXEL(src,   sx, sy);
	uint8_t *dp = GETOFFSET_PIXEL(dst,   dx, dy);

	switch(dst->depth) {
	case 16:
		for (int y = 0; y < height; y++) {
			uint16_t *yls = (uint16_t *)(sp + y * src->bytes_per_line);
			uint16_t *yld = (uint16_t *)(dp + y * dst->bytes_per_line);
			for (int x = 0; x < width; x++) {
				*yld = ALPHABLEND16(*yls, *yld, lv);
				yls++; yld++;
			}
		}
		break;

	case 24:
	case 32:
		for (int y = 0; y < height; y++) {
			uint32_t *yls = (uint32_t *)(sp + y * src->bytes_per_line);
			uint32_t *yld = (uint32_t *)(dp + y * dst->bytes_per_line);
			for (int x = 0; x < width; x++) {
				*yld = ALPHABLEND24(*yls, *yld, lv);
				yls++; yld++;
			}
		}
		break;
	}
}

void gr_blend_screen(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int width, int height) {
	SDL_Rect src_rect = { sx, sy, width, height };
	SDL_Rect dst_rect = { dx, dy, width, height };
	SDL_SetSurfaceBlendMode(src->sdl_surface, SDL_BLENDMODE_ADD);
	SDL_BlitSurface(src->sdl_surface, &src_rect, dst->sdl_surface, &dst_rect);
	SDL_SetSurfaceBlendMode(src->sdl_surface, SDL_BLENDMODE_NONE);
}

void gr_blend_src_bright(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int width, int height, int alpha, int rate) {
}

void gr_blend_add_satur(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int width, int height) {
}


void gr_blend_alpha_map_src_only(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int width, int height) {
	// Dest Pixel = Blend(DestPixel, SrcPixel, SrcAlphaMap);
}

void gr_blend_alpha_map_color(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int width, int height, int r, int g, int b) {
	// Dest Pixel = Blend(DestPixel, Color, SrcAlphaMap);
}

void gr_blend_alpha_map_color_alpha(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int width, int height, int r, int g, int b, int rate) {
	// Dest Pixel = Blend(DestPixel, Color, SrcAlphaMap x rate);
}

void gr_blend_alpha_map_alpha(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int width, int height, int rate) {
	// Dest Pixel = Blend(DestPixel, SrcPixel, SrcAlphaMap x rate);
}

void gr_blend_alpha_map_bright(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int width, int height, int lv) {
	// Dest Pixel = Blend(DestPixel, SrcPixel x lv, SrcAlphaMap);
}

void gr_blend_alpha_map_alpha_src_bright(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int width, int height, int rate, int lv) {
	// Dest Pixel = Blend(DestPixel, SrcPixel x lv, SrcAlphaMap x rate);
}

void gr_blend_use_amap_color(surface_t *dst, int dx, int dy, int width, int height, surface_t *alpha, int ax, int ay, int r, int g, int b, int rate) {
}

void gr_blend_multiply(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int w, int h) {
}

void gr_blend_screen_alpha(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int w, int h, int rate) {
	// Screen x rate
}


void gr_screen_DA_DAxSA(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int width, int height) {
	// DestAlpha = Screen(DestAlpha, SrcAlpha);
}

void gr_add_DA_DAxSA(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int width, int height) {
	// Dest Alpha = Satur(DestAlpha + SrcAlpha);
}

void gr_copy_texture_wrap() {
}

void gr_copy_texture_wrap_alpha() {
}

void gr_copy_stretch_blend() {
}

void gr_copy_stretch_blend_alpha_map(surface_t *dst, int dx, int dy, int dw, int dh, surface_t *src, int sx, int sy, int sw, int sh) {
	float    a1, a2, xd, yd;
	int      *row, *col;
	int      x, y;
	uint8_t  *sp, *dp, *sa;
	
	if (!gr_clip_xywh(dst, &dx, &dy, &dw, &dh)) return;
	if (!gr_clip_xywh(src, &sx, &sy, &sw, &sh)) return;
	
	sp = GETOFFSET_PIXEL(src, sx, sy);
	dp = GETOFFSET_PIXEL(dst, dx, dy);
	sa = GETOFFSET_ALPHA(src, sx, sy);
	
	a1  = (float)sw / (float)dw;
	a2  = (float)sh / (float)dh;
	// src width と dst width が同じときに問題があるので+1
	row = calloc(dw+1, sizeof(int));
	// 1おおきくして初期化しないと col[dw-1]とcol[dw]が同じになる
	// 可能性がある。
	col = calloc(dh+1, sizeof(int));
	
	for (yd = 0.0, y = 0; y < dh; y++) {
		col[y] = yd; yd += a2;
	}
	
	for (xd = 0.0, x = 0; x < dw; x++) {
		row[x] = xd; xd += a1;
	}

	switch(dst->depth) {
	case 16:
	{
		uint16_t *yls, *yld;
		uint8_t *yla;
		
		for (y = 0; y < dh; y++) {
			yls = (uint16_t *)(sp + *(y + col) * src->bytes_per_line);
			yld = (uint16_t *)(dp +   y        * dst->bytes_per_line);
			yla = (uint8_t *)(sa + *(y + col) * src->width);
			for (x = 0; x < dw; x++) {
				*(yld + x) = ALPHABLEND16(*(yls+ *(row + x)), *(yld+x), *(yla+*(row+x)));
			}
			while (y < dh - 1 && *(col + y) == *(col + y + 1)) {
				yld += dst->width;
				for (x = 0; x < dw; x++) {
					*(yld + x) = ALPHABLEND16(*(yls+ *(row+x)), *(yld+x), *(yla+*(row+x)));
				}
				y++;
			}
		}
		break;
	}
	case 24:
	case 32:
	{
		uint32_t *yls, *yld;
		uint8_t  *yla;
		
		for (y = 0; y < dh; y++) {
			yls = (uint32_t *)(sp + *(y + col) * src->bytes_per_line);
			yld = (uint32_t *)(dp +   y        * dst->bytes_per_line);
			yla = (uint8_t  *)(sa + *(y + col) * src->width);
			for (x = 0; x < dw; x++) {
				*(yld + x) = ALPHABLEND24(*(yls+ *(row + x)), *(yld+x), *(yla+*(row+x)));
			}
			while (y < dh - 1 && *(col + y) == *(col + y + 1)) {
				yld += dst->width;
				for (x = 0; x < dw; x++) {
					*(yld + x) = ALPHABLEND24(*(yls+ *(row+x)), *(yld+x), *(yla+*(row+x)));
				}
				y++;
			}
		}
		break;
	}
	}
	
	free(row);
	free(col);
}

// Fill the rectangle with the specified color (rgb) and blend rate (lv)
void gr_fill_alpha_color(surface_t *dst, int dx, int dy, int dw, int dh, int r, int g, int b, int lv) {
	SDL_Surface *src = SDL_CreateRGBSurfaceWithFormat(0, dw, dh, 32, SDL_PIXELFORMAT_ARGB8888);
	SDL_FillRect(src, NULL, SDL_MapRGBA(src->format, r, g, b, lv));
	SDL_SetSurfaceBlendMode(src, SDL_BLENDMODE_BLEND);
	SDL_Rect dstrect = {dx, dy, dw, dh};
	SDL_BlitSurface(src, NULL, dst->sdl_surface, &dstrect);
	SDL_FreeSurface(src);
}

#include "graph2.c"
