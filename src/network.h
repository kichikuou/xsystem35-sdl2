/*
 * network.h  network main
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
/* $Id: network.h,v 1.3 2000/11/25 13:09:12 chikama Exp $ */
#ifndef __NETWORK_H__
#define __NETWORK_H__

int  network_create_channel(int portnum, int use_max);
void network_close(int user_num);
int  network_get_channel();
int  network_get_user_state(int num);
int  network_check_buffer();
void network_read_buffer(int *var);
void network_write_buffer(int *var, int cnt);

#endif  /* __NETWORK_H__ */
