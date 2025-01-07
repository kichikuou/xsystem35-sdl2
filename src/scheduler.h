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
#ifndef __SCHEDULER__
#define __SCHEDULER__

#include <stdbool.h>

enum scheduler_event {
	SCHEDULER_EVENT_SLEEP,
	SCHEDULER_EVENT_INPUT_CHECK_MISS,
	SCHEDULER_EVENT_INPUT_CHECK_HIT,
	SCHEDULER_EVENT_TIMER_CHECK,
	SCHEDULER_EVENT_AUDIO_CHECK,
	SCHEDULER_EVENT_VA_STATUS_CHECK,
};
void scheduler_on_event(enum scheduler_event event);

extern bool scheduler_yield_requested;
static inline void request_yield(void) { scheduler_yield_requested = true; }
static inline void cancel_yield(void) { scheduler_yield_requested = false; }
static inline bool is_yield_requested(void) { return scheduler_yield_requested; }
void scheduler_yield(void);

extern int scheduler_cmd_count;
static inline void scheduler_on_command(void) {
	if (++scheduler_cmd_count >= 10000)
		request_yield();
}

#endif // __SCHEDULER__
