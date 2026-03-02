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
/* $Id: sactstring.h,v 1.1 2003/04/22 16:29:52 chikama Exp $ */

#ifndef __SACTSTRING_H__
#define __SACTSTRING_H__

void sstr_init(void);
void sstr_reset(void);
void sstr_push(int strno);
void sstr_pop(int strno);
void sstr_regist_replace(int sstrtno, int dstrno);
void sstr_num2str(int strno, int fig, int nzeropad, int num);


#endif /* __SACTSTRING_H__ */
