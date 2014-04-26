#include "config.h"

#include <stdio.h>
#include <glib.h>
#include <string.h>

#include "portab.h"
#include "system.h"
#include "surface.h"
#include "graph.h"
#include "ngraph.h"
#include "nact.h"

/*
  gr_xxxx はクリッピングあり
  gre_xxxx はクリッピングなし
*/

#undef WARNING
#define WARNING //

#undef NOTICE
#define NOTICE //


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
 * @return TRUE -> 描画領域がある
 *         FALSE-> 転送領域の高さや幅が０以下になったり、転送元、転送先の座標が
 *                 surfaceの範囲外になり、描画領域がなくなった。
 *         それぞれの引数は適宜変更されている
 */
boolean gr_clip(surface_t *ss, int *sx, int *sy, int *sw, int *sh, surface_t *ds, int *dx, int *dy) {
	int w, h;
	
	if (ss == NULL) {
		WARNING("ss surface is null\n");
		return FALSE;
	}
	if (ss == NULL) {
		WARNING("ss surface is null\n");
		return FALSE;
	}
	
	if (*sx > ss->width) {
		WARNING("sx is too large (sx=%d,width=%d)\n", *sx, ss->width);
		return FALSE;
	}
	if (*sy > ss->height) {
		WARNING("sy is too large (sy=%d,height=%d)\n", *sy, ss->height);
		return FALSE;
	}
	
	if (*sx < 0) {
		WARNING("sx is too small (sx=%d)\n", *sx);
		return FALSE;
	}
	if (*sy < 0) {
		WARNING("sy is too small (sy=%d)\n", *sy);
		return FALSE;
	}
	
	if (*dx > ds->width) {
		WARNING("dx is too large (dx=%d,width=%d)\n", *dx, ds->width);
		return FALSE;
	}
	if (*dy > ds->height) {
		WARNING("dy is too large (dy=%d,height=%d)\n", *dy, ds->height);
		return FALSE;
	}
	
	w = *sw;
	h = *sh;
	
	if (*dx < 0) {
		*sx -= *dx; *sw += *dx; *dx = 0;
	}
	if (*dy < 0) {
		*sy -= *dy; *sh += *dy; *dy = 0;
	}
	
	*sw = MIN(ss->width  - *sx, MIN(ds->width  - *dx, *sw));
	*sh = MIN(ss->height - *sy, MIN(ds->height - *dy, *sh));

	if (*sw <= 0) {
		WARNING("sw become <=0\n");
		return FALSE;
	}
	if (*sh <= 0) {
		WARNING("sh become <=0\n");
		return FALSE;
	}
	
	if (*sw != w) {
		NOTICE("width change %d -> %d\n", w, *sw);
	}
	if (*sh != h) {
		NOTICE("height change %d -> %d\n", h, *sh);
	}
	
	return TRUE;
}

/**
 * surface 内に描画を行う際に、surface の大きさで描画領域をクリッピングする
 * 
 * @param ss: 描画surface
 * @param sx: 描画開始Ｘ座標
 * @param sy: 描画開始Ｙ座標
 * @param sw: 描画幅
 * @param sh: 描画高さ
 * @return TRUE -> 描画領域がある
 *         FALSE-> 描画領域の高さや幅が０以下になったり、描画開始座標が
 *                 surfaceの範囲外になり、描画領域がなくなった。
 *         それぞれの引数は適宜変更されている
 */
