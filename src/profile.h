/*
 * profile.h  rcfile analizer
 *
 * Original GICQ Copyright 1998 Sean Gabriel <gabriel@montana.com>
 * ja extension Patch Copyright Takashi Mizuhiki <mizuhiki@cclub.cc.tut.ac.jp>
 * modified for xsystem35 Masaki Chikama (Wren) <masaki-c@is.aist-nata.ac.jp>
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
/* $Id: profile.h,v 1.5 2001/03/13 18:01:17 chikama Exp $ */

#ifndef __PROFILE_H__
#define __PROFILE_H__

#define RC_NAME ".xsys35rc"

/* 一行は 256 文字を越えない */
#define RC_LINE_CHARS_MAX 256

int  load_profile(void);
char *get_profile(const char *name);

#endif /* __PROFILE_H__ */
