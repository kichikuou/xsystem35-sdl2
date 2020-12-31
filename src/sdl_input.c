/*
 * Copyright (C) 2020 <KichikuouChrome@gmail.com>
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

#include <string.h>
#include <SDL.h>

#include "sdl_core.h"
#include "sdl_private.h"
#include "menu.h"
#include "nact.h"

#define XMARGIN 3
#define YMARGIN 2

struct EditorState {
	INPUTSTRING_PARAM *params;
	SDL_Rect rect;
	char *text;
	char composingText[SDL_TEXTEDITINGEVENT_TEXT_SIZE];
	int cursor;
	boolean done;
};
static struct EditorState *input;

static void redraw() {
	const int bgcolor = nact->msg.WinBackgroundColor;
	const int fgcolor = nact->msg.MsgFontColor;

	ags_fillRectangle(input->rect.x, input->rect.y, input->rect.w, input->rect.h, bgcolor);

	SDL_Rect r = {
		.x = input->rect.x + XMARGIN,
		.y = input->rect.y + YMARGIN,
		.w = input->rect.w - XMARGIN * 2,
		.h = input->rect.h - YMARGIN * 2,
	};

	if (*input->text) {
		int w = sdl_drawString(r.x, r.y, input->text, fgcolor).w;
		r.x += w;
		r.w -= w;
	}

	if (*input->composingText) {
		int w = sdl_drawString(r.x, r.y, input->composingText, fgcolor).w;
		ags_fillRectangle(r.x, r.y + r.h, w, 2, fgcolor);  // underline
	}

	// FIXME: This doesn't work with propotional fonts!
	int cursor_x = r.x + r.h * input->cursor;
	ags_fillRectangle(cursor_x, r.y, 2, r.h, fgcolor);  // cursor

	// Dib geometry -> window geometry
	r.x -= nact->sys_view_area.x;
	r.y -= nact->sys_view_area.y;
	SDL_SetTextInputRect(&r);

	ags_updateArea(input->rect.x, input->rect.y, input->rect.w, input->rect.h);
}

static boolean handle_event(const SDL_Event *e) {
	switch (e->type) {
	case SDL_KEYDOWN:
		if (*input->composingText)
			break;  // Let the IME handle this event.
		switch (e->key.keysym.sym) {
		case SDLK_RETURN:
			input->params->newstring = input->text;
			input->done = TRUE;
			return TRUE;
		case SDLK_ESCAPE:
			input->params->newstring = NULL;
			input->done = TRUE;
			return TRUE;
		case SDLK_BACKSPACE:
			{
				char *p = input->text + strlen(input->text) - 1;
				while (p >= input->text && UTF8_TRAIL_BYTE(*p))
					p--;
				if (p >= input->text)
					*p = '\0';
				redraw();
			}
			return TRUE;
		}
		break;

	case SDL_TEXTINPUT:
		{
			int chars = input->params->max;
			for (const char *p = input->text; *p; p = advance_char(p, UTF8))
				chars--;
			const char *end = e->text.text;
			while (chars > 0 && *end) {
				end = advance_char(end, UTF8);
				chars--;
			}
			strncat(input->text, e->text.text, end - e->text.text);

			input->composingText[0] = '\0';
			input->cursor = 0;
			redraw();
		}
		return TRUE;

	case SDL_TEXTEDITING:
		strncpy(input->composingText, e->edit.text, SDL_TEXTEDITINGEVENT_TEXT_SIZE);
		input->cursor = e->edit.start;
		redraw();
		return TRUE;
	}
	return FALSE;
}

boolean sdl_inputString(INPUTSTRING_PARAM *p) {
	static char *buf;
	free(buf);
	buf = malloc(p->max * MAX_UTF8_BYTES_PAR_CHAR + 1);

	struct EditorState s = {
		.params = p,
		.rect = {
			.x = p->x + nact->sys_view_area.x,  // window geometry -> dib geometry
			.y = p->y + nact->sys_view_area.y,
			.w = p->h * p->max + XMARGIN * 2,
			.h = p->h + YMARGIN * 2
		},
		.text = buf,
	};
	strcpy(s.text, p->oldstring);
	input = &s;

	SDL_StartTextInput();

	void *saved_region = ags_saveRegion(s.rect.x, s.rect.y, s.rect.w, s.rect.h);
	ags_setFont(FONT_GOTHIC, p->h);
	redraw();

	sdl_custom_event_handler = handle_event;
	while (!input->done) {
		sdl_getKeyInfo();  // message pump
		nact->callback();
		sdl_wait_vsync();
	}
	SDL_StopTextInput();
	sdl_custom_event_handler = NULL;

	ags_restoreRegion(saved_region, s.rect.x, s.rect.y);
	ags_updateArea(s.rect.x, s.rect.y, s.rect.w, s.rect.h);

	input = NULL;

	return TRUE;
}
