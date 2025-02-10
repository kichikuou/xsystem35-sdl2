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

extern SDL_Window *sdl_window;
extern SDL_Renderer *sdl_renderer;
extern SDL_Texture *sdl_texture;
extern SDL_Surface *sdl_dib; // offscreen surface
extern SDL_Color sdl_col[256]; // color palette
extern agsurface_t *sdl_dibinfo;
extern int view_w;
extern int view_h;
extern bool sdl_dirty;
extern bool sdl_fs_on;
extern bool (*sdl_custom_event_handler)(const SDL_Event *);

void sdl_event_init(void);
void sdl_cursor_init(void);
int sdl_nearest_color(int r, int g, int b);
bool sdl_joy_open(int index);
SDL_Surface *sdl_dib_to_surface_with_alpha(int x, int y, int w, int h);
SDL_Surface *sdl_dib_to_surface_colorkey(int x, int y, int w, int h, int col);

#endif /* __SDL_PRIVATE_H__ */
