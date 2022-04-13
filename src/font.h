/*
 * font.h  header for access to fontdevice 
 *
 * Copyright (C) 1997-1998 Masaki Chikama (Wren) <chikama@kasumi.ipl.mech.nagoya-u.ac.jp>
 *               1998-                           <masaki-c@is.aist-nara.ac.jp>
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
/* $Id: font.h,v 1.9 2002/12/21 12:28:35 chikama Exp $ */

#ifndef __FONT_H__
#define __FONT_H__

#include <SDL_surface.h>
#include "config.h"
#include "portab.h"

/* font の種類 */
#define FONTTYPEMAX 2

extern void font_init(void);
extern void font_set_name_and_index(int type, const char *name, int index);
extern void font_set_antialias(boolean enable);
extern boolean font_get_antialias(void);
extern void font_select(int type, int size);
extern struct SDL_Surface *font_get_glyph(const char *str_utf8);
extern SDL_Rect font_draw_glyph(int x, int y, const char *str_utf8, BYTE col);

#ifdef __EMSCRIPTEN__
extern int load_mincho_font(void);
#endif

#endif  /* __FONT_H__ */
