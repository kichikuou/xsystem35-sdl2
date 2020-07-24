/*
 * hankaku.c  zenkaku<->hankaku conversion
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
 *
*/
#ifndef __HANKAKU_H__
#define __HANKAKU_H__

#include "portab.h"

extern BYTE *zen2han(const BYTE *src);
extern BYTE *han2zen(const BYTE *src);
extern char *format_number(int n, int width, char *buf);
extern char *format_number_zenkaku(int n, int width, char *buf);

#endif /* __HANKAKU_H__ */
