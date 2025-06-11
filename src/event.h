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

extern bool (*event_custom_handler)(const SDL_Event *);

void event_init(void);
void event_remove(void);
void event_set_joy_device_index(int index);
void event_set_mouse_location(int x, int y);
void event_set_mouse_internal_location(int x, int y);
int event_get_key(void);
int event_get_mouse(SDL_Point *p);
void event_get_wheel(int *forward, int *back);
void event_clear_wheel(void);
int event_get_joy(void);
void event_post_debugger_command(void *data);
void event_handle_event(SDL_Event *e);

#endif // __EVENT_H__
