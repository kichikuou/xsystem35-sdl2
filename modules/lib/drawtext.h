/*
 * drawtext.h  DLL用に surface 上に文字を描く
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
/* $Id: drawtext.h,v 1.1 2003/04/22 16:34:28 chikama Exp $ */

#ifndef __DRAWTEXT_H__
#define __DRAWTEXT_H__

extern int dt_setfont(int type, int size);
extern int dt_drawtext(surface_t *sf, int x, int y, char *buf);
extern int dt_drawtext_col(surface_t *sf, int x, int y, char *buf, int r, int g, int b);

#endif /* __DRAWTEXT_H__ */
