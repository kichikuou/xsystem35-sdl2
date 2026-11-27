/* Regression tests for independent MG capture and visible R line breaks.
 * Graphics and string storage are boundary stubs; message.c is linked unchanged.
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "nact.h"
#include "input.h"
#include "msgskip.h"
#include "texthook.h"
#include "variable.h"

static NACTINFO state;
NACTINFO *nact = &state;
static char strings[16][512];
static SDL_Point drawn[16];
static int draws, updates, waits, fills;

size_t svar_length(int no) { return strlen(strings[no]); }
void svar_set(int no, const char *s) { snprintf(strings[no], sizeof(strings[no]), "%s", s ? s : ""); }
void svar_append(int no, const char *s) {
	assert(strlen(strings[no]) + strlen(s) < sizeof(strings[no]));
	strcat(strings[no], s);
}
void texthook_message(const char *s) { (void)s; }
void texthook_newline(void) {}
void texthook_nextpage(void) {}
bool msgskip_isSkipping(void) { return false; }
int sys_getInputInfo(void) { return 0; }
void sys_hit_any_key(void) { waits++; }
void sys_sleep(int ms) { (void)ms; }
int ags_drawString(int x, int y, const char *s, int col, int size, SDL_Rect *r) {
	(void)s; (void)col;
	assert(draws < 16);
	drawn[draws++] = (SDL_Point){x, y};
	*r = (SDL_Rect){x, y, 24, size};
	return r->w;
}
void ags_updateArea(int x, int y, int w, int h) { (void)x; (void)y; (void)w; (void)h; updates++; }
void ags_fillRectangle(int x, int y, int w, int h, int col) { (void)x; (void)y; (void)w; (void)h; (void)col; fills++; }
void ags_drawRectangle(int x, int y, int w, int h, int col) { (void)x; (void)y; (void)w; (void)h; (void)col; }
void ags_wrapColor(int x, int y, int w, int h, int p1, int p2) { (void)x; (void)y; (void)w; (void)h; (void)p1; (void)p2; }
void ags_delRegion(void *r) { (void)r; }
void ags_putRegion(void *r, int x, int y) { (void)r; (void)x; (void)y; }
void *ags_saveRegion(int x, int y, int w, int h) { (void)x; (void)y; (void)w; (void)h; return NULL; }

static void reset(bool capture, bool display, int policy_r) {
	memset(&state, 0, sizeof(state));
	memset(strings, 0, sizeof(strings));
	draws = updates = waits = fills = 0;
	msg_init();
	state.msg.mg_getString = capture;
	state.msg.mg_dspMsg = display;
	state.msg.mg_policyR = policy_r;
	state.msg.win->x = 40;
	state.msg.win->y = 250;
	state.msg.win->height = 100;
	msg_setMessageLocation(40, 250);
}

static void visible_lines_test(bool capture, int policy_r) {
	reset(capture, true, policy_r);
	// Three text runs separated by R, as in the reported prologue dialogue.
	const char *lines[] = {"first", "second", "third"};
	for (int i = 0; i < 3; i++) {
		if (i) msg_nextLine();
		msg_putMessage(lines[i]);
		assert(drawn[i].x == 40);
		assert(drawn[i].y == 250 + i * 18);
	}
	assert(draws == 3 && updates == 3 && waits == 0);
	if (capture && policy_r == 1) {
		assert(!strcmp(strings[1], "firstsecondthird"));
		assert(state.msg.mg_curStrVarNo == 1);
	} else if (capture) {
		for (int i = 0; i < 3; i++) assert(!strcmp(strings[i + 1], lines[i]));
		assert(state.msg.mg_curStrVarNo == 3);
	} else {
		assert(strings[1][0] == '\0');
	}
}

static void hidden_capture_test(int policy_r) {
	reset(true, false, policy_r);
	state.msg.win->height = 16;
	for (int i = 0; i < 3; i++) {
		msg_putMessage("hidden");
		msg_nextLine();
	}
	SDL_Point loc;
	msg_getMessageLocation(&loc);
	assert(loc.x == 40 && loc.y == 250);
	assert(draws == 0 && updates == 0 && waits == 0 && fills == 0);
	assert(state.msg.mg_curStrVarNo == (policy_r == 1 ? 1 : 4));
	assert(!strcmp(strings[1], policy_r == 1 ? "hiddenhiddenhidden" : "hidden"));
}

static void visible_page_test(bool capture) {
	reset(capture, true, 1);
	state.msg.win->height = 16;
	msg_putMessage("before");
	msg_nextLine();
	msg_putMessage("after");
	assert(waits == 1 && fills == 1 && draws == 2);
	assert(drawn[1].x == 40 && drawn[1].y == 252);
	assert(!capture || !strcmp(strings[1], "beforeafter"));
}

int main(void) {
	visible_lines_test(false, 0);
	visible_lines_test(true, 0);
	visible_lines_test(true, 1);
	hidden_capture_test(0);
	hidden_capture_test(1);
	visible_page_test(false);
	visible_page_test(true);
	puts("7 message capture/display regression cases passed");
	return 0;
}
