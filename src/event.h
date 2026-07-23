/*
 * Copyright (C) 2000-     Fumihiko Murata       <fmurata@p1.tcnet.ne.jp>
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

#ifndef __EVENT_H__
#define __EVENT_H__

#include "config.h"
#include <SDL_events.h>
#include "portab.h"

void event_init(void);
void event_remove(void);
void event_set_joy_device_index(int index);
void event_set_mouse_internal_location(int x, int y);
SDL_Point event_get_touch_position(const SDL_TouchFingerEvent *e);
int event_get_key(void);
int event_get_mouse(SDL_Point *p);
void event_get_wheel(vmvar_t *forward, vmvar_t *back);
void event_clear_wheel(void);
int event_get_joy(void);
void event_reset_input_state(void);
void event_post_debugger_command(void *data);
void event_handle_event(SDL_Event *e);

#endif // __EVENT_H__
