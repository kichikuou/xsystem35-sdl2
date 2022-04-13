/*
 * graphics.h  graphics related definition
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
/* $Id: graphics.h,v 1.24 2000/11/25 13:09:04 chikama Exp $ */

#ifndef __GRAPHICS__
#define __GRAPHICS__

#include "portab.h"
#include <SDL_rect.h>

typedef struct {
        BYTE red[256];
        BYTE green[256];
        BYTE blue[256];
} Palette256;

typedef struct {
        BYTE r,g,b;
        DWORD pixel;
} Palette;

typedef SDL_Point MyPoint;

typedef struct {
        int width;
        int height;
} MyDimension;

typedef SDL_Rect MyRectangle;

typedef struct {
        int width;
        int height;
        int depth;
} DispInfo;

#endif /* !__GRAPHICS__ */
