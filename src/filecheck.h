/*
 * filecheck.c  save/load file existance and kanjicode check 
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
/* $Id: filecheck.h,v 1.2 2000/11/25 13:09:04 chikama Exp $ */

#ifndef __FILECHECK_H__
#define __FILECHECK_H__

#define FILENAME_KANJI_CODE_EUC  0
#define FILENAME_KANJI_CODE_SJIS 1

extern void fc_init(char *name);
extern char *fc_search(char *req);
extern char *fc_add(char *req);
extern void fc_set_default_kanjicode(int c);

#endif /* !__FILECHECK_H__ */
