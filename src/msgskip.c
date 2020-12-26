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

#include <stdint.h>
#include <string.h>
#include "portab.h"
#include "mmap.h"
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
	uint8_t bloom[];
} MsgSkipData;

static struct {
	mmap_t *map;
	MsgSkipData *data;
	boolean dirty;
} msgskip;

// Knuth's multiplicative hash.
static uint32_t hash(uint32_t x, int s) {
	return (x * 2654435761ULL) >> s;
}

void msgskip_init(const char *msgskip_file) {
	if (!msgskip_file)
		return;
	size_t length = ((BLOOM_FILTER_SIZE + 7) >> 3) + 4;
	mmap_t *m = map_file_readwrite(msgskip_file, length);
	if (!m)
		return;
	msgskip.map = m;
	msgskip.data = m->addr;
	msgskip.dirty = false;

	if (msgskip.data->size != BLOOM_FILTER_SIZE) {
		msgskip.data->size = BLOOM_FILTER_SIZE;
		memset(msgskip.data->bloom, 0, length - 4);
	}

#ifdef __EMSCRIPTEN__
	EM_ASM({ setInterval(() => _msgskip_syncFile(), 5000); });
#endif
}

void msgskip_onMessage(void) {
	if (!msgskip.data)
		return;
	uint32_t h1 = hash(sl_getPage(), 7);
	uint32_t h2 = hash(sl_getIndex(), 15);
	uint8_t unseen = 0;
	for (int i = 0; i < BLOOM_FILTER_HASHES; i++) {
		uint32_t h = (h1 + i * h2) % BLOOM_FILTER_SIZE;
		uint8_t bit = 1 << (h & 7);
		unseen |= (msgskip.data->bloom[h >> 3] & bit) ^ bit;
		msgskip.data->bloom[h >> 3] |= bit;
	}
	if (unseen) {
		msgskip.dirty = true;
		set_skipMode(FALSE);
		enable_msgSkip(FALSE);
	} else {
		enable_msgSkip(TRUE);
	}
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
