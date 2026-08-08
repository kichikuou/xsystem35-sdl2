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
#include "input_modal.h"

#if defined(__ANDROID__)

#include <SDL.h>
#include <jni.h>
#include "system.h"

#define STRING "Ljava/lang/String;"

bool input_modal_string(INPUTSTRING_PARAM *p) {
	static char buf[256];
	JNIEnv *env = SDL_AndroidGetJNIEnv();
	if ((*env)->PushLocalFrame(env, 16) < 0) {
		WARNING("Failed to allocate JVM local references");
		return false;
	}

	jstring msg = (*env)->NewStringUTF(env, p->title);
	jstring oldstring = (*env)->NewStringUTF(env, p->oldstring);
	if (!msg || !oldstring) {
		WARNING("Failed to allocate a string");
		(*env)->PopLocalFrame(env, NULL);
		return false;
	}

	jobject context = SDL_AndroidGetActivity();
	jmethodID mid = (*env)->GetMethodID(env, (*env)->GetObjectClass(env, context),
										"inputString", "(" STRING STRING "I)" STRING);
	jstring newstring = (jstring)(*env)->CallObjectMethod(env, context, mid, msg, oldstring, p->max);
	if (newstring) {
		const char *newstr_utf8 = (*env)->GetStringUTFChars(env, newstring, NULL);
		strcpy(buf, newstr_utf8);
		(*env)->ReleaseStringUTFChars(env, newstring, newstr_utf8);
		(*env)->PopLocalFrame(env, NULL);
		p->newstring = buf;
		return true;
	}

	(*env)->PopLocalFrame(env, NULL);
	p->newstring = p->oldstring;
	return false;
}

bool input_modal_string_inline(INPUTSTRING_PARAM *p) {
	p->newstring = p->oldstring;
	return true;
}

bool input_modal_number(INPUTNUM_PARAM *p) {
	JNIEnv *env = SDL_AndroidGetJNIEnv();
	if ((*env)->PushLocalFrame(env, 16) < 0) {
		WARNING("Failed to allocate JVM local references");
		return false;
	}

	jstring msg = (*env)->NewStringUTF(env, p->title);
	if (!msg) {
		WARNING("Failed to allocate a string");
		(*env)->PopLocalFrame(env, NULL);
		return false;
	}

	jobject context = SDL_AndroidGetActivity();
	jmethodID mid = (*env)->GetMethodID(env, (*env)->GetObjectClass(env, context),
										"inputNumber", "(" STRING "III)I");
	int v = (*env)->CallIntMethod(env, context, mid, msg, p->min, p->max, p->def);

	(*env)->PopLocalFrame(env, NULL);
	if (v < 0)
		return false;  // cancelled
	p->value = v;
	return true;
}

#elif defined(__EMSCRIPTEN__)

#include <emscripten.h>

bool input_modal_string(INPUTSTRING_PARAM *p) {
	static char buf[256];
	int ok = EM_ASM_({
			var r = xsystem35.shell.inputString(UTF8ToString($0), UTF8ToString($1), $2);
			if (r) {
				stringToUTF8(r, $3, $4);
				return 1;
			}
			return 0;
		}, p->title, p->oldstring, p->max, buf, sizeof buf);
	if (ok)
		p->newstring = buf;
	else
		p->newstring = p->oldstring;
	return ok;
}

bool input_modal_string_inline(INPUTSTRING_PARAM *p) {
	p->newstring = p->oldstring;
	return true;
}

bool input_modal_number(INPUTNUM_PARAM *p) {
	int v = EM_ASM_({
			return xsystem35.shell.inputNumber(UTF8ToString($0), $1, $2, $3);
		}, p->title, p->min, p->max, p->def);
	if (v < 0)
		return false;  // cancelled
	p->value = v;
	return true;
}

#else  // not Android or Emscripten

#include <SDL.h>

#include "nact.h"
#include "font.h"
#include "gfx.h"
#include "modal.h"
#include "utfsjis.h"

// The string-input dialog.
struct string_state {
	modal base;
	INPUTSTRING_PARAM *param;
	bool done;
	bool accepted;
	bool focus_init;  // focus the text field on the first frame
	FontSpec font;    // inline edit (input_modal_string_inline) only
};

// Heap buffer holding the edited text. It must outlive the modal loop because
// the caller reads p->newstring after input_modal_string() returns; it is freed
// and reallocated on the next call.
static char *str_buf;
static int str_bufsz;

static void str_buf_init(const char *initial, int max_chars) {
	free(str_buf);
	str_bufsz = max_chars * MAX_UTF8_BYTES_PAR_CHAR + 1;
	str_buf = malloc(str_bufsz);
	strncpy(str_buf, initial ? initial : "", str_bufsz - 1);
	str_buf[str_bufsz - 1] = '\0';
}

