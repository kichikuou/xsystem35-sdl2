/*
 * vsp.h  extract VSP cg
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
/* $Id: vsp.h,v 1.1 2000/09/20 10:33:17 chikama Exp $ */

#ifndef __VSP__
#define __VSP__

#include "portab.h"
#include "cg.h"

typedef struct {
	int vspX0;  /* display location x    */
	int vspY0;  /* display location y    */
	int vspXW;  /* image width           */
	int vspYW;  /* image height          */
	int vspPb;  /* default pallet bank   */
	int vspPp;  /* pointer to pallet     */
	int vspDp;  /* pointer to pixel data */
} vsp_header; 
/*
 * vspPb:
 *   VSP has only 16 pallets. When 256 pallets mode, this parameter determine
 *  where to copy these 16 pallets in 256 pallets.
 *  The parameter is from 0 to 15, and for example, if it is 1 then copy 
 *  16 palltes into pal[16] ~ pal[31].
*/
	
extern boolean vsp_checkfmt(BYTE *data);
extern cgdata *vsp_extract(BYTE *data);
extern cgdata *vsp_getpal(BYTE *data);

#endif /* !__VSP__ */
