/*
 * joystick_dmy.h  joystick interface for dummy
 *
 * Copyright (C) 1999- Fumihiko Murata <fmurata@p1.tcnet.ne.jp>
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
/* $Id: joystick_dmy.c,v 1.5 2000/11/25 13:09:07 chikama Exp $ */

#include "joystick.h"
#include "portab.h"

void joy_set_devicename(char *name) {
	return;
}

int joy_open(void) {
	return -1;
}

void joy_close(void) {
	return;
}

int joy_getinfo(void) {
	return 0;
}