#ifndef _WIN32  // input_modal_string is provided by win/dialog.c on Windows.

static bool inputstring_build(mu_Context *ctx, modal *modal) {
	struct string_state *st = (struct string_state *)modal;
	const char *title = (st->param->title && *st->param->title)
	                        ? st->param->title : _("Enter a string");
	int row_h = ctx->text_height(ctx->style->font) + ctx->style->padding * 2;
	int w = 320;
	int h = 3 * (row_h + ctx->style->spacing) + ctx->style->padding * 2
	        + ctx->style->title_height;
	mu_Rect r = modal_centered_rect(w, h);

	if (mu_begin_window_ex(ctx, title, r,
	        MU_OPT_NORESIZE | MU_OPT_NOCLOSE | MU_OPT_NOSCROLL)) {
		mu_layout_row(ctx, 1, (int[]){ -1 }, 0);

		char info[64];
		snprintf(info, sizeof(info), _("Up to %d characters"), st->param->max);
		mu_label(ctx, info);

		mu_Id id = mu_get_id(ctx, "value", 5);
		mu_Rect box = mu_layout_next(ctx);
		if (st->focus_init) {
			mu_set_kb_focus(ctx, id);
			st->focus_init = false;
		}
		if (mu_textbox_raw(ctx, str_buf, str_bufsz, st->param->max, id, box, 0)
		    & MU_RES_SUBMIT) {
			st->accepted = true;
			st->done = true;
		}

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

bool input_modal_string(INPUTSTRING_PARAM *p) {
	struct string_state st = {
		.base = { .build = inputstring_build, .handler = modal_default_handler },
		.param = p,
		.focus_init = true,
	};
	str_buf_init(p->oldstring, p->max);
	modal_run(&st.base);

	p->newstring = st.accepted ? str_buf : p->oldstring;
	return true;
}

#endif  // !_WIN32

// The inline text input (MJ command). It shares the textbox input handling but
// draws itself with the game's font and palette.

// Look up an AGS color index in the active palette and return it as a mu_Color.
static mu_Color palette_color(int index) {
	SDL_Color c = gfx_getPaletteColor(index);
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
		mu_Id id = mu_get_id(ctx, "editstr", 7);
		if (st->focus_init) {
			mu_set_kb_focus(ctx, id);
			st->focus_init = false;
		}
		if (mu_textbox_input(ctx, str_buf, str_bufsz, st->param->max, id, rect, 0)
		    & MU_RES_SUBMIT) {
			st->accepted = true;
			st->done = true;
		}

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
		if (*ctx->preedit) {
			mu_draw_text(ctx, font, ctx->preedit, -1, mu_vec2(x, y), fg);
			int cw = ctx->text_width(font, ctx->preedit, -1);
			mu_draw_rect(ctx, mu_rect(x, y + fh, cw, 2), fg);  // preedit underline
			x += cw;
		}
		mu_draw_rect(ctx, mu_rect(x, y, 2, fh), fg);  // caret
		mu_end_window(ctx);
	}

	if (st->base.cancelled)  // Esc; accepted stays false
		st->done = true;
	return !st->done;
}

bool input_modal_string_inline(INPUTSTRING_PARAM *p) {
	struct string_state st = {
		.base = { .build = inline_edit_build, .handler = modal_default_handler,
		          .no_dim = true },
		.param = p,
		.focus_init = true,
		.font = { FONT_GOTHIC, FONT_WEIGHT_NORMAL, p->h },
	};
	str_buf_init(p->oldstring, p->max);
	modal_run(&st.base);

	p->newstring = st.accepted ? str_buf : NULL;
	return true;
}

// The number-input dialog.

#ifndef _WIN32  // input_modal_number is provided by win/dialog.c on Windows.

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
	mu_Rect r = modal_centered_rect(w, h);

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
			mu_set_kb_focus(ctx, id);
			st->focus_init = false;
		}
		int res = mu_textbox_raw(ctx, st->buf, sizeof(st->buf), 0, id, box, 0);
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

bool input_modal_number(INPUTNUM_PARAM *p) {
	struct number_state st = {
		.base = { .build = inputnumber_build, .handler = modal_default_handler },
		.param = p,
		.focus_init = true,
	};
	snprintf(st.buf, sizeof(st.buf), "%d", p->def);
	modal_run(&st.base);

	if (!st.accepted)
		return false;
	long v = strtol(st.buf, NULL, 10);
	if (v < p->min)
		v = p->min;
	if (v > p->max)
		v = p->max;
	p->value = (int)v;
	return true;
}

#endif  // !_WIN32

#endif  // platform select
