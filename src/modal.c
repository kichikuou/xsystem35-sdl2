/*
 * modal.c  modal popup/dialog infrastructure
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
#include "config.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "event.h"
#include "font.h"
#include "gfx.h"
#include "gfx_private.h"
#include "modal.h"
#include "nact.h"
#include "system.h"

#define MODAL_FONT_SIZE 16

static mu_Context *ctx;
static modal *current_modal;

// Touch input. A tap is translated into a synthetic mouse hover->press->release
// sequence spread across several frames, decoupled from the finger's real
// timing. This is required because microui only registers a control as hovered
// on a frame where no button is held, and the hovered window (hover_root) itself
// updates a frame late; so the press must trail the finger landing by a couple
// of frames or the tap never clicks. Holding the press also lets sliders drag.
#define TOUCH_HOVER_FRAMES 2
static enum {
	TOUCH_NONE,
	TOUCH_HOVER,   // finger down; letting microui settle hover before pressing
	TOUCH_PRESS,   // mouse button held (a drag tracks the finger in this phase)
} touch_phase;
static int touch_hover_frames;          // frames left to dwell in TOUCH_HOVER
static bool touch_release_after_press;  // finger lifted early; release once pressed
static SDL_Point touch_pos;

static const FontSpec menu_font = { FONT_GOTHIC, FONT_WEIGHT_NORMAL, MODAL_FONT_SIZE };

static int text_width_cb(mu_Font font, const char *text, int len) {
	int w;
	font_measure_text(*(const FontSpec *)font, text, len, &w, NULL);
	return w;
}

static int text_height_cb(mu_Font font) {
	int h;
	font_measure_text(*(const FontSpec *)font, "", -1, NULL, &h);
	return h;
}

// A light color scheme (light gray windows, dark text), overriding microui's
// dark default palette.
static void set_light_style(mu_Style *style) {
	static const mu_Color light[MU_COLOR_MAX] = {
		[MU_COLOR_TEXT]        = { 30,  30,  30,  255 },
		[MU_COLOR_BORDER]      = { 160, 160, 160, 255 },
		[MU_COLOR_WINDOWBG]    = { 235, 235, 235, 255 },
		[MU_COLOR_TITLEBG]     = { 210, 210, 210, 255 },
		[MU_COLOR_TITLETEXT]   = { 32,  32,  32,  255 },
		[MU_COLOR_PANELBG]     = { 0,   0,   0,   0   },
		[MU_COLOR_BUTTON]      = { 210, 210, 210, 255 },
		[MU_COLOR_BUTTONHOVER] = { 225, 225, 225, 255 },
		[MU_COLOR_BUTTONFOCUS] = { 200, 200, 210, 255 },
		[MU_COLOR_BASE]        = { 250, 250, 250, 255 },
		[MU_COLOR_BASEHOVER]   = { 240, 240, 240, 255 },
		[MU_COLOR_BASEFOCUS]   = { 235, 235, 242, 255 },
		[MU_COLOR_SCROLLBASE]  = { 200, 200, 200, 255 },
		[MU_COLOR_SCROLLTHUMB] = { 170, 170, 170, 255 },
	};
	memcpy(style->colors, light, sizeof(light));
}

static void init_context(void) {
	mu_init(ctx);
	ctx->text_width = text_width_cb;
	ctx->text_height = text_height_cb;
	ctx->style->font = (mu_Font)&menu_font;
	ctx->style->spacing = 8;
	ctx->style->thumb_size = 16;
	// microui uses style->size.y as the default height of a control row, so
	// set it to the font height to make each row tall enough for its text.
	ctx->style->size.y = text_height_cb(ctx->style->font);
	set_light_style(ctx->style);
}

bool modal_default_handler(const SDL_Event *e, modal *modal) {
	switch (e->type) {
	case SDL_QUIT:
		return false;
	case SDL_MOUSEMOTION:
		mu_input_mousemove(ctx, e->motion.x, e->motion.y);
		break;
	case SDL_MOUSEWHEEL:
		mu_input_scroll(ctx, 0, e->wheel.y * -30);
		break;
	case SDL_TEXTINPUT:
		mu_input_text(ctx, e->text.text);
		break;
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP: {
		int b = 0;
		switch (e->button.button) {
		case SDL_BUTTON_LEFT:   b = MU_MOUSE_LEFT;   break;
		case SDL_BUTTON_RIGHT:  b = MU_MOUSE_RIGHT;  break;
		case SDL_BUTTON_MIDDLE: b = MU_MOUSE_MIDDLE; break;
		}
		if (!b)
			break;
		if (e->type == SDL_MOUSEBUTTONDOWN)
			mu_input_mousedown(ctx, e->button.x, e->button.y, b);
		else
			mu_input_mouseup(ctx, e->button.x, e->button.y, b);
		break;
	}
	// Touch handling (see touch_phase above). The press/release is driven
	// from the frame loop in modal_run(), not emitted here directly.
	case SDL_FINGERDOWN:
		touch_pos = event_get_touch_position(&e->tfinger);
		mu_input_mousemove(ctx, touch_pos.x, touch_pos.y);
		touch_phase = TOUCH_HOVER;
		touch_hover_frames = TOUCH_HOVER_FRAMES;
		touch_release_after_press = false;
		break;
	case SDL_FINGERMOTION:
		touch_pos = event_get_touch_position(&e->tfinger);
		mu_input_mousemove(ctx, touch_pos.x, touch_pos.y);
		break;
	case SDL_FINGERUP:
		touch_pos = event_get_touch_position(&e->tfinger);
		if (touch_phase == TOUCH_PRESS) {
			mu_input_mouseup(ctx, touch_pos.x, touch_pos.y, MU_MOUSE_LEFT);
			touch_phase = TOUCH_NONE;
		} else if (touch_phase == TOUCH_HOVER) {
			// Tap ended before the press was emitted; release right after it.
			touch_release_after_press = true;
		}
		break;
	case SDL_KEYDOWN:
	case SDL_KEYUP: {
		if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_ESCAPE)
			modal->cancelled = true;
		int c = 0;
		switch (e->key.keysym.sym) {
		case SDLK_LSHIFT: case SDLK_RSHIFT: c = MU_KEY_SHIFT;     break;
		case SDLK_LCTRL:  case SDLK_RCTRL:  c = MU_KEY_CTRL;      break;
		case SDLK_LALT:   case SDLK_RALT:   c = MU_KEY_ALT;       break;
		case SDLK_RETURN:                   c = MU_KEY_RETURN;    break;
		case SDLK_BACKSPACE:                c = MU_KEY_BACKSPACE; break;
		}
		if (!c)
			break;
		if (e->type == SDL_KEYDOWN)
			mu_input_keydown(ctx, c);
		else
			mu_input_keyup(ctx, c);
		break;
	}
	}
	return true;
}

static void render_text(mu_Font font, const char *str, mu_Vec2 pos, mu_Color color) {
	if (!*str)
		return;
	SDL_Color col = { color.r, color.g, color.b, color.a };
	SDL_Surface *s = font_render_text(*(const FontSpec *)font, str, col);
	if (!s)
		return;
	SDL_Texture *t = SDL_CreateTextureFromSurface(gfx_renderer, s);
	if (t) {
		SDL_Rect dst = { pos.x, pos.y, s->w, s->h };
		SDL_RenderCopy(gfx_renderer, t, NULL, &dst);
		SDL_DestroyTexture(t);
	}
	SDL_FreeSurface(s);
}

// Draw a line thickened into a square brush of the given width, so it looks
// the same regardless of direction.
static void draw_thick_line(float x1, float y1, float x2, float y2, int thickness) {
	float half = thickness / 2.0f;
	for (int dx = 0; dx <= thickness; dx++) {
		for (int dy = 0; dy <= thickness; dy++) {
			float ox = dx - half, oy = dy - half;
			SDL_RenderDrawLineF(gfx_renderer, x1 + ox, y1 + oy, x2 + ox, y2 + oy);
		}
	}
}

static void render_icon(int id, mu_Rect rect, mu_Color color) {
	SDL_SetRenderDrawColor(gfx_renderer, color.r, color.g, color.b, color.a);
	switch (id) {
	case MU_ICON_CHECK: {
		// A check mark: a short stroke down to the bottom vertex, then a
		// longer stroke up to the top-right.
		float x = rect.x, y = rect.y, w = rect.w, h = rect.h;
		float ax = x + w * 0.22f, ay = y + h * 0.50f;
		float bx = x + w * 0.42f, by = y + h * 0.70f;
		float cx = x + w * 0.78f, cy = y + h * 0.28f;
		int thickness = rect.w / 8;
		if (thickness < 1)
			thickness = 1;
		draw_thick_line(ax, ay, bx, by, thickness);
		draw_thick_line(bx, by, cx, cy, thickness);
		break;
	}
	case MU_ICON_CLOSE: {
		// An X mark: two crossing diagonal strokes.
		float x = rect.x, y = rect.y, w = rect.w, h = rect.h;
		float lx = x + w * 0.32f, rx = x + w * 0.68f;
		float ty = y + h * 0.32f, by = y + h * 0.68f;
		int thickness = rect.w / 16;
		if (thickness < 1)
			thickness = 1;
		draw_thick_line(lx, ty, rx, by, thickness);
		draw_thick_line(lx, by, rx, ty, thickness);
		break;
	}
	default: {
		SDL_Rect r = { rect.x + rect.w / 2 - 2, rect.y + rect.h / 2 - 2, 4, 4 };
		SDL_RenderFillRect(gfx_renderer, &r);
		break;
	}
	}
}

static void modal_render(void) {
	SDL_SetRenderDrawBlendMode(gfx_renderer, SDL_BLENDMODE_BLEND);

	// Dim the game behind the menu.
	if (!current_modal->no_dim) {
		SDL_SetRenderDrawColor(gfx_renderer, 0, 0, 0, 110);
		SDL_RenderFillRect(gfx_renderer, NULL);
	}

	mu_Command *cmd = NULL;
	while (mu_next_command(ctx, &cmd)) {
		switch (cmd->type) {
		case MU_COMMAND_RECT: {
			mu_Color c = cmd->rect.color;
			SDL_SetRenderDrawColor(gfx_renderer, c.r, c.g, c.b, c.a);
			SDL_Rect r = { cmd->rect.rect.x, cmd->rect.rect.y,
			               cmd->rect.rect.w, cmd->rect.rect.h };
			SDL_RenderFillRect(gfx_renderer, &r);
			break;
		}
		case MU_COMMAND_TEXT:
			render_text(cmd->text.font, cmd->text.str, cmd->text.pos, cmd->text.color);
			break;
		case MU_COMMAND_ICON:
			render_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color);
			break;
		case MU_COMMAND_CLIP: {
			SDL_Rect r = { cmd->clip.rect.x, cmd->clip.rect.y,
			               cmd->clip.rect.w, cmd->clip.rect.h };
			SDL_RenderSetClipRect(gfx_renderer, &r);
			break;
		}
		}
	}
	SDL_RenderSetClipRect(gfx_renderer, NULL);
	SDL_SetRenderDrawColor(gfx_renderer, 0, 0, 0, 255);
	SDL_SetRenderDrawBlendMode(gfx_renderer, SDL_BLENDMODE_NONE);
}

bool modal_handle_event(const SDL_Event *e) {
	if (!current_modal)
		return false;
	return current_modal->handler(e, current_modal);
}

mu_Rect modal_centered_rect(int width, int height) {
	int view_w, view_h;
	gfx_getViewSize(&view_w, &view_h);
	return mu_rect((view_w - width) / 2, (view_h - height) / 2,
	               width, height);
}

EMSCRIPTEN_KEEPALIVE  // Prevent inlining, because this function is listed in ASYNCIFY_ADD
void modal_run(modal *m) {
	assert(!ctx);  // modals are not nestable
	ctx = calloc(1, sizeof(*ctx));
	init_context();

	assert(!current_modal);
	current_modal = m;

	mu_Id prev_hash = 0;

	touch_phase = TOUCH_NONE;
	touch_release_after_press = false;
	bool open = true;
	while (open && !nact->is_quit) {
		// Drive the synthetic touch sequence.
		if (touch_phase == TOUCH_HOVER) {
			if (touch_hover_frames > 0) {
				touch_hover_frames--;
			} else {
				mu_input_mousedown(ctx, touch_pos.x, touch_pos.y, MU_MOUSE_LEFT);
				touch_phase = TOUCH_PRESS;
			}
		}
		if (touch_phase == TOUCH_PRESS && touch_release_after_press) {
			mu_input_mouseup(ctx, touch_pos.x, touch_pos.y, MU_MOUSE_LEFT);
			touch_phase = TOUCH_NONE;
			touch_release_after_press = false;
		}

		event_get_key();  // pump SDL events -> trampoline -> m->handler -> microui

		mu_begin(ctx);
		open = m->build(ctx, m);
		mu_end(ctx);

		// Present only when the overlay changed.
		mu_Id hash = mu_get_id(ctx, ctx->command_list.items, ctx->command_list.idx);
		if (hash != prev_hash) {
			prev_hash = hash;
			gfx_requestRedraw();
		}
		nact->callback();
		sys_wait_vsync();  // -> gfx_updateScreen() -> modal_render_overlay()
	}

	current_modal = NULL;
	free(ctx);
	ctx = NULL;
	gfx_requestRedraw();  // repaint once more to clear the overlay

	// The gesture that opened/dismissed the dialog must not leave the engine
	// with a key or button stuck "down".
	event_reset_input_state();
}

void modal_render_overlay(void) {
	if (!ctx)
		return;
	modal_render();
}

// Mid-gray used for disabled controls; reads as "grayed out" in any scheme.
static const mu_Color DISABLED_GRAY = { 128, 128, 128, 255 };

// This mirrors mu_checkbox()'s drawing but registers no control.
void modal_disabled_checkbox(mu_Context *ctx, const char *label, int state) {
	mu_Rect r = mu_layout_next(ctx);
	mu_Rect box = mu_rect(r.x, r.y, r.h, r.h);
	ctx->draw_frame(ctx, box, MU_COLOR_BASE);
	if (state)
		mu_draw_icon(ctx, MU_ICON_CHECK, box, DISABLED_GRAY);
	r = mu_rect(r.x + box.w, r.y, r.w - box.w, r.h);
	mu_Color saved_text = ctx->style->colors[MU_COLOR_TEXT];
	ctx->style->colors[MU_COLOR_TEXT] = DISABLED_GRAY;
	mu_draw_control_text(ctx, label, r, MU_COLOR_TEXT, 0);
	ctx->style->colors[MU_COLOR_TEXT] = saved_text;
}

void modal_disabled_button(mu_Context *ctx, const char *label) {
	mu_Rect r = mu_layout_next(ctx);
	mu_draw_rect(ctx, r, ctx->style->colors[MU_COLOR_BUTTON]);
	mu_Font font = ctx->style->font;
	int tw = ctx->text_width(font, label, -1);
	int th = ctx->text_height(font);
	mu_Vec2 pos = mu_vec2(r.x + (r.w - tw) / 2, r.y + (r.h - th) / 2);
	mu_push_clip_rect(ctx, r);
	mu_draw_text(ctx, font, label, -1, pos, DISABLED_GRAY);
	mu_pop_clip_rect(ctx);
}
