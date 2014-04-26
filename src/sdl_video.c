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
#include <SDL/SDL.h>
#include <glib.h>

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

boolean RawKeyInfo[256];


/* SDL の初期化 */
int sdl_Initilize(void) {
	sdl_videodev = g_new0(struct sdl_private_data, 1);

	/* make topleve window */
	window_init();
	
	/* offscreen Pixmap */
	makeDIB(SYS35_DEFAULT_WIDTH, SYS35_DEFAULT_HEIGHT, SYS35_DEFAULT_DEPTH);
	
	/* title */
	SDL_WM_SetCaption("XSystem3.5 Version "VERSION, NULL);
	
	/* init cursor */
	sdl_cursor_init();
	
	sdl_vm_init();
	
	memset(RawKeyInfo, 0, sizeof(RawKeyInfo));

	sdl_setWindowSize(0, 0, SYS35_DEFAULT_WIDTH, SYS35_DEFAULT_HEIGHT);

	sdl_shadow_init();
	
	joy_open();
	return 0;
}

void sdl_Remove(void) {
	if (sdl_videodev == NULL) return;

	if (sdl_display) {
		NOTICE("Now SDL shutdown ... ");
		
		SDL_FreeSurface(sdl_dib);
		
		joy_close();
		
		SDL_Quit();
		
		NOTICE("Done!\n");
	}
}

/* name is EUC */
void sdl_setWindowTitle(char *name) {
	SDL_WM_SetCaption(name, NULL);
}

/* Visual に応じて Window を生成する */
static void window_init(void) {
	char s[256];
	
	SDL_Init(SDL_INIT_VIDEO
#ifdef HAVE_SDLJOY
		 |SDL_INIT_JOYSTICK
#endif
		);
	
	sdl_vinfo = SDL_GetVideoInfo();
	
	SDL_VideoDriverName(s, 256);
	
	NOTICE("Video Info %s\n", s);
	NOTICE("  hw_available %d\n", sdl_vinfo->hw_available);
	NOTICE("  wm_available %d\n", sdl_vinfo->wm_available);
	NOTICE("  Accelerated blits HW --> HW %d\n", sdl_vinfo->blit_hw);
	NOTICE("  Accelerated blits with Colorkey %d\n", sdl_vinfo->blit_hw_CC);
	NOTICE("  Accelerated blits with Alpha %d\n", sdl_vinfo->blit_hw_A);
	NOTICE("  Accelerated blits SW --> HW %d\n", sdl_vinfo->blit_sw);
	NOTICE("  Accelerated blits with Colorkey %d\n", sdl_vinfo->blit_sw_CC);
	NOTICE("  Accelerated blits with Alpha %d\n", sdl_vinfo->blit_sw_A);
	NOTICE("  Accelerated color fill %d\n", sdl_vinfo->blit_fill);
	NOTICE("  The total amount of video memory %dK\n", sdl_vinfo->video_mem);
	NOTICE("    BitsPerPixel %d\n", sdl_vinfo->vfmt->BitsPerPixel);
	NOTICE("    R %04x G %04x B %04x A %04x\n",
	       sdl_vinfo->vfmt->Rmask, sdl_vinfo->vfmt->Gmask,
	       sdl_vinfo->vfmt->Bmask, sdl_vinfo->vfmt->Amask);
	
	sdl_vflag = SDL_HWSURFACE | SDL_ANYFORMAT;
	
	sdl_display = SDL_SetVideoMode(SYS35_DEFAULT_WIDTH, SYS35_DEFAULT_HEIGHT,
				       sdl_vinfo->vfmt->BitsPerPixel, sdl_vflag);
	
	NOTICE("Dsp_depth %d flag %x\n",
	       sdl_display->format->BitsPerPixel, sdl_display->flags);
}

static void makeDIB(int width, int height, int depth) {
	
	if (sdl_display->format->BitsPerPixel == 8 && depth != 8) {
		SYSERROR("You cannot play highcolor game in 256 color mode\n");
	}
	
	if (depth == 24) {
		WARNING("SDL warning: if you are using SDL version 1.1.x (x<=3) and someting is wrong with graphics, please use version 1.1.4 or later\n");
	}
	
	if (sdl_dib) {
		SDL_FreeSurface(sdl_dib);
	}
	
	if (depth == 8) {
		if (sdl_display->format->BitsPerPixel == depth) {
			sdl_dib = SDL_AllocSurface(SDL_SWSURFACE, width, height,
						   depth, 0, 0, 0, 0);
		} else {
			sdl_dib = SDL_AllocSurface(SDL_HWSURFACE, width, height,
						   depth, 0, 0, 0, 0);
		}
	} else {
		sdl_dib = SDL_AllocSurface(SDL_HWSURFACE, width, height,
					   sdl_display->format->BitsPerPixel, 0, 0, 0, 0);
	}
	
#if 0
	if (sdl_dib->format->BitsPerPixel == 8) {
		memset(sdl_dib->format->palette->colors, 0, sizeof(SDL_Color)*256);
	}
#endif

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
		g_free(sdl_dibinfo);
	}
	
	sdl_dibinfo = g_new0(agsurface_t, 1);
	sdl_dibinfo->depth           = sdl_dib->format->BitsPerPixel;
	sdl_dibinfo->bytes_per_pixel = sdl_dib->format->BytesPerPixel;
	sdl_dibinfo->bytes_per_line  = sdl_dib->pitch;
	sdl_dibinfo->pixel  = sdl_dib->pixels;
	sdl_dibinfo->width  = width;
	sdl_dibinfo->height = height;
	sdl_dibinfo->alpha  = NULL;
	
	image_setdepth(sdl_dibinfo->depth);
}

void sdl_setFontDevice(FONT *f) {
        sdl_font = f;
}

/* offscreen の設定 */
void sdl_setWorldSize(int width, int height, int depth) {
	makeDIB(width, height, depth);
	SDL_FillRect(sdl_dib, NULL, 0);
}

/* Windowの size と depth の取得 */
void sdl_getWindowInfo(DispInfo *info) {
	info->width  = sdl_display->w;
	info->height = sdl_display->h;
	info->depth  = sdl_display->format->BitsPerPixel;
}

/*  DIBの取得 */
agsurface_t *sdl_getDIB(void) {
	return sdl_dibinfo;
}