boolean gr_clip_xywh(surface_t *ss, int *sx, int *sy, int *sw, int *sh) {
	int w, h;
	
	if (ss == NULL) {
		WARNING("ss surface is null\n");
		return FALSE;
	}
	
	if (*sx > ss->width) {
		WARNING("sx is too large (sx=%d,width=%d)\n", *sx, ss->width);
		return FALSE;
	}
	if (*sy > ss->height) {
		WARNING("sy is too large (sy=%d,height=%d)\n", *sy, ss->height);
		return FALSE;
	}
	
	w = *sw;
	h = *sh;
	
	if (*sx < 0) {
		*sw += *sx; *sx = 0;
	}
	if (*sy < 0) {
		*sh += *sy; *sy = 0;
	}
	
	*sw = MIN(ss->width  - *sx, *sw);
	*sh = MIN(ss->height - *sy, *sh);
	
	if (*sw <= 0) {
		WARNING("sw become <=0\n");
		return FALSE;
	}
	if (*sh <= 0) {
		WARNING("sh become <=0\n");
		return FALSE;
	}
	
	if (*sw != w) {
		NOTICE("width change %d -> %d\n", w, *sw);
	}
	if (*sh != h) {
		NOTICE("height change %d -> %d\n", h, *sh);
	}
	
	return TRUE;
}

void gr_init() {
}


void gr_blend(surface_t *dst, int dx, int dy, surface_t *src, int sx, int sy, int width, int height, int lv) {
	
	
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
	BYTE    *sp, *dp, *sa;
	
	if (!gr_clip_xywh(dst, &dx, &dy, &dw, &dh)) return;
	if (!gr_clip_xywh(src, &sx, &sy, &sw, &sh)) return;
	
	sp = GETOFFSET_PIXEL(src, sx, sy);
	dp = GETOFFSET_PIXEL(dst, dx, dy);
	sa = GETOFFSET_ALPHA(src, sx, sy);
	
	a1  = (float)sw / (float)dw;
	a2  = (float)sh / (float)dh;
	// src width と dst width が同じときに問題があるので+1
	row = g_new0(int, dw+1);
	// 1おおきくして初期化しないと col[dw-1]とcol[dw]が同じになる
	// 可能性がある。
	col = g_new0(int, dh+1);
	
	for (yd = 0.0, y = 0; y < dh; y++) {
		col[y] = yd; yd += a2;
	}
	
	for (xd = 0.0, x = 0; x < dw; x++) {
		row[x] = xd; xd += a1;
	}

	switch(dst->depth) {
	case 15:
	{
		WORD *yls, *yld;
		BYTE *yla;
		
		for (y = 0; y < dh; y++) {
			yls = (WORD *)(sp + *(y + col) * src->bytes_per_line);
			yld = (WORD *)(dp +   y        * dst->bytes_per_line);
			yla = (BYTE *)(sa + *(y + col) * src->width);
			for (x = 0; x < dw; x++) {
				*(yld + x) = ALPHABLEND15(*(yls+ *(row + x)), *(yld+x), *(yla+*(row+x)));
			}
			while(*(col + y) == *(col + y + 1)) {
				yld += dst->width;
				for (x = 0; x < dw; x++) {
					*(yld + x) = ALPHABLEND15(*(yls+ *(row+x)), *(yld+x), *(yla+*(row+x)));
				}
				y++;
			}
		}
		break;
	}
	case 16:
	{
		WORD *yls, *yld;
		BYTE *yla;
		
		for (y = 0; y < dh; y++) {
			yls = (WORD *)(sp + *(y + col) * src->bytes_per_line);
			yld = (WORD *)(dp +   y        * dst->bytes_per_line);
			yla = (BYTE *)(sa + *(y + col) * src->width);
			for (x = 0; x < dw; x++) {
				*(yld + x) = ALPHABLEND16(*(yls+ *(row + x)), *(yld+x), *(yla+*(row+x)));
			}
			while(*(col + y) == *(col + y + 1)) {
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
		DWORD *yls, *yld;
		BYTE  *yla;
		
		for (y = 0; y < dh; y++) {
			yls = (DWORD *)(sp + *(y + col) * src->bytes_per_line);
			yld = (DWORD *)(dp +   y        * dst->bytes_per_line);
			yla = (BYTE  *)(sa + *(y + col) * src->width);
			for (x = 0; x < dw; x++) {
				*(yld + x) = ALPHABLEND24(*(yls+ *(row + x)), *(yld+x), *(yla+*(row+x)));
			}
			while(*(col + y) == *(col + y + 1)) {
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
	
	g_free(row);
	g_free(col);
}

#include "graph2.c"
