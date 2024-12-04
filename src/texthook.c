/*
 * Copyright (C) 2019 <KichikuouChrome@gmail.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include "scenario.h"
#include "texthook.h"
#include "nact.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

void texthook_set_mode(enum texthook_mode m) {
	// Do nothing
}

void texthook_message(const char *m) {
	char *utf = toUTF8(m);
	EM_ASM_({ xsystem35.texthook.message(UTF8ToString($0), $1); },
			utf, sl_getPage());
	free(utf);
}

EM_JS(void, texthook_newline, (void), {
	xsystem35.texthook.newline();
});

EM_JS(void, texthook_nextpage, (void), {
	xsystem35.texthook.nextpage();
});

EM_JS(void, texthook_keywait, (void), {
	xsystem35.texthook.keywait();
});

#else

static enum texthook_mode mode = TEXTHOOK_NONE;

static struct {
	int newlines;
} print;

static void texthook_print_message(const char *m) {
	if (print.newlines)
		printf("%d:", sl_getPage());
	char *utf = toUTF8(m);
	printf("%s", utf);
	free(utf);
	print.newlines = 0;
}

static void texthook_print_newline(void) {
	if (print.newlines < 2) {
		putchar('\n');
		fflush(stdout);
		print.newlines++;
	}
}

static void texthook_print_nextpage(void) {
	while (print.newlines < 2) {
		putchar('\n');
		fflush(stdout);
		print.newlines++;
	}
}

static void texthook_print_keywait(void) {
	texthook_print_newline();
}

static struct {
	char *buf;
	size_t size;
	size_t pos;
} copy;

static void texthook_copy_message(const char *m) {
	size_t len = strlen(m);
	if (copy.pos + len + 1 > copy.size) {
		copy.size += min(copy.pos + len + 1, 100);
		copy.buf = realloc(copy.buf, copy.size);
	}
	strcpy(copy.buf + copy.pos, m);
	copy.pos += len;
}

static void texthook_copy_newline(void) {
	if (copy.pos == 0)
		return;
	texthook_message("\n");
}

static void texthook_copy_to_clipboard(void) {
	char *utf = toUTF8(copy.buf);
	SDL_SetClipboardText(utf);
	free(utf);
	copy.pos = 0;
}

static void texthook_copy_nextpage(void) {
	if (copy.pos > 0)
		texthook_copy_to_clipboard();
}

static void texthook_copy_keywait(void) {
	if (copy.pos > 0)
		texthook_copy_to_clipboard();
}

void texthook_set_mode(enum texthook_mode m) {
	mode = m;
}

void texthook_message(const char *m) {
	switch (mode) {
	case TEXTHOOK_NONE:
		break;
	case TEXTHOOK_PRINT:
		texthook_print_message(m);
		break;
	case TEXTHOOK_COPY:
		texthook_copy_message(m);
		break;
	}
}

void texthook_newline(void) {
	switch (mode) {
	case TEXTHOOK_NONE:
		break;
	case TEXTHOOK_PRINT:
		texthook_print_newline();
		break;
	case TEXTHOOK_COPY:
		texthook_copy_newline();
		break;
	}
}

void texthook_nextpage(void) {
	switch (mode) {
	case TEXTHOOK_NONE:
		break;
	case TEXTHOOK_PRINT:
		texthook_print_nextpage();
		break;
	case TEXTHOOK_COPY:
		texthook_copy_nextpage();
		break;
	}
}

void texthook_keywait(void) {
	switch (mode) {
	case TEXTHOOK_NONE:
		break;
	case TEXTHOOK_PRINT:
		texthook_print_keywait();
		break;
	case TEXTHOOK_COPY:
		texthook_copy_keywait();
		break;
	}
}

#endif
