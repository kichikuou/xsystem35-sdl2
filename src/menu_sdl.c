/*
 * menu_sdl.c  popup menu for SDL
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

#include <stdio.h>
#include <stdlib.h>

#include "portab.h"
#include "system.h"
#include "nact.h"
#include "menu.h"
#include "editor.h"
#include "gfx_private.h"
#include "msgskip.h"
#include "modal.h"
#ifdef _WIN32
#include "win/dialog.h"
#endif

// The middle-click popup menu.

struct popup_state {
	modal base;
	mu_Rect rect;  // popup window rect from the most recent frame
};

// Message-skip state, kept in sync via menu_setSkipState().
static bool skip_enabled = true;
static bool skip_activated;

static bool menu_popup_handler(const SDL_Event *e, modal *modal) {
	struct popup_state *st = (struct popup_state *)modal;
	if (e->type == SDL_MOUSEBUTTONUP) {
		bool point_in_menu =
			e->button.x >= st->rect.x && e->button.x < st->rect.x + st->rect.w &&
			e->button.y >= st->rect.y && e->button.y < st->rect.y + st->rect.h;
		if (e->button.button == SDL_BUTTON_RIGHT || !point_in_menu)
			st->base.cancelled = true;
	}
	return modal_default_handler(e, modal);
}

static bool menu_build(mu_Context *ctx, modal *modal) {
	struct popup_state *st = (struct popup_state *)modal;
	bool keep_open = true;

	int rows = 3;
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

		if (mu_button(ctx, _("Quit"))) {
			keep_open = false;
			menu_quitmenu_open();
		}
		mu_end_window(ctx);
	}

	if (st->base.cancelled)
		keep_open = false;

	return keep_open;
}

void menu_open(void) {
#ifdef _WIN32
	return;  // Windows uses the native menu bar
#else
	struct popup_state st = {
		.base = { .build = menu_build, .handler = menu_popup_handler },
	};
	modal_run(&st.base);
#endif
}

void menu_quitmenu_open(void) {
	const SDL_MessageBoxButtonData buttons[] = {
		{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Quit" },
		{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "Cancel" },
	};
	const SDL_MessageBoxData messagebox_data = {
		.flags = SDL_MESSAGEBOX_INFORMATION,
		.window = gfx_window,
		.title = "Confirm",
		.message = "Quit xsystem35?",
		.numbuttons = SDL_arraysize(buttons),
		.buttons = buttons,
	};
	int buttonid = 0;
	if (SDL_ShowMessageBox(&messagebox_data, &buttonid) < 0) {
		WARNING("error displaying message box");
		return;
	}
	if (buttonid == 1) {
		nact_quit(false);
	}
}

void menu_resetmenu_open(void) {
	const SDL_MessageBoxButtonData buttons[] = {
		{ SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Restart" },
		{ SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "Cancel" },
	};
	const SDL_MessageBoxData messagebox_data = {
		.flags = SDL_MESSAGEBOX_INFORMATION,
		.window = gfx_window,
		.title = "Confirm",
		.message = "Restart the game?",
		.numbuttons = SDL_arraysize(buttons),
		.buttons = buttons,
	};
	int buttonid = 0;
	if (SDL_ShowMessageBox(&messagebox_data, &buttonid) < 0) {
		WARNING("error displaying message box");
		return;
	}
	if (buttonid == 1) {
		nact_quit(true);
	}
}

bool menu_inputstring(INPUTSTRING_PARAM *p) {
#ifdef _WIN32
	return input_string(p);
#else
	p->newstring = p->oldstring;
	return true;
#endif
}

bool menu_inputstring2(INPUTSTRING_PARAM *p) {
	return edit_string(p);
}

// The number-input dialog.

struct number_state {
	modal base;
	INPUTNUM_PARAM *param;
	char buf[32];     // edited value as text
	bool done;        // set when OK/Cancel/Esc ends the dialog
	bool accepted;    // true on OK, false on Cancel
	bool focus_init;  // focus the input field on the first frame
};

// Parse st->buf; returns true (and stores the value) if it is a valid integer
// within [min, max].
static bool num_value_valid(const struct number_state *st, int *out) {
	if (!st->buf[0])
		return false;
	char *end;
	long v = strtol(st->buf, &end, 10);
	if (*end != '\0' || v < st->param->min || v > st->param->max)
		return false;
	if (out)
		*out = (int)v;
	return true;
}

// Add `delta` to the current value, clamp to [min, max], and write it back.
static void num_adjust(struct number_state *st, int delta) {
	long v = strtol(st->buf, NULL, 10) + delta;
	if (v < st->param->min)
		v = st->param->min;
	if (v > st->param->max)
		v = st->param->max;
	snprintf(st->buf, sizeof(st->buf), "%ld", v);
}

static bool inputnumber_build(mu_Context *ctx, modal *modal) {
	struct number_state *st = (struct number_state *)modal;
	const char *title = (st->param->title && *st->param->title)
	                        ? st->param->title : _("Enter a number");
	int row_h = ctx->text_height(ctx->style->font) + ctx->style->padding * 2;
	int w = 240;
	int h = 3 * (row_h + ctx->style->spacing) + ctx->style->padding * 2
	        + ctx->style->title_height;
	mu_Rect r = mu_rect((view_w - w) / 2, (view_h - h) / 2, w, h);

	if (mu_begin_window_ex(ctx, title, r,
	        MU_OPT_NORESIZE | MU_OPT_NOCLOSE | MU_OPT_NOSCROLL)) {
		mu_layout_row(ctx, 1, (int[]){ -1 }, 0);

		char info[64];
		snprintf(info, sizeof(info), "%d - %d", st->param->min, st->param->max);
		mu_label(ctx, info);

		// Input field flanked by - / + spin buttons.
		int content = w - ctx->style->padding * 2;
		int bw = row_h;  // square spin buttons
		int tbw = content - (bw + ctx->style->spacing) * 2;
		mu_layout_row(ctx, 3, (int[]){ tbw, bw, bw }, 0);

		mu_Id id = mu_get_id(ctx, "value", 5);
		mu_Rect box = mu_layout_next(ctx);
		if (st->focus_init) {
			mu_set_focus(ctx, id);
			st->focus_init = false;
		}
		int res = mu_textbox_raw(ctx, st->buf, sizeof(st->buf), id, box, 0);
		if (mu_button(ctx, "-"))
			num_adjust(st, -1);
		if (mu_button(ctx, "+"))
			num_adjust(st, 1);

		// OK is only enabled when the field holds an in-range number; Enter
		// (the textbox SUBMIT) is gated the same way.
		bool valid = num_value_valid(st, NULL);
		if ((res & MU_RES_SUBMIT) && valid) {
			st->accepted = true;
			st->done = true;
		}

		int half = (content - ctx->style->spacing) / 2;
		mu_layout_row(ctx, 2, (int[]){ half, -1 }, 0);
		if (valid) {
			if (mu_button(ctx, _("OK"))) {
				st->accepted = true;
				st->done = true;
			}
		} else {
			modal_disabled_button(ctx, _("OK"));
		}
		if (mu_button(ctx, _("Cancel"))) {
			st->accepted = false;
			st->done = true;
		}
		mu_end_window(ctx);
	}

	if (st->base.cancelled) {  // Esc
		st->accepted = false;
		st->done = true;
	}
	return !st->done;
}

bool menu_inputnumber(INPUTNUM_PARAM *p) {
#ifdef _WIN32
	return input_number(p);
#else
	struct number_state st = {
		.base = { .build = inputnumber_build, .handler = modal_default_handler },
		.param = p,
		.focus_init = true,
	};
	snprintf(st.buf, sizeof(st.buf), "%d", p->def);

	SDL_StartTextInput();
	modal_run(&st.base);
	SDL_StopTextInput();

	if (!st.accepted)
		return false;
	long v = strtol(st.buf, NULL, 10);
	if (v < p->min)
		v = p->min;
	if (v > p->max)
		v = p->max;
	p->value = (int)v;
	return true;
#endif
}

void menu_init(void) {
#ifdef _WIN32
	win_menu_init();
#endif
}

void menu_gtkmainiteration() {
	return;
}

#ifndef _WIN32
void menu_setSkipState(bool enabled, bool activated) {
	skip_enabled = enabled;
	skip_activated = activated;
}
#endif
