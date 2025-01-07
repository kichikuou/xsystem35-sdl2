/*
 * Copyright (C) 2025 <KichikuouChrome@gmail.com>
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
#include "scheduler.h"
#include "scenario.h"
#include "sdl_core.h"
#include "nact.h"
#include "hacks.h"

bool scheduler_yield_requested;
int scheduler_cmd_count;

#define BLOOM_FILTER_HASHES 4

// Knuth's multiplicative hash.
static uint32_t hash(uint32_t x, int s) {
	return (x * 2654435761ULL) >> s;
}

/*
 * System 3.x games are basically driven by busy loops that repeatedly check
 * the state of input devices or timers until their state changes. On modern
 * computers such loops waste power, and in emscripten SDL events are not
 * queued until we call emscripten_sleep(). This function detects such busy
 * loops and schedules a yield.
*/
void scheduler_on_event(enum scheduler_event event) {
	static int prev_cmd_count = -1;
	static uint64_t bloom = 0;

	switch (event) {
	case SCHEDULER_EVENT_SLEEP:
		scheduler_yield_requested = false;
		bloom = 0;
		scheduler_cmd_count = 0;
		prev_cmd_count = -1;
		break;

	case SCHEDULER_EVENT_INPUT_CHECK_MISS:
	case SCHEDULER_EVENT_INPUT_CHECK_HIT:
	case SCHEDULER_EVENT_TIMER_CHECK:
	case SCHEDULER_EVENT_AUDIO_CHECK:
	case SCHEDULER_EVENT_VA_STATUS_CHECK:
		// A single Sys3x command may generate multiple scheduler events.
		// Deduplicate them.
		if (prev_cmd_count == scheduler_cmd_count)
			break;
		prev_cmd_count = scheduler_cmd_count;

		if (event == SCHEDULER_EVENT_INPUT_CHECK_HIT) {
			// There are new input events. Let's process them without delay.
			bloom = 0;
		} else {
			uint32_t h1 = hash(sl_getPage(), 7);
			uint32_t h2 = hash(sl_getIndex(), 15);
			uint64_t bits = 0;
			for (int i = 0; i < BLOOM_FILTER_HASHES; i++) {
				uint32_t h = (h1 + (i + 1) * h2) % 64;
				bits |= 1ULL << h;
			}
			if ((bloom & bits) == bits) {
				request_yield();
				bloom = 0;
			} else {
				bloom |= bits;
			}
		}
		break;
	}
}

void scheduler_yield(void) {
	if (game_id == GAME_AGAKE)
		sdl_sleep(6);  // The slot game runs in a 6ms-cycle loop.
	else
		sdl_wait_vsync();
}
