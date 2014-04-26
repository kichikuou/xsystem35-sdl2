/*
 * sactlog.h: バックログ
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
/* $Id: sactlog.h,v 1.1 2003/04/25 17:23:55 chikama Exp $ */

#ifndef __SACTLOG_H__
#define __SACTLOG_H__

extern int sblog_start(void);
extern int sblog_end(void);
extern int sblog_pageup(void);
extern int sblog_pagedown(void);
extern int sblog_pagepre(void);
extern int sblog_pagenext(void);

#endif
