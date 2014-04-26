/*
 * image.h  image¡‡∫Ó
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

#include "cg.h"
#include "ags.h"

extern void image_setdepth(int);
extern void image_expandPixel(agsurface_t *img_src, agsurface_t *img_dst, int lv);
extern void image_fadeOut(agsurface_t *img, int lv, int col);
extern void image_fadeIn(agsurface_t *src, agsurface_t *dst, int lv);
extern void image_scaledCopyArea(agsurface_t *dib, agsurface_t *src, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, int mirror);

extern void image_drawImage8_fromData(agsurface_t *img, cgdata *cg, int x, int y, int w, int h);
extern void image_drawImage16_fromData(agsurface_t *img, cgdata *cg, int x, int y, int w, int h);
extern void image_drawLine(agsurface_t *dib, int x0, int y0, int x1, int y1, int col);
extern void image_drawRectangle(agsurface_t *dib, int x, int y, int w, int h, int col);
extern void image_fillRectangle(agsurface_t *dib, int x, int y, int w, int h, int col);
extern void image_copyArea(agsurface_t *dib, int sx, int sy, int w, int h, int dx, int dy);
extern void image_copyAreaSP(agsurface_t *dib, int sx, int sy, int w, int h, int dx, int dy, int col);
extern void image_copyAreaSP16_shadow(agsurface_t *dib, int sx, int sy, int w, int h, int dx, int dy, int lv);
extern void image_copyAreaSP16_alphaBlend(agsurface_t *dib, int sx, int sy, int w, int h, int dx, int dy, int col);
extern void image_copyAreaSP16_alphaLevel(agsurface_t *dib, int sx, int sy, int w, int h, int dx, int dy, int col);
extern void image_copyAreaSP16_whiteLevel(agsurface_t *dib, int sx, int sy, int w, int h, int dx, int dy, int col);
extern void image_copy_from_alpha(agsurface_t *dib, int sx, int sy, int w, int h, int dx, int dy, ALPHA_DIB_COPY_TYPE flag);
extern void image_copy_to_alpha(agsurface_t *dib, int sx, int sy, int w, int h, int dx, int dy, ALPHA_DIB_COPY_TYPE flag);
extern void image_wrapColor(agsurface_t *dib, int sx, int sy, int w, int h, int col, int rate);
extern void image_getPixel(agsurface_t *dib, int x, int y, Pallet *cell);

extern agsurface_t* image_saveRegion(agsurface_t *dib, int x, int y, int w, int h);
extern void image_putRegion(agsurface_t *dib, agsurface_t *dst, int x, int y);
extern void image_restoreRegion(agsurface_t *dib, agsurface_t *dst, int x, int y);
extern void image_copyRegion(agsurface_t *src, int sx, int sy, int w, int h, agsurface_t *dib, int dx, int dy);
extern void image_delRegion(agsurface_t *r);

extern void image_getGlyphImage(agsurface_t *dib, agsurface_t *glyph, int dx, int dy, int col);
extern void image_getGlyphImageNto8(agsurface_t *dib, agsurface_t *glyph, int dx, int dy, int col);
extern void image_changeColorArea8(agsurface_t *dib, int sx, int sy, int w, int h, int src, int dst, int cnt);
extern MyRectangle* image_imageFlood8(agsurface_t *dib, int x, int y, int c);
extern int  image_index2pixel(int depth, int pal);
extern void image_get_glyph(agsurface_t *gls, agsurface_t *gld);
extern void image_draw_antialiased_pattern(agsurface_t *dib, agsurface_t *pattern, int dx, int dy, int dw, int dh, int col);
extern void image_draw_pattern(agsurface_t *dib, agsurface_t *pattern, int dx, int dy, int dw, int dh, int col);
extern void image_trans_pixel_24to24p(agsurface_t *src, agsurface_t *dst);
extern void image_Mosaic(agsurface_t *dib, int sx, int sy, int w, int h, int dx, int dy, int slice);
extern void image_fillRectangleNeg(agsurface_t *dib, int x, int y, int w, int h, int col);

#endif  /* __IMAGE__ */

