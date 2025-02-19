#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>

#include "portab.h"
#include "surface.h"
#include "system.h"

static surface_t *create(int width, int height, int depth, bool has_pixel, bool has_alpha) {
	surface_t *s = calloc(1, sizeof(surface_t));
	
	s->width = width;
	s->height = height;
	
	s->depth = depth;
	
	if (has_pixel) {
		uint32_t format = 0;
		switch (s->depth) {
		case 16:
			format = SDL_PIXELFORMAT_RGB565;
			break;
		case 24:
		case 32:
			format = SDL_PIXELFORMAT_RGB888;
			depth = 32;
			break;
		default:
			SYSERROR("depth %d is not supported", s->depth);
		}
		s->sdl_surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, depth, format);
	}
	
	if (has_alpha) {
		s->alpha = calloc(width * (height +1), sizeof(uint8_t));
	}
	
	return s;
}

/**
 * surfaceの生成 (pixel + alpha)
 * @param width:  surfaceの幅
 * @param height: surfaceの高さ
 * @param depth:  surfaceのpixelのBPP (8|16|24|32), alphaは8固定
 * @return 生成した surface オブジェクト
 */
surface_t *sf_create_surface(int width, int height, int depth) {
	return create(width, height, depth, true, true);
}

/**
 * surfaceの生成 (alphaのみ)
 * @param width:  surfaceの幅
 * @param height: surfaceの高さ
 * @return 生成した surface オブジェクト
 */
surface_t *sf_create_alpha(int width, int height) {
	return create(width, height, 8, false, true);
}

/**
 * surfaceの生成 (pixelのみ)
 * @param width:  surfaceの幅
 * @param height: surfaceの高さ
 * @param depth:  surfaceのBPP(8|16|24|32)
 * @return 生成した surface オブジェクト
 */
surface_t *sf_create_pixel(int width, int height, int depth) {
	return create(width, height, depth, true, false);
}

/**
 * surfaceオブジェクトの開放
 * @param s: 開放するオブジェクト
 * @return:  なし
 */
void sf_free(surface_t *s) {
	if (s == NULL) return;
	if (s->sdl_surface) SDL_FreeSurface(s->sdl_surface);
	if (s->alpha) free(s->alpha);
	free(s);
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
	
	sf = malloc(sizeof(surface_t));
	memcpy(sf, in, sizeof(surface_t));
	
	if (in->sdl_surface) {
		sf->sdl_surface = SDL_ConvertSurface(in->sdl_surface, in->sdl_surface->format, 0);
	}
	
	if (in->alpha) {
		len = sf->width * sf->height;
		sf->alpha = malloc(sizeof(uint8_t) * (len + sf->width));
		memcpy(sf->alpha, in->alpha, len);
	}
	
	return sf;
}
