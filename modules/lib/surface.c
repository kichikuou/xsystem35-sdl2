#include "config.h"

#include <stdio.h>
#include <glib.h>
#include <string.h>

#include "portab.h"
#include "surface.h"
#include "system.h"

static surface_t *create(int width, int height, int depth, boolean has_pixel, boolean has_alpha) {
	surface_t *s = g_new0(surface_t, 1);
	
	s->width = width;
	s->height = height;
	s->has_alpha = has_alpha;
	s->has_pixel = has_pixel;
	
	s->bytes_per_line = width;
	s->bytes_per_pixel = 1;
	s->depth = depth;
	
	if (s->has_pixel) {
		switch (s->depth) {
		case 8:
			s->pixel = g_new0(BYTE, width * (height +1));
			s->bytes_per_line = width;
			s->bytes_per_pixel = 1;
			break;
		case 15:
		case 16:
			s->pixel = g_new0(BYTE, width * (height +1) * 2);
			s->bytes_per_line = width * 2;
			s->bytes_per_pixel = 2;
			break;
		case 24:
		case 32:
			s->pixel = g_new0(BYTE, width * (height +1) * 4);
			s->bytes_per_line = width * 4;
			s->bytes_per_pixel = 4;
			break;
		default:
			WARNING("depth %d is not supported\n", s->depth);
		}
	} else {
		s->pixel = NULL;
	}
	
	if (s->has_alpha) {
		s->alpha = g_new0(BYTE, width * (height +1));
	}
	
	return s;
}

/**
 * surfaceの生成 (pixel + alpha)
 * @param width:  surfaceの幅
 * @param height: surfaceの高さ
 * @param depth:  surfaceのpixelのBPP (8|15|16|24|32), alphaは8固定
 * @return 生成した surface オブジェクト
 */
surface_t *sf_create_surface(int width, int height, int depth) {
	return create(width, height, depth, TRUE, TRUE);
}

/**
 * surfaceの生成 (alphaのみ)
 * @param width:  surfaceの幅
 * @param height: surfaceの高さ
 * @return 生成した surface オブジェクト
 */
surface_t *sf_create_alpha(int width, int height) {
	return create(width, height, 8, FALSE, TRUE);
}

/**
 * surfaceの生成 (pixelのみ)
 * @param width:  surfaceの幅
 * @param height: surfaceの高さ
 * @param depth:  surfaceのBPP(8|15|16|24|32)
 * @return 生成した surface オブジェクト
 */
surface_t *sf_create_pixel(int width, int height, int depth) {
	return create(width, height, depth, TRUE, FALSE);
}

/**
 * surfaceオブジェクトの開放
 * @param s: 開放するオブジェクト
 * @return:  なし
 */
void sf_free(surface_t *s) {
	if (s == NULL) return;
	if (s->pixel) g_free(s->pixel);
	if (s->alpha) g_free(s->alpha);
	g_free(s);
}

/**
 * surface の複製を作成(dupulicate)
 * @param in: 複製もと
 * @return  : 複製したsurface
 */
surface_t *sf_dup(surface_t *in) {
	surface_t *sf;
	int len;
	
	if (in == NULL) return NULL;
	
	sf = g_new(surface_t, 1);
	memcpy(sf, in, sizeof(surface_t));
	
	if (in->has_pixel) {
		len = sf->bytes_per_line * sf->height;
		sf->pixel = g_new(BYTE, len + sf->bytes_per_line);
		memcpy(sf->pixel, in->pixel, len);
	}
	
	if (in->has_alpha) {
		len = sf->width * sf->height;
		sf->alpha = g_new(BYTE, len + sf->width);
		memcpy(sf->alpha, in->alpha, len);
	}
	
	return sf;
}

/**
 * surface全体のコピー
 * @param dst: コピー先 surface
 * @param src: コピー元 surface
 * @return なし
 */
void sf_copyall(surface_t *dst, surface_t *src) {
	int len;
	
	if (src == NULL || dst == NULL) return;
	
	if (src->width != dst->width) return;
	
	if (src->height != dst->height) return;
	
	if (src->bytes_per_pixel != dst->bytes_per_pixel) return;
	
	if (src->has_alpha && dst->has_alpha) {
		len = src->width * src->height;
		memcpy(dst->alpha, src->alpha, len);
	}
	
	if (src->has_pixel && dst->has_pixel) {
		len = src->bytes_per_line * src->height;
		memcpy(dst->pixel, src->pixel, len);
	}
}

/**
 * surface の複製
 * @param in: 複製もと
 * @param copypixel: pixelをコピーするか
 * @param copyalpha: alpha pixel をコピーするか
 * @return: 複製した surface
 */
surface_t *sf_dup2(surface_t *in, boolean copypixel, boolean copyalpha) {
	surface_t *sf;
	int len;
	
	if (in == NULL) return NULL;
	
	sf = g_new(surface_t, 1);
	memcpy(sf, in, sizeof(surface_t));
	
	if (in->has_pixel) {
		len = sf->bytes_per_line * sf->height;
		sf->pixel = g_new(BYTE, len + sf->bytes_per_line);
		if (copypixel) {
			memcpy(sf->pixel, in->pixel, len);
		}
	}
	
	if (in->has_alpha) {
		len = sf->width * sf->height;
		sf->alpha = g_new(BYTE, len + sf->width);
		if (copyalpha) {
			memcpy(sf->alpha, in->alpha, len);
		}
	}
	
	return sf;
}

