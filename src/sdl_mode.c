/*
 * sdl_darw.c  SDL video mode and full-screen
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
/* $Id: sdl_mode.c,v 1.6 2003/01/04 17:01:02 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <limits.h>
#include <SDL/SDL.h>

#include "portab.h"
#include "system.h"
#include "sdl_private.h"


static SDL_Rect **modes;


void sdl_vm_init(void) {
	modes = SDL_ListModes(NULL, SDL_FULLSCREEN | SDL_HWSURFACE);
	
        /* Check is there are any modes available */
	if (modes == (SDL_Rect **)0) {
		SYSERROR("No modes available!\n");
	}
	
        /* Check if or resolution is restricted */
	if (modes == (SDL_Rect **)-1){
		NOTICE("All resolutions available.\n");
	} else {
		int i;
		/* Print valid modes */
		NOTICE("Available Modes\n");
		for(i = 0; modes[i]; i++) {
			NOTICE("  %d x %d\n", modes[i]->w, modes[i]->h);
		}
	}
	
}

static int search_preferable_fullscreen_mode() {
	int i, vm = 0, delta = INT_MAX;
	
	/* すべてのmodeのなかで最も適切なモードを選択 */
	for (i = 0;  modes[i]; i++) {
		if (modes[i]->w >= view_w && 
		    modes[i]->h >= view_y) {
			int deltaw = modes[i]->w - view_w;
			int deltah = modes[i]->h - view_h;
			if (delta > (deltaw + deltah)) {
				vm = i;
				delta = deltaw + deltah;
			}
		}
	}
	return vm;
}

static void enter_fullscreen() {
	Uint32 mode = sdl_vflag;

	mode |= SDL_FULLSCREEN;
	
	sdl_display = SDL_SetVideoMode(view_w, view_h, sdl_vinfo->vfmt->BitsPerPixel, mode);
}

static void quit_fullscreen() {

}


void sdl_FullScreen(boolean on) {
	
	if (on && !sdl_fs_on) {
		sdl_fs_on = TRUE;
		enter_fullscreen();
	} else if (!on && sdl_fs_on) {
		quit_fullscreen();
		sdl_fs_on = FALSE;
	}
}


/* Windowの大きさの変更 */
void sdl_setWindowSize(int x, int y, int w, int h) {
	Uint32 mode = sdl_vflag;
	
 	view_x = x;
	view_y = y;
	
	if (w == view_w && h == view_h) return;
	
	view_w = w;
	view_h = h;
	
	if (sdl_fs_on) {
		int m = search_preferable_fullscreen_mode();
		
		if (modes[m]->w != view_w || modes[m]->h != view_h) {
			winoffset_x = (modes[m]->w - view_w) / 2;
			winoffset_y = (modes[m]->h - view_h) / 2;
			w = modes[m]->w;
			h = modes[m]->h;
		} else {
			winoffset_x = winoffset_y = 0;
		}
		
		mode |= SDL_FULLSCREEN;
	}
	
	sdl_display = SDL_SetVideoMode(w, h, sdl_vinfo->vfmt->BitsPerPixel, mode);
	ms_active = (SDL_GetAppState() & SDL_APPMOUSEFOCUS) ? TRUE : FALSE;
}
