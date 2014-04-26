/*
 * pms.h  extract 8/16 bit PMS cg
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
 * @version 1.1     00/09/17 rewrite for changeing interface
*/
/* $Id: pms.h,v 1.1 2000/09/20 10:33:17 chikama Exp $ */

#ifndef __PMS__
#define __PMS__

#include "portab.h"
#include "cg.h"

typedef struct {
	int pmsVer;     /* pms data version             */
	int pmsHdrSize; /* size of header               */
	int pmsBpp;     /* pms data depth, 8 or 16      */
	int pmsBppS;    /* shadow data depth, if exists */
	int pmsSf;      /* sprite flag (not used?)      */
	int pmsBf;      /* pallet bank                  */
	int pmsX0;      /* display location x           */
	int pmsY0;      /* display location y           */
	int pmsXW;      /* image width                  */
	int pmsYW;      /* image height                 */
	int pmsDp;      /* pointer to data              */
	int pmsPp;      /* pointer to pallet or shadow  */
	int pmsCp;      /* pointer to comment           */
} pms_header;
/*
 * pmsBppS
 *  If pmsBpp is 16 and pmsPp is not 0, shadow (alpha data) is exists and
 *  pmsBppS shows shadow's depth (normally 8).
 *
 * pmsBf
 *  If pmsBpp is 8, pmsBf determine which pallets is used in this data and
 *  needed to copy. The pmsBf is mask and if bit is on then copy.
 *  For example, if pmsBf=0xff00, then copy pal[128] ~ pal[255].
 *
 * pmsPp
 *  If pmsBpp is 8 then this is pointer to pallet. If pmsBpp is 16 and
 *  pmsPp is not 0, then shadow is exists and this is pointer to it's data.
 *
*/ 

extern boolean  pms256_checkfmt(BYTE *data);
extern cgdata  *pms256_extract(BYTE *data);
extern boolean  pms64k_checkfmt(BYTE *data);
extern cgdata  *pms64k_extract(BYTE *data);
extern cgdata  *pms_getpal(BYTE *data);

#endif /* !__PMS__ */

