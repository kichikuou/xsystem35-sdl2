/*
 * joystick.h  joystick interface
 *
 * Copyright (C) 1999-  Fumihiko Murata <fmurata@p1.tcnet.ne.jp>
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
/* $Id: joystick.h,v 1.7 2001/01/21 23:04:16 chikama Exp $ */

#ifndef __JOYSTICK_H__
#define __JOYSTICK_H__

#include "portab.h"

extern void joy_set_devicename(char *);
extern int  joy_open(void);
extern void joy_close(void);
extern int  joy_getinfo(void);

#endif /* !__JOYSTICK_H__ */
