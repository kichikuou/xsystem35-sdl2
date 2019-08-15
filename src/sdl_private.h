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
#include "font.h"

struct sdl_private_data {
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	SDL_Surface     *dsp; /* toplevel surface */

	SDL_Surface     *dib; /* offscreen surface */
	
	SDL_Color       col[256]; /* color pallet */
	
	unsigned long       white; /* white pixel */
	
	agsurface_t *cimg;
	
	SDL_Rect       view;
	
	FONT *font;

	boolean dirty;

	boolean ms_active;   /* mouse is active */

	boolean fs_on;

	int      winoffset_x; /* draw offset in Window x */
	int      winoffset_y; /*                       y */

};

extern void sdl_cursor_init(void);
extern void sdl_shadow_init(void);
extern int sdl_nearest_color(int r, int g, int b);

#define SDL_AllocSurface(flags,w,h,d,r,g,b,a) SDL_CreateRGBSurface(0,w,h,d,r,g,b,a)

extern struct sdl_private_data *sdl_videodev;

#define sdl_window (sdl_videodev->window)
#define sdl_renderer (sdl_videodev->renderer)
#define sdl_texture (sdl_videodev->texture)
#define sdl_display (sdl_videodev->dsp)
#define sdl_dib (sdl_videodev->dib)
#define sdl_col (sdl_videodev->col)
#define sdl_white (sdl_videodev->white)
#define sdl_dibinfo (sdl_videodev->cimg)
#define sdl_font (sdl_videodev->font)
#define sdl_view (sdl_videodev->view)
#define view_x (sdl_videodev->view.x)
#define view_y (sdl_videodev->view.y)
#define view_w (sdl_videodev->view.w)
#define view_h (sdl_videodev->view.h)
#define sdl_dirty (sdl_videodev->dirty)
#define ms_active (sdl_videodev->ms_active)
#define sdl_fs_on (sdl_videodev->fs_on)
#define winoffset_x (sdl_videodev->winoffset_x)
#define winoffset_y (sdl_videodev->winoffset_y)

#define setRect(r,xx,yy,ww,hh) (r).x=(xx),(r).y=(yy),(r).w=(ww),(r).h=(hh)
#define setOffset(s,x,y) (s->pixels) + (x) * (s->format->BytesPerPixel) + (y) * s->pitch

#endif /* __SDL_PRIVATE_H__ */
