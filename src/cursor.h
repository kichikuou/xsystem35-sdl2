/*
 * cursor.h カーソル処理(general)
 *
 * Copyright (C) 2000- TAJIRI Yasuhiro  <tajiri@venus.dti.ne.jp>
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
/* $Id: cursor.h,v 1.3 2001/03/30 19:16:38 chikama Exp $ */

#ifndef __CURSOR_H__
#define __CURSOR_H__

void cursor_init(void);
void cursor_load(int no, int linkno);
void cursor_set_type(int type);

#endif  /* __CURSOR_H__ */
