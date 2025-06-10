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
#include <SDL.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include "portab.h"
#include "system.h"
#include "gfx.h"
#include "gfx_private.h"
#include "sdl_core.h"
#include "xsystem35.h"
#include "image.h"

SDL_Window *gfx_window;
SDL_Renderer *gfx_renderer;
SDL_Texture *gfx_texture;
SDL_Surface *main_surface; // offscreen surface
SDL_Palette *gfx_palette;
surface_t *gfx_dibinfo;
int view_w;
int view_h;
bool gfx_dirty;
bool gfx_fullscreen;
bool (*sdl_custom_event_handler)(const SDL_Event *);

static void window_init(const char *render_driver);
static void makeDIB(int width, int height, int depth);

/* SDL の初期化 */
int gfx_Initialize(const char *render_driver) {
	window_init(render_driver);
	
	/* offscreen Pixmap */
	makeDIB(SYS35_DEFAULT_WIDTH, SYS35_DEFAULT_HEIGHT, SYS35_DEFAULT_DEPTH);

	sdl_event_init();

	gfx_setWindowSize(SYS35_DEFAULT_WIDTH, SYS35_DEFAULT_HEIGHT);

#ifdef __EMSCRIPTEN__
	// Prevent SDL from calling emscripten_exit_fullscreen on visibilitychange
	emscripten_set_visibilitychange_callback(NULL, 0, NULL);
#endif

	return 0;
}

void gfx_Remove(void) {
	if (gfx_palette)
		SDL_FreePalette(gfx_palette);
	if (main_surface)
		SDL_FreeSurface(main_surface);
	if (gfx_renderer)
		SDL_DestroyRenderer(gfx_renderer);
	sdl_event_remove();
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
	SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0");

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
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
	Uint32 flags = SDL_WINDOW_FULLSCREEN;
#else
	Uint32 flags = SDL_WINDOW_RESIZABLE;
#endif
	gfx_window = SDL_CreateWindow(
		title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		SYS35_DEFAULT_WIDTH, SYS35_DEFAULT_HEIGHT, flags);
	gfx_renderer = SDL_CreateRenderer(gfx_window, -1, 0);
	SDL_SetRenderDrawColor(gfx_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	gfx_palette = SDL_AllocPalette(256);
}

static void makeDIB(int width, int height, int depth) {
	
	if (main_surface) {
		SDL_FreeSurface(main_surface);
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

	main_surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, depth, format);
	
	if (main_surface->format->BitsPerPixel == 8) {
		SDL_SetSurfacePalette(main_surface, gfx_palette);
	}

	if (gfx_dibinfo) {
		free(gfx_dibinfo);
	}
	
	gfx_dibinfo = calloc(1, sizeof(surface_t));
	gfx_dibinfo->width  = width;
	gfx_dibinfo->height = height;
	gfx_dibinfo->alpha  = NULL;
	gfx_dibinfo->sdl_surface = main_surface;
	
	image_setdepth(main_surface->format->BitsPerPixel);
}

/* offscreen の設定 */
void gfx_setWorldSize(int width, int height, int depth) {
	makeDIB(width, height, depth);
	SDL_FillRect(main_surface, NULL, 0);
}

/* display の size と depth の取得 */
void gfx_getWindowInfo(int *width, int *height, int *depth) {
	SDL_DisplayMode dm;
	SDL_GetCurrentDisplayMode(0, &dm);
	if (width)  *width  = dm.w;
	if (height) *height = dm.h;
	if (depth)  *depth  = SDL_BITSPERPIXEL(dm.format);
}

/*  DIBの取得 */
surface_t *gfx_getDIB(void) {
	return gfx_dibinfo;
}

SDL_Surface *gfx_createSurfaceView(SDL_Surface *sf, int x, int y, int w, int h) {
	uint8_t *pixels = sf->pixels;
	pixels += y * sf->pitch + x * sf->format->BytesPerPixel;
	SDL_Surface *view = SDL_CreateRGBSurfaceWithFormatFrom(
		pixels, w, h, sf->format->BitsPerPixel, sf->pitch, sf->format->format);
	if (sf->format->palette)
		SDL_SetSurfacePalette(view, sf->format->palette);
	return view;
}

/* AutoRepeat の設定 */
void sdl_setAutoRepeat(bool enable) {
	if (enable) {
		// SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	} else {
		// SDL_EnableKeyRepeat(0, 0);
	}
}

void gfx_setFullscreen(bool on) {
#ifndef __EMSCRIPTEN__
	if (on == gfx_fullscreen)
		return;
	SDL_SetWindowFullscreen(gfx_window, on ? SDL_WINDOW_FULLSCREEN_DESKTOP: 0);
	gfx_fullscreen = on;
#endif
}

bool gfx_isFullscreen(void) {
	return gfx_fullscreen;
}

void gfx_raiseWindow(void) {
	SDL_RaiseWindow(gfx_window);
}

void gfx_setWindowSize(int w, int h) {
	if (w == view_w && h == view_h) return;

	view_w = w;
	view_h = h;

#ifndef __ANDROID__
	SDL_SetWindowSize(gfx_window, w, h);
#endif
	SDL_RenderSetLogicalSize(gfx_renderer, w, h);
	if (gfx_texture)
		SDL_DestroyTexture(gfx_texture);
	gfx_texture = SDL_CreateTexture(
		gfx_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, w, h);

#ifdef __EMSCRIPTEN__
	EM_ASM( xsystem35.shell.windowSizeChanged(); );
#endif
}

void gfx_setIntegerScaling(bool enable) {
	SDL_RenderSetIntegerScale(gfx_renderer, enable);
}

bool EMSCRIPTEN_KEEPALIVE save_screenshot(const char* path) {
	SDL_Rect *r = &nact->ags.view_area;
	SDL_Surface *view = gfx_createSurfaceView(main_surface, r->x, r->y, r->w, r->h);
	bool ok = SDL_SaveBMP(view, path) == 0;
	SDL_FreeSurface(view);
	return ok;
}
