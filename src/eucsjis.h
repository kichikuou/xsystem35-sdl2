/*
 * eucsjis.h -- euc/sjis related function
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
/* $Id: eucsjis.h,v 1.2 2000/09/20 10:33:16 chikama Exp $ */

#ifndef __EUCSJIS__
#define __EUCSJIS__

#include "portab.h"

/* for future */
#define sjis2lang sjis2euc
#define lang2sjis euc2sjis 

#define CHECKSJIS1BYTE(b) ( ((b) & 0xe0) == 0x80 || ((b) & 0xe0) == 0xe0 )

extern BYTE*   sjis2euc(BYTE *src);
extern BYTE*   euc2sjis(BYTE *src);
extern boolean sjis_has_hankaku(BYTE *src);
extern boolean sjis_has_zenkaku(BYTE *src);
extern int     sjis_count_char(BYTE *src);
extern void    sjis_toupper(BYTE *src);
extern BYTE*   sjis_toupper2(BYTE *src);

#endif /* __EUCSJIS__ */
