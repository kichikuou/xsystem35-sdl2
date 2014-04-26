/*
 * imput.c キーボードマウス関連
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
/* $Id: imput.h,v 1.15 2001/03/30 19:16:38 chikama Exp $ */

#ifndef __IMPUT__
#define __IMPUT__

#include "portab.h"
#include "graphics.h"
#include "graphicsdevice.h"

#define SYS35KEY_NULL  0
#define SYS35KEY_UP    1
#define SYS35KEY_DOWN  2
#define SYS35KEY_LEFT  4
#define SYS35KEY_RIGHT 8
#define SYS35KEY_RET  16
#define SYS35KEY_SPC  32
#define SYS35KEY_ESC  64
#define SYS35KEY_TAB 128

extern int sys_getMouseInfo(MyPoint *p, boolean is_dibgeo);
extern int sys_getInputInfo(void);
extern int sys_getKeyInfo(void);
extern int sys_getJoyInfo(void);
extern void sys_key_releasewait(int key, boolean zi_mask_enabled);
extern void sys_hit_any_key();
extern void set_skipMode(boolean bool);
extern void set_skipMode2(boolean bool);
extern boolean get_skipMode();
extern boolean get_skipMode2();
extern void set_hak_keymode(int key, int mode);

#define sys_keywait(r,flg) Keywait(r,flg)

#endif /* __IMPUT__ */
