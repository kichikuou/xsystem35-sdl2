/*
 * modal.h  modal popup/dialog infrastructure
 *
 * Copyright (C) 2026 kichikuou <KichikuouChrome@gmail.com>
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
#ifndef __MODAL_H__
#define __MODAL_H__

#include <stdbool.h>

#include "microui/microui.h"

union SDL_Event;

typedef struct modal {
	// Build one frame of the modal's UI. Returns false to close the modal.
	bool (*build)(mu_Context *ctx, struct modal *modal);
	// SDL event handler. Returns false to fall through to the default event handling.
	bool (*handler)(const union SDL_Event *e, struct modal *modal);

	bool cancelled;          // set by the handler on Esc; build returns false
} modal;

// Default modal event handler. Use it as a modal's `handler` for simple dialogs, or
// call it from a custom handler to feed microui and get the shared behavior.
bool modal_default_handler(const union SDL_Event *e, modal *modal);

// Run a modal loop that drives `modal` until `build` returns false.
void modal_run(modal *modal);

// Draw a grayed-out, non-interactive checkbox.
void modal_disabled_checkbox(mu_Context *ctx, const char *label, int state);

#endif /* __MODAL_H__ */
