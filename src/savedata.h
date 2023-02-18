/*
 * savedata.h  セーブデータの管理
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
/* $Id: savedata.h,v 1.13 2001/05/08 05:36:08 chikama Exp $ */

#ifndef __SAVEDATA__
#define __SAVEDATA__

#include "portab.h"

#define SAVE_MAXNUMBER    26

#define SAVE_SAVEERR      255
#define SAVE_LOADERR      255
#define SAVE_LOADSHORTAGE 254
#define SAVE_OTHERERR     201
#define SAVE_LOADOK       0
#define SAVE_SAVEOK1      1
#define SAVE_SAVEOK0      0

struct VarRef;

int save_loadAll(int no);
int save_saveAll(int no);
int save_loadPartial(int no, struct VarRef *vref, int cnt);
int save_savePartial(int no, struct VarRef *vref, int cnt);
int save_copyAll(int dstno, int srcno);
int save_vars_to_file(char *fname_utf8, struct VarRef *src, int cnt);
int load_vars_from_file(char *fname_utf8, struct VarRef *dest, int cnt);
int save_strs_to_file(char *fname_utf8, int start, int cnt);
int load_strs_from_file(char *fname_utf8, int start, int cnt);
const char *save_get_file(int index);
int save_delete_file(int index);

#endif /* __SAVEDATA__ */
