/*
 * alpha_plane.c  alpha plane
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
/* $Id: alpha_plane.h,v 1.2 2001/12/07 15:03:01 chikama Exp $ */

#ifndef __ALPHA_PLANE__
#define __ALPHA_PLANE__

#include "portab.h"
#include "ags.h"

extern void alpha_set_pixels(agsurface_t *dst, int dx, int dy, int w, int h, BYTE *src, int src_pitch);
extern void alpha_get_pixel(agsurface_t *suf, int x, int y, BYTE *pic);
extern void alpha_lowercut(agsurface_t *suf, int sx, int sy, int w, int h, int s, int d);
extern void alpha_uppercut(agsurface_t *suf, int sx, int sy, int w, int h, int s, int d);
extern void alpha_set_level(agsurface_t *suf, int sx, int sy, int w, int h, int lv);
extern void alpha_copy_area(agsurface_t *suf, int sx, int sy, int w, int h, int dx, int dy);

#endif /* !__ALPHA_PLANE__ */
