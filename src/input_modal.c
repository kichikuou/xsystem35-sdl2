/*
 * input_modal.c  string- and number-input dialogs
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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "portab.h"
#include "nact.h"
#include "menu.h"
#include "font.h"
#include "gfx_private.h"
#include "modal.h"
#include "utfsjis.h"
#ifdef _WIN32
#include "win/dialog.h"
#endif

// The string-input dialog.
// The input field is not a microui textbox: to support IME (SDL_TEXTEDITING)
// composition we keep our own buffer (str_buf) and feed it from the handler,
// mirroring editor.c.
struct string_state {
	modal base;
	INPUTSTRING_PARAM *param;
	char composing[SDL_TEXTEDITINGEVENT_TEXT_SIZE];  // IME preedit text
	bool done;
	bool accepted;
	FontSpec font;  // inline edit (menu_inputstring2) only
};

// Heap buffer holding the string dialog's edited text. It must outlive the
// modal loop because the caller reads p->newstring after menu_inputstring()
// returns; it is freed and reallocated on the next call.
static char *str_buf;

// Append `add` to str_buf, truncating so the total stays within
// st->param->max characters (counted as UTF-8 code points).
static void str_append(struct string_state *st, const char *add) {
	int room = st->param->max;
	for (const char *p = str_buf; *p; p = advance_char(p, UTF8))
		room--;
	const char *end = add;
	while (room > 0 && *end) {
		end = advance_char(end, UTF8);
		room--;
	}
	strncat(str_buf, add, end - add);
}

// Delete the last UTF-8 character of str_buf.
static void str_backspace(void) {
	if (!*str_buf)
		return;
	char *p = str_buf + strlen(str_buf) - 1;
	while (p > str_buf && UTF8_TRAIL_BYTE(*p))
		p--;
	*p = '\0';
}

static bool menu_string_handler(const SDL_Event *e, modal *modal) {
	struct string_state *st = (struct string_state *)modal;
	switch (e->type) {
	case SDL_KEYDOWN:
		// While an IME composition is active, let it consume the key.
		if (!*st->composing) {
			switch (e->key.keysym.sym) {
			case SDLK_RETURN:
				st->accepted = true;
				st->done = true;
				break;
			case SDLK_ESCAPE:
				st->base.cancelled = true;
				break;
			case SDLK_BACKSPACE:
				str_backspace();
				break;
			}
		}
		return true;  // never forward keys to microui (there is no textbox)
	case SDL_TEXTINPUT:
		str_append(st, e->text.text);
		st->composing[0] = '\0';
		return true;
	case SDL_TEXTEDITING:
		strncpy(st->composing, e->edit.text, sizeof(st->composing) - 1);
		st->composing[sizeof(st->composing) - 1] = '\0';
		return true;
	}
	return modal_default_handler(e, modal);
}

// Tell SDL where the text input is, so the IME candidate window is positioned
// near the field. `box` is in logical (view) coordinates; convert to window
// pixels for SDL_SetTextInputRect.
static void set_text_input_rect(mu_Rect box) {
	SDL_Rect r = { box.x, box.y, box.w, box.h };
#if SDL_VERSION_ATLEAST(2, 0, 18)
	int x2, y2;
	SDL_RenderLogicalToWindow(gfx_renderer, box.x, box.y, &r.x, &r.y);
	SDL_RenderLogicalToWindow(gfx_renderer, box.x + box.w, box.y + box.h, &x2, &y2);
	r.w = x2 - r.x;
	r.h = y2 - r.y;
#endif
	SDL_SetTextInputRect(&r);
}

// Draw the IME-aware text field (committed text, preedit with underline, caret).
static void draw_string_field(mu_Context *ctx, mu_Rect box, const struct string_state *st) {
	mu_draw_rect(ctx, box, ctx->style->colors[MU_COLOR_BASE]);
	mu_draw_box(ctx, box, ctx->style->colors[MU_COLOR_BORDER]);

	mu_Font font = ctx->style->font;
	mu_Color col = ctx->style->colors[MU_COLOR_TEXT];
	int th = ctx->text_height(font);
	int x = box.x + ctx->style->padding;
	int y = box.y + (box.h - th) / 2;

	mu_push_clip_rect(ctx, box);
	if (*str_buf) {
		mu_draw_text(ctx, font, str_buf, -1, mu_vec2(x, y), col);
		x += ctx->text_width(font, str_buf, -1);
	}
	if (*st->composing) {
		mu_draw_text(ctx, font, st->composing, -1, mu_vec2(x, y), col);
		int cw = ctx->text_width(font, st->composing, -1);
		mu_draw_rect(ctx, mu_rect(x, y + th, cw, 1), col);  // preedit underline
		x += cw;
	}
	mu_draw_rect(ctx, mu_rect(x, y, 1, th), col);  // caret
	mu_pop_clip_rect(ctx);

	set_text_input_rect(box);
}

static bool inputstring_build(mu_Context *ctx, modal *modal) {
	struct string_state *st = (struct string_state *)modal;
	const char *title = (st->param->title && *st->param->title)
	                        ? st->param->title : _("Enter a string");
	int row_h = ctx->text_height(ctx->style->font) + ctx->style->padding * 2;
	int w = 320;
	int h = 3 * (row_h + ctx->style->spacing) + ctx->style->padding * 2
	        + ctx->style->title_height;
	mu_Rect r = mu_rect((view_w - w) / 2, (view_h - h) / 2, w, h);

	if (mu_begin_window_ex(ctx, title, r,
	        MU_OPT_NORESIZE | MU_OPT_NOCLOSE | MU_OPT_NOSCROLL)) {
		mu_layout_row(ctx, 1, (int[]){ -1 }, 0);

		char info[64];
		snprintf(info, sizeof(info), _("Up to %d characters"), st->param->max);
		mu_label(ctx, info);

		draw_string_field(ctx, mu_layout_next(ctx), st);

		int content = w - ctx->style->padding * 2;
		int half = (content - ctx->style->spacing) / 2;
		mu_layout_row(ctx, 2, (int[]){ half, -1 }, 0);
		if (mu_button(ctx, _("OK"))) {
			st->accepted = true;
			st->done = true;
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

bool menu_inputstring(INPUTSTRING_PARAM *p) {
#ifdef _WIN32
	return input_string(p);
#else
	struct string_state st = {
		.base = { .build = inputstring_build, .handler = menu_string_handler },
		.param = p,
	};
	free(str_buf);
	str_buf = malloc(p->max * MAX_UTF8_BYTES_PAR_CHAR + 1);
	strcpy(str_buf, p->oldstring ? p->oldstring : "");

	SDL_StartTextInput();
	modal_run(&st.base);
	SDL_StopTextInput();

	p->newstring = st.accepted ? str_buf : p->oldstring;
	return true;
#endif
}

// The inline text input (MJ command). It reuses the IME buffer handling
// and the event handler of menu_inputstring.

// Look up an AGS color index in the active palette and return it as a mu_Color.
static mu_Color palette_color(int index) {
	SDL_Color c = gfx_palette->colors[index];
	return (mu_Color){ c.r, c.g, c.b, 255 };
}

static bool inline_edit_build(mu_Context *ctx, modal *modal) {
	const int EDIT_XMARGIN = 3;
	const int EDIT_YMARGIN = 2;

	struct string_state *st = (struct string_state *)modal;
	int fh = st->param->h;
	mu_Rect rect = mu_rect(st->param->x, st->param->y,
	                       fh * st->param->max + EDIT_XMARGIN * 2, fh + EDIT_YMARGIN * 2);

	// A frameless, title-less window just to set up microui's clip/draw context;
	// all drawing below is done manually with the game's colors.
	if (mu_begin_window_ex(ctx, "editstr", rect,
	        MU_OPT_NOFRAME | MU_OPT_NOTITLE | MU_OPT_NORESIZE | MU_OPT_NOSCROLL)) {
		mu_Color bg = palette_color(nact->msg.WinBackgroundColor);
		mu_Color fg = palette_color(nact->msg.MsgFontColor);
		mu_Font font = (mu_Font)&st->font;

		mu_draw_rect(ctx, rect, bg);

		int x = rect.x + EDIT_XMARGIN;
		int y = rect.y + EDIT_YMARGIN;
		if (*str_buf) {
			mu_draw_text(ctx, font, str_buf, -1, mu_vec2(x, y), fg);
			x += ctx->text_width(font, str_buf, -1);
		}
		if (*st->composing) {
			mu_draw_text(ctx, font, st->composing, -1, mu_vec2(x, y), fg);
			int cw = ctx->text_width(font, st->composing, -1);
			mu_draw_rect(ctx, mu_rect(x, y + fh, cw, 2), fg);  // preedit underline
			x += cw;
		}
		mu_draw_rect(ctx, mu_rect(x, y, 2, fh), fg);  // caret
		set_text_input_rect(rect);
		mu_end_window(ctx);
	}

	if (st->base.cancelled)  // Esc; accepted stays false
		st->done = true;
	return !st->done;
}

bool menu_inputstring2(INPUTSTRING_PARAM *p) {
	struct string_state st = {
		.base = { .build = inline_edit_build, .handler = menu_string_handler,
		          .no_dim = true },
		.param = p,
		.font = { FONT_GOTHIC, FONT_WEIGHT_NORMAL, p->h },
	};
	free(str_buf);
	str_buf = malloc(p->max * MAX_UTF8_BYTES_PAR_CHAR + 1);
	strcpy(str_buf, p->oldstring ? p->oldstring : "");

	SDL_StartTextInput();
	modal_run(&st.base);
	SDL_StopTextInput();

	p->newstring = st.accepted ? str_buf : NULL;
	return true;
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
