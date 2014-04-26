/*
 * qnt.h  extract QNT cg
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
 * @version 1.0     01/07/30 initial version
 * @version 1.1     01/11/28 add header size information
*/
/* $Id: qnt.h,v 1.2 2001/11/29 11:21:44 chikama Exp $ */

#ifndef __QNT_H__
#define __QNT_H__

#include "portab.h"
#include "cg.h"

typedef struct {
	int hdr_size;     /* header size */
	int x0;           /* display location x */
	int y0;           /* display location y */
	int width;        /* image width        */
	int height;       /* image height       */
	int bpp;          /* image data depth   */
	int rsv;          /* reserved data      */
	int pixel_size;   /* compressed pixel size       */
	int alpha_size;   /* compressed alpha pixel size */
} qnt_header;

extern boolean qnt_checkfmt(BYTE *data);
extern cgdata *qnt_extract(BYTE *data);

#endif /* __QNT_H__ */
