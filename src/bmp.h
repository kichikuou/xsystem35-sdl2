/*
 * bmp.h  extract 8/24 bit BMP cg
 *
 * Copyright (C) 1999 TAJIRI,Yasuhiro <tajiri@wizard.elec.waseda.ac.jp>
 * rewrited      2000 Masaki Chikam   <masaki-c@is.aist-nara.ac.jp>
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
 * @version 1.1     00/09/17 rewrite for changeing interface
 *
*/
/* $Id: bmp.h,v 1.1 2000/09/20 10:33:14 chikama Exp $ */

#ifndef __BMP__
#define __BMP__

#include "portab.h"
#include "cg.h"

typedef enum {
	BMP_OS2 = 1,
	BMP_WIN = 2
} BMP_Type;

typedef struct {
	int bmpSize;    /* header + data size?  */
	int bmpXW;      /* image width          */
	int bmpYW;      /* image height         */
	int bmpBpp;     /* image depth          */
	int bmpCp;      /* pointer to comment ? */
	int bmpImgSize; /* image size           */
	int bmpDp;      /* pointer to data      */
	int bmpPp;      /* pointer to pixel     */
	BMP_Type bmpTp; /* bmp type             */
} bmp_header;

extern boolean bmp256_checkfmt(BYTE *data);
extern cgdata *bmp256_extract(BYTE *data);
extern boolean bmp16m_checkfmt(BYTE *data);
extern cgdata *bmp16m_extract(BYTE *data);
extern cgdata *bmp_getpal(BYTE *data);

#endif /* !__BMP__ */
