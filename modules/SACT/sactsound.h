/*
 * sactsound.h: SACTの効果音関連
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
/* $Id: sactsound.h,v 1.1 2003/04/22 16:29:52 chikama Exp $ */

#ifndef __SACTSOUND_H__
#define __SACTSOUND_H__

void ssnd_init(void);
void ssnd_play(int no);
void ssnd_stop(int no, int fadetime);
void ssnd_wait(int no);
void ssnd_waitkey(int no, int *res);
void ssnd_prepare(int no);
void ssnd_prepareLRrev(int no);
void ssnd_playLRrev(int no);
int ssnd_getlinknum(int no);
void ssnd_stopall(int time);

#endif /* __SACTSOUND_H__ */
