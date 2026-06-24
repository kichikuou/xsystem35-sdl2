/*
 * virtual_pointer.h  Trackpad-style virtual mouse pointer for touch devices
 *
 * Copyright (C) 2026 <KichikuouChrome@gmail.com>
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
#ifndef __VIRTUAL_POINTER_H__
#define __VIRTUAL_POINTER_H__

#include <stdbool.h>
#include <SDL.h>

/* Returns true if the virtual mouse pointer feature is enabled. */
bool vp_is_enabled(void);

/*
 * Draws the virtual cursor on top of the presented frame. No-op when the
 * feature is disabled. Called from gfx_updateScreen() between the game image
 * RenderCopy and RenderPresent.
 */
void vp_draw(SDL_Renderer *renderer);

#endif /* __VIRTUAL_POINTER_H__ */
