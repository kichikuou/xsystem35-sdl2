/*
 * sdl_core.h  SDL acess wrapper
 *
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
/* $Id: sdl_core.h,v 1.18 2003/01/04 17:01:02 chikama Exp $ */

#ifndef __SDL_CORE__
#define __SDL_CORE__

#include "config.h"
#include <sys/types.h>
#include <SDL_events.h>
#include <SDL_surface.h>
#include "portab.h"
#include "ags.h"
#include "cursor.h"
#include "effect.h"

struct inputstring_param;

/* Effects */
struct sdl_effect;
struct sdl_effect *sdl_effect_init(SDL_Rect *rect, surface_t *old, int ox, int oy, surface_t *new, int nx, int ny, enum sdl_effect_type effect);
struct sdl_effect *sdl_sprite_effect_init(SDL_Rect *rect, int dx, int dy, int sx, int sy, int col, enum sdl_effect_type type);
struct sdl_effect *sdl_effect_magnify_init(surface_t *surface, SDL_Rect *view_rect, SDL_Rect *target_rect);
struct sdl_effect *sdl_effect_sactamask_init(SDL_Surface *mask);
void sdl_effect_step(struct sdl_effect *eff, float progress);
void sdl_effect_finish(struct sdl_effect *eff);

/* key/pointer 関係 */
extern void sdl_setJoyDeviceIndex(int index);
extern void sdl_setCursorLocation(int x, int y);
extern void sdl_setCursorInternalLocation(int x, int y);
extern void sdl_setCursorType(int type);
extern bool sdl_cursorNew(uint8_t* data, int no, CursorImage *cursorImage,  TCursorDirEntry *cursordirentry);
extern int  sdl_getKeyInfo();
extern int  sdl_getMouseInfo(SDL_Point *p);
extern void sdl_getWheelInfo(int *forward, int *back);
extern void sdl_clearWheelInfo(void);
extern int  sdl_getJoyInfo(void);
extern void sdl_setAutoRepeat(bool enable);
extern SDL_Point sdl_translateMouseCoords(int x, int y);

/* misc */
enum messagebox_type {
	MESSAGEBOX_ERROR,
	MESSAGEBOX_WARNING,
	MESSAGEBOX_INFO,
};
extern uint32_t sdl_getTicks(void);
extern void sdl_sleep(int msec);
extern void sdl_wait_vsync();
extern void sdl_showMessageBox(enum messagebox_type type, const char* title_utf8, const char* message_utf8);
extern bool sdl_inputString(struct inputstring_param *);
extern void sdl_post_debugger_command(void *data);
extern void sdl_handle_event(SDL_Event *e);

extern bool save_screenshot(const char* path);

#endif /* !__SDL_CORE__ */
