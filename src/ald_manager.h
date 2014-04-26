/*
 * ald_manager.h  dri file manager
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
/* $Id: ald_manager.h,v 1.3 2003/04/22 16:34:28 chikama Exp $ */

#ifndef __ALD_MANAGER__
#define __ALD_MANAGER__

#include "portab.h"
#include "dri.h"

#define DRIFILETYPEMAX 7
typedef enum {
	DRIFILE_SCO =0,  /* scenario data */
	DRIFILE_CG  =1,  /* graphics data */
	DRIFILE_WAVE=2,  /* wave data */
	DRIFILE_MIDI=3,  /* midi data */
	DRIFILE_DATA=4,  /* misc data */
	DRIFILE_RSC =5,  /* resource data */
	DRIFILE_BGM =6   /* stream music data */
} DRIFILETYPE;

extern void     ald_init(int type, char **file, int cnt, boolean mmap);
extern dridata *ald_getdata(DRIFILETYPE type, int no);
extern void     ald_freedata(dridata *data);

#endif /* !__ALD_MANAGER__ */

