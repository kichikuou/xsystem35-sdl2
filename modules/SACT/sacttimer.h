/*
 * sacttimer.h: SACTのタイマ関連
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
/* $Id: sacttimer.h,v 1.1 2003/04/22 16:29:52 chikama Exp $ */

#ifndef __SACTTIMER_H__
#define __SACTTIMER_H__

#include <sys/time.h>
#include <unistd.h>

struct _stimer {
	int val;
	struct timeval tv_base;
};
typedef struct _stimer stimer_t;


void stimer_init(void);
void stimer_reset(int id, int val);
int stimer_get(int id);

#endif
