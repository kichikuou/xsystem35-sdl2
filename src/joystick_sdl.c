/*
 * joystick_sdl.c  joystick interface for SDL 1.1 and over
 *
 * Copyright (C) 2000- Fumihiko Murata <fmurata@p1.tcnet.ne.jp>
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
/* $Id: joystick_sdl.c,v 1.3 2000/11/25 13:09:08 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>

#include "portab.h"
#include "joystick.h"
#include "sdl_core.h"

static int device_index = -1;
static SDL_Joystick *js;

static boolean joy_open_index(int index) {
	js = SDL_JoystickOpen(index);
	if (!js)
		return FALSE;

	const char *name = SDL_JoystickName(js);
	int axes = SDL_JoystickNumAxes(js);
	int buttons = SDL_JoystickNumButtons(js);
	SDL_JoystickEventState(SDL_ENABLE);
	printf("SDL joystick '%s' %d axes %d buttons\n", name, axes, buttons);
	return TRUE;
}

void joy_set_deviceindex(int index) {
	device_index = index;
}

int joy_open(void) {
	if (device_index >= 0) {
		return joy_open_index(device_index) ? 1 : -1;
	} else {
		for (int i = 0; i < SDL_NumJoysticks(); i++) {
			if (joy_open_index(i))
				return 1;
		}
	}
	return -1;
}

void joy_close(void) {
	SDL_JoystickClose(js);
}

int joy_getinfo(void) {
	if (js) {
		return sdl_getjoyinfo();
	}
	return 0;
}
