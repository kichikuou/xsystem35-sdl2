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

#include "config.h"
#include "portab.h"

/* font の種類 */
#define FONTTYPEMAX 2

struct agsurface;

struct _FONT {
	
	boolean antialiase_on;
	
	char *name[FONTTYPEMAX];
	char face[FONTTYPEMAX];
	
	void (*sel_font)(int type, int size);
	
	struct agsurface *(*get_glyph)(const char *str_utf8);
	
	int (*draw_glyph)(int x, int y, const char *str_utf8, int col);
	
	boolean (*self_drawable)();
};

typedef struct _FONT FONT;

extern void font_init(void);

extern FONT *font_sdlttf_new();

#ifdef __EMSCRIPTEN__
int load_mincho_font(void);
#endif

#endif  /* __FONT_H__ */
