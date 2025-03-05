/*
 * sdl_video.c  SDL video init
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
/* $Id: sdl_video.c,v 1.11 2003/01/04 17:01:02 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <SDL.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

#include "portab.h"
#include "system.h"
#include "sdl_core.h"
#include "sdl_private.h"
#include "xsystem35.h"
#include "image.h"

SDL_Window *sdl_window;
SDL_Renderer *sdl_renderer;
SDL_Texture *sdl_texture;
SDL_Surface *main_surface; // offscreen surface
SDL_Color sdl_col[256]; // color palette
surface_t *sdl_dibinfo;
int view_w;
int view_h;
bool sdl_dirty;
bool sdl_fs_on;
bool (*sdl_custom_event_handler)(const SDL_Event *);

static void window_init(const char *render_driver);
static void makeDIB(int width, int height, int depth);

static int joy_device_index = -1;

static SDL_Joystick *js;

bool sdl_joy_open(int index) {
	if (js)
		return false;

	js = SDL_JoystickOpen(index);
	if (!js)
		return false;

	const char *name = SDL_JoystickName(js);
	int axes = SDL_JoystickNumAxes(js);
	int buttons = SDL_JoystickNumButtons(js);
	SDL_JoystickEventState(SDL_ENABLE);
	NOTICE("SDL joystick '%s' %d axes %d buttons", name, axes, buttons);
	return true;
}

static bool joy_open(void) {
	if (joy_device_index >= 0)
		return sdl_joy_open(joy_device_index);

	for (int i = 0; i < SDL_NumJoysticks(); i++) {
		if (sdl_joy_open(i))
			return true;
	}
	return false;
}

/* SDL の初期化 */
int sdl_Initialize(const char *render_driver) {
	window_init(render_driver);
	
	/* offscreen Pixmap */
	makeDIB(SYS35_DEFAULT_WIDTH, SYS35_DEFAULT_HEIGHT, SYS35_DEFAULT_DEPTH);
	
	sdl_event_init();
	sdl_cursor_init();
	
	sdl_setWindowSize(SYS35_DEFAULT_WIDTH, SYS35_DEFAULT_HEIGHT);

#ifdef __EMSCRIPTEN__
	// Prevent SDL from calling emscripten_exit_fullscreen on visibilitychange
	emscripten_set_visibilitychange_callback(NULL, 0, NULL);
#endif

	joy_open();
	return 0;
}

void sdl_Remove(void) {
	if (main_surface)
		SDL_FreeSurface(main_surface);
	if (sdl_renderer)
		SDL_DestroyRenderer(sdl_renderer);
	if (js)
		SDL_JoystickClose(js);
	SDL_Quit();
}

/* name is UTF-8 */
#ifdef __EMSCRIPTEN__
EM_JS(void, sdl_setWindowTitle, (char *name), {
	xsystem35.shell.setWindowTitle(UTF8ToString(name));
});
#else
void sdl_setWindowTitle(char *name) {
	SDL_SetWindowTitle(sdl_window, name);
}
#endif

static void window_init(const char *render_driver) {
	if (render_driver)
		SDL_SetHint(SDL_HINT_RENDER_DRIVER, render_driver);
	SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
	SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0");

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
	
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
	sdl_window = SDL_CreateWindow(
		title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		SYS35_DEFAULT_WIDTH, SYS35_DEFAULT_HEIGHT, flags);
	sdl_renderer = SDL_CreateRenderer(sdl_window, -1, 0);
	SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
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
		memset(main_surface->format->palette->colors, 0, sizeof(SDL_Color)*256);
	}

	if (sdl_dibinfo) {
		free(sdl_dibinfo);
	}
	
	sdl_dibinfo = calloc(1, sizeof(surface_t));
	sdl_dibinfo->width  = width;
	sdl_dibinfo->height = height;
	sdl_dibinfo->alpha  = NULL;
	sdl_dibinfo->sdl_surface = main_surface;
	
	image_setdepth(main_surface->format->BitsPerPixel);
}

/* offscreen の設定 */
void sdl_setWorldSize(int width, int height, int depth) {
	makeDIB(width, height, depth);
	SDL_FillRect(main_surface, NULL, 0);
}

/* display の size と depth の取得 */
void sdl_getWindowInfo(DispInfo *info) {
	SDL_DisplayMode dm;
	SDL_GetCurrentDisplayMode(0, &dm);
	info->width  = dm.w;
	info->height = dm.h;
	info->depth  = SDL_BITSPERPIXEL(dm.format);
}

/*  DIBの取得 */
surface_t *sdl_getDIB(void) {
	return sdl_dibinfo;
}

SDL_Surface *sdl_createSurfaceView(SDL_Surface *sf, int x, int y, int w, int h) {
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

void sdl_setFullscreen(bool on) {
#ifndef __EMSCRIPTEN__
	if (on == sdl_fs_on)
		return;
	SDL_SetWindowFullscreen(sdl_window, on ? SDL_WINDOW_FULLSCREEN_DESKTOP: 0);
	sdl_fs_on = on;
#endif
}

bool sdl_isFullscreen(void) {
	return sdl_fs_on;
}

void sdl_raiseWindow(void) {
	SDL_RaiseWindow(sdl_window);
}

void sdl_setWindowSize(int w, int h) {
	if (w == view_w && h == view_h) return;

	view_w = w;
	view_h = h;

#ifndef __ANDROID__
	SDL_SetWindowSize(sdl_window, w, h);
#endif
	SDL_RenderSetLogicalSize(sdl_renderer, w, h);
	if (sdl_texture)
		SDL_DestroyTexture(sdl_texture);
	sdl_texture = SDL_CreateTexture(
		sdl_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, w, h);

#ifdef __EMSCRIPTEN__
	EM_ASM( xsystem35.shell.windowSizeChanged(); );
#endif
}

void sdl_showMessageBox(enum messagebox_type type, const char* title_utf8, const char* message_utf8) {
	uint32_t flags = 0;
	switch (type) {
	case MESSAGEBOX_ERROR: flags = SDL_MESSAGEBOX_ERROR; break;
	case MESSAGEBOX_WARNING: flags = SDL_MESSAGEBOX_WARNING; break;
	case MESSAGEBOX_INFO: flags = SDL_MESSAGEBOX_INFORMATION; break;
	}
	SDL_ShowSimpleMessageBox(flags, title_utf8, message_utf8, sdl_window);
}

void sdl_setJoyDeviceIndex(int index) {
	joy_device_index = index;
}

void sdl_setIntegerScaling(bool enable) {
	SDL_RenderSetIntegerScale(sdl_renderer, enable);
}

bool EMSCRIPTEN_KEEPALIVE save_screenshot(const char* path) {
	SDL_Rect *r = &nact->ags.view_area;
	SDL_Surface *view = sdl_createSurfaceView(main_surface, r->x, r->y, r->w, r->h);
	bool ok = SDL_SaveBMP(view, path) == 0;
	SDL_FreeSurface(view);
	return ok;
}
