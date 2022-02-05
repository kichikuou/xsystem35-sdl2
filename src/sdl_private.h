/*
 * sdl_private.h  SDL only private data
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
/* $Id: sdl_private.h,v 1.4 2002/04/29 05:48:21 chikama Exp $ */

#ifndef __SDL_PRIVATE_H__
#define __SDL_PRIVATE_H__

#include "config.h"

#include <SDL.h>

#include "portab.h"
#include "ags.h"

struct sdl_private_data {
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	SDL_Surface     *dsp; /* toplevel surface */

	SDL_Surface     *dib; /* offscreen surface */
	
	SDL_Color       col[256]; /* color palette */
	
	agsurface_t *cimg;
	
	int     view_w;
	int     view_h;
	
	boolean dirty;

	boolean ms_active;   /* mouse is active */

	boolean fs_on;

	boolean (*custom_event_handler)(const SDL_Event *);
};

void sdl_cursor_init(void);
void sdl_shadow_init(void);
int sdl_nearest_color(int r, int g, int b);
boolean sdl_joy_open(int index);

extern struct sdl_private_data *sdl_videodev;

#define sdl_window (sdl_videodev->window)
#define sdl_renderer (sdl_videodev->renderer)
#define sdl_texture (sdl_videodev->texture)
#define sdl_display (sdl_videodev->dsp)
#define sdl_dib (sdl_videodev->dib)
#define sdl_col (sdl_videodev->col)
#define sdl_dibinfo (sdl_videodev->cimg)
#define view_w (sdl_videodev->view_w)
#define view_h (sdl_videodev->view_h)
#define sdl_dirty (sdl_videodev->dirty)
#define ms_active (sdl_videodev->ms_active)
#define sdl_fs_on (sdl_videodev->fs_on)
#define sdl_custom_event_handler (sdl_videodev->custom_event_handler)

#endif /* __SDL_PRIVATE_H__ */
