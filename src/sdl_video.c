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
#include "font.h"
#include "joystick.h"
#include "image.h"

static void window_init(void);
static void makeDIB(int width, int height, int depth);

struct sdl_private_data *sdl_videodev;


/* SDL の初期化 */
int sdl_Initilize(void) {
	sdl_videodev = calloc(1, sizeof(struct sdl_private_data));

	/* make topleve window */
	window_init();
	
	/* offscreen Pixmap */
	makeDIB(SYS35_DEFAULT_WIDTH, SYS35_DEFAULT_HEIGHT, SYS35_DEFAULT_DEPTH);
	
	/* init cursor */
	sdl_cursor_init();
	
	sdl_setWindowSize(0, 0, SYS35_DEFAULT_WIDTH, SYS35_DEFAULT_HEIGHT);

	sdl_shadow_init();
	
#ifdef __EMSCRIPTEN__
	// Prevent SDL from calling emscripten_exit_fullscreen on visibilitychange
	emscripten_set_visibilitychange_callback(NULL, 0, NULL);
#endif

	joy_open();
	return 0;
}

void sdl_Remove(void) {
	if (sdl_videodev == NULL) return;

	if (sdl_display) {
		NOTICE("Now SDL shutdown ... ");
		
		SDL_FreeSurface(sdl_dib);

		SDL_DestroyRenderer(sdl_renderer);
		
		joy_close();
		
		SDL_Quit();
		
		NOTICE("Done!\n");
	}
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

/* Visual に応じて Window を生成する */
static void window_init(void) {
	
	SDL_Init(SDL_INIT_VIDEO
#ifdef HAVE_SDLJOY
		 |SDL_INIT_JOYSTICK
#endif
		);
	
#ifdef __EMSCRIPTEN__
	// Stop SDL from calling emscripten_sleep() in functions that are called
	// indirectly, which does not work with ASYNCIFY_IGNORE_INDIRECT=1. For
	// details, see https://github.com/emscripten-core/emscripten/issues/10746.
	SDL_SetHint(SDL_HINT_EMSCRIPTEN_ASYNCIFY, "0");
#endif

#ifdef __ANDROID__
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
	Uint32 flags = SDL_WINDOW_FULLSCREEN;
#else
	Uint32 flags = SDL_WINDOW_RESIZABLE;
#endif
	sdl_window = SDL_CreateWindow("XSystem3.5 Version "VERSION,
								  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
								  SYS35_DEFAULT_WIDTH, SYS35_DEFAULT_HEIGHT,
								  flags);
	sdl_renderer = SDL_CreateRenderer(sdl_window, -1, 0);
}

static void makeDIB(int width, int height, int depth) {
	
	if (sdl_dib) {
		SDL_FreeSurface(sdl_dib);
	}
	
#ifdef ENABLE_MODULES
	// Graphic routines in modules/ assume 4 bytes/pixel mode for 24-bit surfaces.
	if (depth == 24)
		depth = 32;
#endif

	sdl_dib = SDL_CreateRGBSurface(0, width, height, depth, 0, 0, 0, 0);
	
	if (sdl_dib->format->BitsPerPixel == 8) {
		memset(sdl_dib->format->palette->colors, 0, sizeof(SDL_Color)*256);
	}

#if 0
	printf("DIB depth %d\n", sdl_dib->format->BitsPerPixel);
	printf("  R %04x G %04x B %04x A %04x\n",
	       sdl_dib->format->Rmask, sdl_dib->format->Gmask,
	       sdl_dib->format->Bmask, sdl_dib->format->Amask);
#endif
	if (depth > 8) {
		sdl_white = (sdl_dib->format->Rmask | sdl_dib->format->Gmask | sdl_dib->format->Bmask);
	}
	
	if (sdl_dibinfo) {
		free(sdl_dibinfo);
	}
	
	sdl_dibinfo = calloc(1, sizeof(agsurface_t));
	sdl_dibinfo->depth           = sdl_dib->format->BitsPerPixel;
	sdl_dibinfo->bytes_per_pixel = sdl_dib->format->BytesPerPixel;
	sdl_dibinfo->bytes_per_line  = sdl_dib->pitch;
	sdl_dibinfo->pixel  = sdl_dib->pixels;
	sdl_dibinfo->width  = width;
	sdl_dibinfo->height = height;
	sdl_dibinfo->alpha  = NULL;
	
	image_setdepth(sdl_dibinfo->depth);
}

void sdl_setFontDevice(struct _FONT *f) {
	sdl_font = f;
}

/* offscreen の設定 */
void sdl_setWorldSize(int width, int height, int depth) {
	makeDIB(width, height, depth);
	SDL_FillRect(sdl_dib, NULL, 0);
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
agsurface_t *sdl_getDIB(void) {
	return sdl_dibinfo;
}

/* AutoRepeat の設定 */
void sdl_setAutoRepeat(boolean bool) {
	if (bool) {
		// SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	} else {
		// SDL_EnableKeyRepeat(0, 0);
	}
}

#ifdef __EMSCRIPTEN__

void* EMSCRIPTEN_KEEPALIVE sdl_getDisplaySurface() {
	if (SDL_MUSTLOCK(sdl_display))
		return NULL;
	return sdl_display->pixels;
}

#endif
