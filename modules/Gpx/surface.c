#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>

#include "portab.h"
#include "surface.h"
#include "system.h"

static surface_t *create(int width, int height, bool has_pixel, bool has_alpha) {
	surface_t *s = calloc(1, sizeof(surface_t));
	s->width = width;
	s->height = height;

	if (has_pixel) {
		s->sdl_surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_RGB888);
	}
	
	if (has_alpha) {
		s->alpha = calloc(width * (height +1), sizeof(uint8_t));
	}
	
	return s;
}

// Create a surface with pixel and alpha data
surface_t *sf_create_surface(int width, int height) {
	return create(width, height, true, true);
}

// Create a surface with alpha data only
surface_t *sf_create_alpha(int width, int height) {
	return create(width, height, false, true);
}

// Create a surface with pixel data only
surface_t *sf_create_pixel(int width, int height) {
	return create(width, height, true, false);
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
