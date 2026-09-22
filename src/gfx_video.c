/*
 * gfx_video.c  SDL video init
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
/* $Id: gfx_video.c,v 1.11 2003/01/04 17:01:02 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include "sdl_compat.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include "portab.h"
#include "system.h"
#include "gfx.h"
#include "gfx_private.h"
#include "xsystem35.h"
#include "image.h"

static SDL_Window *gfx_window;
SDL_Renderer *gfx_renderer;
SDL_Texture *gfx_texture;
SDL_Surface *main_surface; // offscreen surface
SDL_Palette *gfx_palette;
surface_t *gfx_dibinfo;
int view_w;
int view_h;
static bool gfx_fullscreen;
static bool gfx_integer_scaling;

static void window_init(const char *render_driver);
static void makeDIB(int width, int height, int depth);

/* SDL の初期化 */
int gfx_Initialize(const char *render_driver) {
	window_init(render_driver);
	
	/* offscreen Pixmap */
	makeDIB(SYS35_DEFAULT_WIDTH, SYS35_DEFAULT_HEIGHT, SYS35_DEFAULT_DEPTH);

	gfx_setViewSize(SYS35_DEFAULT_WIDTH, SYS35_DEFAULT_HEIGHT);

#ifdef __EMSCRIPTEN__
	// Prevent SDL from calling emscripten_exit_fullscreen on visibilitychange
	emscripten_set_visibilitychange_callback(NULL, 0, NULL);
#endif

	return 0;
}

void gfx_Remove(void) {
	if (gfx_palette)
		sdl_destroy_palette(gfx_palette);
	if (main_surface)
		sdl_destroy_surface(main_surface);
	if (gfx_renderer)
		SDL_DestroyRenderer(gfx_renderer);
	SDL_Quit();
}

/* name is UTF-8 */
#ifdef __EMSCRIPTEN__
EM_JS(void, gfx_setWindowTitle, (char *name), {
	xsystem35.shell.setWindowTitle(UTF8ToString(name));
});
#else
void gfx_setWindowTitle(char *name) {
	SDL_SetWindowTitle(gfx_window, name);
}
#endif

