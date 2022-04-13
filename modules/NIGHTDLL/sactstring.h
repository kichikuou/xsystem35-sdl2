/*
 * sactstring.h: SACTの文字列操作関連
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
/* $Id: sactstring.h,v 1.1 2003/11/09 15:06:12 chikama Exp $ */

#ifndef __SACTSTRING_H__
#define __SACTSTRING_H__

extern int nt_sstr_init();
extern int nt_sstr_push(char *str);
extern int nt_sstr_pop(char *str, int maxlen);
extern int nt_sstr_regist_replace(char *sstr, char *dstr);
extern int nt_sstr_num2str(int strno, int fig, int nzeropad, int num);
extern char *nt_sstr_replacestr(char *msg);


#endif /* __SACTSTRING_H__ */
