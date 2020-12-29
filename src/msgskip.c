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

#include "msgskip.h"
#include <stdint.h>
#include <string.h>
#include "portab.h"
#include "mmap.h"
#include "nact.h"
#include "menu.h"
#include "input.h"
#include "scenario.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <sys/mman.h>
#endif

// These parameters are chosen so that the false positive rate is less than 0.5%
// in Kichikuou Rance (122,915 distinct messages).
#define BLOOM_FILTER_SIZE	1355477
#define BLOOM_FILTER_HASHES	8

typedef struct {
	uint32_t size;
	uint8_t seen[];
} MsgSkipData;

static struct {
	mmap_t *map;
	MsgSkipData *data;
	unsigned flags;
	boolean for_ain_message;
	boolean dirty;
	boolean enabled;	// The menu item is enabled
	boolean activated;	// Turned on by the user
	boolean paused;		// Used by MsgSkip module
} msgskip = {
	.flags = MSGSKIP_STOP_ON_UNSEEN | MSGSKIP_STOP_ON_MENU | MSGSKIP_STOP_ON_CLICK,
	.enabled = TRUE,
};

// Knuth's multiplicative hash.
static uint32_t hash(uint32_t x, int s) {
	return (x * 2654435761ULL) >> s;
}

static void msgskip_action(boolean unseen) {
	if (unseen && !(msgskip.flags & MSGSKIP_SKIP_UNSEEN)) {
		if (msgskip.flags & MSGSKIP_STOP_ON_UNSEEN)
			msgskip_activate(FALSE);
		msgskip_enableMenu(FALSE);
	} else {
		msgskip_enableMenu(TRUE);
	}
}

void msgskip_init(const char *msgskip_file) {
	if (!msgskip_file)
		return;
	if (msgskip.data) {
		// Called from MsgSkip.Start but we've already initialized with the
		// MsgSkip file specified in the gameresource file. Do nothing.
		return;
	}

	size_t size;
	if (nact->ain.msg) {
		msgskip.for_ain_message = true;
		size = nact->ain.msgnum;
	} else {
		msgskip.for_ain_message = false;
		size = BLOOM_FILTER_SIZE;
	}

	size_t length = ((size + 7) >> 3) + 4;
	mmap_t *m = map_file_readwrite(msgskip_file, length);
	if (!m)
		return;

	msgskip.map = m;
	msgskip.data = m->addr;

	if (msgskip.data->size != size) {
		msgskip.data->size = size;
		memset(msgskip.data->seen, 0, length - 4);
	}

#ifdef __EMSCRIPTEN__
	EM_ASM({ setInterval(() => _msgskip_syncFile(), 5000); });
#endif
}

boolean msgskip_isSkipping(void) {
	return msgskip.enabled & msgskip.activated;
}

boolean msgskip_isActivated(void) {
	return msgskip.activated;
}

void msgskip_enableMenu(boolean enable) {
	if (enable == msgskip.enabled)
		return;
	msgskip.enabled = enable;
	menu_setSkipState(msgskip.enabled, msgskip.activated);
}

EMSCRIPTEN_KEEPALIVE
void msgskip_activate(boolean activate) {
	if (msgskip.activated == activate)
		return;
	msgskip.activated = activate;
	menu_setSkipState(msgskip.enabled, msgskip.activated);
}

void msgskip_onMessage(void) {
	if (!msgskip.data || msgskip.for_ain_message)
		return;
	uint32_t h1 = hash(sl_getPage(), 7);
	uint32_t h2 = hash(sl_getIndex(), 15);
	uint8_t unseen = 0;
	for (int i = 0; i < BLOOM_FILTER_HASHES; i++) {
		uint32_t h = (h1 + i * h2) % BLOOM_FILTER_SIZE;
		uint8_t bit = 1 << (h & 7);
		unseen |= (msgskip.data->seen[h >> 3] & bit) ^ bit;
		msgskip.data->seen[h >> 3] |= bit;
	}
	if (unseen)
		msgskip.dirty = true;
	msgskip_action(unseen);
}

void msgskip_onAinMessage(int msgid) {
	if (msgskip.paused || !msgskip.data || !msgskip.for_ain_message
		|| (unsigned)msgid >= msgskip.data->size)
		return;
	uint8_t bit = 1 << (msgid & 7);
	boolean unseen = !(msgskip.data->seen[msgid >> 3] & bit);
	msgskip.data->seen[msgid >> 3] |= bit;
	if (unseen)
		msgskip.dirty = true;
	msgskip_action(unseen);
}

void msgskip_pause(boolean pause) {
	msgskip.paused = pause;
}

unsigned msgskip_getFlags() {
	return msgskip.flags;
}

EMSCRIPTEN_KEEPALIVE
void msgskip_setFlags(unsigned flags, unsigned mask) {
	msgskip.flags &= ~mask;
	msgskip.flags |= (flags & mask);
}

#ifdef __EMSCRIPTEN__

EMSCRIPTEN_KEEPALIVE
void msgskip_syncFile() {
	if (!msgskip.dirty)
		return;
	msync(msgskip.map->addr, msgskip.map->length, MS_ASYNC);
	EM_ASM( xsystem35.shell.syncfs(); );
	msgskip.dirty = false;
}

#endif // __EMSCRIPTEN__
