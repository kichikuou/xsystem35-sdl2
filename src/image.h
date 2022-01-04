/*
 * image.h  image操作
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
/* $Id: image.h,v 1.24 2003/01/12 10:48:50 chikama Exp $ */

#ifndef __IMAGE__
#define __IMAGE__

#include <SDL_surface.h>
#include "cg.h"
#include "ags.h"

extern void image_setdepth(int);
extern void image_copy_from_alpha(agsurface_t *dib, int sx, int sy, int w, int h, int dx, int dy, ALPHA_DIB_COPY_TYPE flag);
extern void image_copy_to_alpha(agsurface_t *dib, int sx, int sy, int w, int h, int dx, int dy, ALPHA_DIB_COPY_TYPE flag);

extern BYTE *changeImage16AlphaLevel(cgdata *cg);

#endif  /* __IMAGE__ */
