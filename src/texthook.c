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
#include "scenario.h"
#include "texthook.h"
#include "nact.h"

#ifdef __EMSCRIPTEN__  // ----------------------------------------------
#include <emscripten.h>

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


#elif defined(TEXTHOOK_PRINT)  // --------------------------------------

static int newlines = 0;

void texthook_message(const char *m) {
	if (newlines)
		printf("%d:", sl_getPage());
	char *utf = toUTF8(m);
	printf("%s", utf);
	free(utf);
	newlines = 0;
}

void texthook_newline(void) {
	if (newlines < 2) {
		putchar('\n');
		fflush(stdout);
		newlines++;
	}
}

void texthook_nextpage(void) {
	while (newlines < 2) {
		putchar('\n');
		fflush(stdout);
		newlines++;
	}
}

void texthook_keywait(void) {
	texthook_newline();
}

#else  // --------------------------------------------------------------

void texthook_message(const char *m) {
}

void texthook_newline(void) {
}

void texthook_nextpage(void) {
}

void texthook_keywait(void) {
}

#endif  // -------------------------------------------------------------
