/*
 * menu.c  popup menu
 *
 * Copyright (C) 2000- Masaki Chikama (Wren) <masaki-c@is.aist-nara.ac.jp>
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

#include "config.h"

#ifdef __EMSCRIPTEN__

#include <stdio.h>
#include <emscripten.h>

#include "portab.h"
#include "menu.h"

void menu_open(void) {
	return;
}

void menu_quitmenu_open(void) {
	return;
}

void menu_init(void) {
	return;
}

EM_JS(void, menu_setSkipState, (bool enabled, bool activated), {
	xsystem35.shell.setSkipButtonState(enabled, activated);
});

#else  // !__EMSCRIPTEN__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "portab.h"
#include "system.h"
#include "nact.h"
#include "menu.h"
#include "gfx_private.h"
#include "msgskip.h"
#include "modal.h"
#include "volume.h"

// The middle-click popup menu.

// What the popup menu resolved to.
enum menu_action {
	MENU_ACTION_NONE,
	MENU_ACTION_VOLUME,
	MENU_ACTION_RESET,
	MENU_ACTION_QUIT,
};

struct popup_state {
	modal base;
	mu_Rect rect;  // popup window rect from the most recent frame
	enum menu_action action;  // what the user selected
	// A three-finger tap opens the menu, but SDL_GetNumTouchFingers() reports the
	// live finger count, so it already returns 3 while the first of the three
	// near-simultaneous FINGERDOWN events is being processed - the menu opens on
	// that first event. The other two FINGERDOWN events of the same gesture are
	// still queued and, once the menu is up, would be seen as taps that dismiss
	// it. Ignore touches until every finger has lifted, then arm tap-to-dismiss.
	bool touch_armed;
};

#ifndef _WIN32  // Windows uses the native menu bar

// Message-skip state, kept in sync via menu_setSkipState().
static bool skip_enabled = true;
static bool skip_activated;

static bool point_in_rect(int x, int y, mu_Rect r) {
	return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h;
}

static bool menu_popup_handler(const SDL_Event *e, modal *modal) {
	struct popup_state *st = (struct popup_state *)modal;
	if (e->type == SDL_MOUSEBUTTONUP) {
		if (e->button.button == SDL_BUTTON_RIGHT ||
		    !point_in_rect(e->button.x, e->button.y, st->rect))
			st->base.cancelled = true;
	} else if (e->type == SDL_FINGERUP) {
		// Arm tap-to-dismiss once the opening gesture's fingers are all lifted.
		if (SDL_GetNumTouchFingers(e->tfinger.touchId) == 0)
			st->touch_armed = true;
	} else if (e->type == SDL_FINGERDOWN && st->touch_armed) {
		// A tap outside the menu dismisses it.
		int x = e->tfinger.x * view_w, y = e->tfinger.y * view_h;
		if (!point_in_rect(x, y, st->rect))
			st->base.cancelled = true;
	}
	return modal_default_handler(e, modal);
}

static bool menu_build(mu_Context *ctx, modal *modal) {
	struct popup_state *st = (struct popup_state *)modal;
	bool keep_open = true;

	int rows = 5;
	int row_h = ctx->style->size.y + ctx->style->padding * 2;
	int w = 200;
	// `rows` rows with (rows - 1) inter-row gaps, plus the window's body padding.
	int h = rows * row_h + (rows - 1) * ctx->style->spacing + ctx->style->padding * 2;
	mu_Rect r = mu_rect((view_w - w) / 2, (view_h - h) / 2, w, h);
	st->rect = r;

	if (mu_begin_window_ex(ctx, "menu", r,
	        MU_OPT_NOTITLE | MU_OPT_NORESIZE | MU_OPT_NOCLOSE | MU_OPT_NOSCROLL)) {
		mu_layout_row(ctx, 1, (int[]){ -1 }, 0);

		if (skip_enabled) {
			int sk = skip_activated;
			if (mu_checkbox(ctx, _("Message Skip"), &sk))
				msgskip_activate(sk);
		} else {
			modal_disabled_checkbox(ctx, _("Message Skip"), skip_activated);
		}

		int warp = nact->ags.mouse_warp_enabled;
		if (mu_checkbox(ctx, _("Mouse Auto Move"), &warp))
			nact->ags.mouse_warp_enabled = warp;

		if (mu_button(ctx, _("Volume"))) {
			keep_open = false;
			st->action = MENU_ACTION_VOLUME;
		}

		if (mu_button(ctx, _("Restart"))) {
			keep_open = false;
			st->action = MENU_ACTION_RESET;
		}

		if (mu_button(ctx, _("Quit"))) {
			keep_open = false;
			st->action = MENU_ACTION_QUIT;
		}
		mu_end_window(ctx);
	}

	if (st->base.cancelled)
		keep_open = false;

	return keep_open;
}

void menu_open(void) {
	struct popup_state st = {
		.base = { .build = menu_build, .handler = menu_popup_handler },
		.action = MENU_ACTION_NONE,
	};
	modal_run(&st.base);
	switch (st.action) {
	case MENU_ACTION_VOLUME:
		volume_dialog_open();
		break;
	case MENU_ACTION_RESET:
		menu_resetmenu_open();
		break;
	case MENU_ACTION_QUIT:
		menu_quitmenu_open();
		break;
	case MENU_ACTION_NONE:
		break;
	}
}

#endif  // !_WIN32

static bool confirm_lose_progress(const char *title, const char *confirm_label) {
	const SDL_MessageBoxButtonData buttons[] = {
		{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, confirm_label },
		{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, _("Cancel") },
	};
	const SDL_MessageBoxData messagebox_data = {
		.flags = SDL_MESSAGEBOX_INFORMATION,
		.window = gfx_window,
		.title = title,
		.message = _("Any unsaved progress will be lost."),
		.numbuttons = SDL_arraysize(buttons),
		.buttons = buttons,
	};
	int buttonid = 0;
	if (SDL_ShowMessageBox(&messagebox_data, &buttonid) < 0) {
		WARNING("error displaying message box");
		return false;
	}
	return buttonid == 1;
}

void menu_quitmenu_open(void) {
	if (confirm_lose_progress(_("Quit game?"), _("Quit")))
		nact_quit(false);
}

void menu_resetmenu_open(void) {
	if (confirm_lose_progress(_("Restart game?"), _("Restart")))
		nact_quit(true);
}

// On Windows, menu_open/menu_init/menu_setSkipState are provided by win/menubar.c.
#ifndef _WIN32
void menu_init(void) {
}

void menu_setSkipState(bool enabled, bool activated) {
	skip_enabled = enabled;
	skip_activated = activated;
}
#endif

#endif  // !__EMSCRIPTEN__