static void window_init(const char *render_driver) {
	if (render_driver)
		SDL_SetHint(SDL_HINT_RENDER_DRIVER, render_driver);
	SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
#if XSYSTEM35_SDL_VERSION == 2
	SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0");
#endif

	SDL_Init(SDL_INIT_VIDEO);
	
#ifdef __EMSCRIPTEN__
	// Stop SDL from calling emscripten_sleep() in functions that are called
	// indirectly, which does not work with ASYNCIFY_IGNORE_INDIRECT=1. For
	// details, see https://github.com/emscripten-core/emscripten/issues/10746.
	SDL_SetHint(SDL_HINT_EMSCRIPTEN_ASYNCIFY, "0");

	const char *title = NULL;  // Don't let SDL change document.title.
#else
	const char title[] = "XSystem35 Version " VERSION;
#endif

#ifdef __ANDROID__
	SDL_SetHint(SDL_HINT_ANDROID_TRAP_BACK_BUTTON, "1");
#endif

	gfx_window = sdl_create_window(title, SYS35_DEFAULT_WIDTH,
		SYS35_DEFAULT_HEIGHT, SDL_WINDOW_RESIZABLE);
	gfx_renderer = sdl_create_renderer(gfx_window);
	SDL_SetRenderDrawColor(gfx_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	gfx_palette = sdl_create_palette(256);
}

static void makeDIB(int width, int height, int depth) {
	
	if (main_surface) {
		sdl_destroy_surface(main_surface);
	}

	uint32_t format = 0;
	switch (depth) {
	case 8:
		format = SDL_PIXELFORMAT_INDEX8;
		break;
	case 16:
		format = SDL_PIXELFORMAT_RGB565;
		break;
	case 24:
		format = SDL_PIXELFORMAT_RGB888;
		// Graphic routines in modules/ assume 4 bytes/pixel mode for 24-bit surfaces.
		depth = 32;
		break;
	default:
		SYSERROR("invalid pixel depth %d", depth);
	}

	main_surface = sdl_create_surface(width, height, depth, format);
	
	if (sdl_surface_bits_per_pixel(main_surface) == 8) {
		sdl_set_surface_palette(main_surface, gfx_palette);
	}

	if (gfx_dibinfo) {
		free(gfx_dibinfo);
	}
	
	gfx_dibinfo = calloc(1, sizeof(surface_t));
	gfx_dibinfo->width  = width;
	gfx_dibinfo->height = height;
	gfx_dibinfo->alpha  = NULL;
	gfx_dibinfo->sdl_surface = main_surface;
	
	image_setdepth(sdl_surface_bits_per_pixel(main_surface));
}

/* offscreen の設定 */
void gfx_setWorldSize(int width, int height, int depth) {
	makeDIB(width, height, depth);
	SDL_FillRect(main_surface, NULL, 0);
}

void gfx_getViewSize(int *width, int *height) {
	if (width)
		*width = view_w;
	if (height)
		*height = view_h;
}

SDL_Window *gfx_getWindow(void) {
	return gfx_window;
}

static SDL_Point view_to_window_point(int x, int y) {
	return sdl_render_coordinates_to_window(gfx_renderer, x, y);
}

void gfx_warpMouse(int x, int y) {
	SDL_Point point = view_to_window_point(x, y);
	SDL_WarpMouseInWindow(gfx_window, point.x, point.y);
}

SDL_Rect gfx_viewToWindowRect(SDL_Rect rect) {
	SDL_Point tl = view_to_window_point(rect.x, rect.y);
	SDL_Point br = view_to_window_point(rect.x + rect.w, rect.y + rect.h);
	return (SDL_Rect){tl.x, tl.y, br.x - tl.x, br.y - tl.y};
}

/*  DIBの取得 */
surface_t *gfx_getDIB(void) {
	return gfx_dibinfo;
}

SDL_Surface *gfx_createSurfaceView(SDL_Surface *sf, int x, int y, int w, int h) {
	uint8_t *pixels = sf->pixels;
	pixels += y * sf->pitch + x * sdl_surface_bytes_per_pixel(sf);
	SDL_Surface *view = sdl_create_surface_from(pixels, w, h,
		sdl_surface_bits_per_pixel(sf), sf->pitch, sdl_surface_format(sf));
	SDL_Palette *palette = sdl_get_surface_palette(sf);
	if (palette)
		sdl_set_surface_palette(view, palette);
	return view;
}

void gfx_setFullscreen(bool on) {
#ifndef __EMSCRIPTEN__
	if (on == gfx_fullscreen)
		return;
	sdl_set_window_fullscreen(gfx_window, on);
	gfx_fullscreen = on;
#endif
}

bool gfx_isFullscreen(void) {
	return gfx_fullscreen;
}

void gfx_raiseWindow(void) {
	SDL_RaiseWindow(gfx_window);
}

void gfx_setViewSize(int w, int h) {
	if (w == view_w && h == view_h) return;

	view_w = w;
	view_h = h;

#ifndef __ANDROID__
	sdl_set_window_content_size(gfx_window, w, h);
#endif
	sdl_set_render_logical_presentation(
		gfx_renderer, w, h, gfx_integer_scaling);
	if (gfx_texture)
		SDL_DestroyTexture(gfx_texture);
	gfx_texture = SDL_CreateTexture(
		gfx_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, w, h);
#ifdef __ANDROID__
	SDL_SetTextureScaleMode(gfx_texture, SDL_SCALEMODE_LINEAR);
#endif

#ifdef __EMSCRIPTEN__
	EM_ASM( xsystem35.shell.windowSizeChanged(); );
#endif
}

void gfx_setIntegerScaling(bool enable) {
	gfx_integer_scaling = enable;
	sdl_set_render_logical_presentation(
		gfx_renderer, view_w, view_h, gfx_integer_scaling);
}

bool EMSCRIPTEN_KEEPALIVE save_screenshot(const char* path) {
	SDL_Rect *r = &nact->ags.view_area;
	SDL_Surface *view = gfx_createSurfaceView(main_surface, r->x, r->y, r->w, r->h);
	bool ok = SDL_SaveBMP(view, path) == 0;
	sdl_destroy_surface(view);
	return ok;
}
