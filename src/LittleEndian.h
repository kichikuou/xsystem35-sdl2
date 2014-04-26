/*
 * LittleEndian.h  get little endian value
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
 * @version         0.00 97/11/06 ΩÈ»«
*/
/* $Id: LittleEndian.h,v 1.6 2000/11/25 13:08:56 chikama Exp $ */

#ifndef __LITTLEENDIAN__
#define __LITTIEENDIAN__

#include "portab.h"

extern int LittleEndian_getDW(const BYTE *b,int index);
extern int LittleEndian_get3B(const BYTE *b,int index);
extern int LittleEndian_getW(const BYTE *b,int index);
extern void LittleEndian_putW(int num, BYTE *b, int index);

#endif /* !__LITTLEENDIAN__ */
