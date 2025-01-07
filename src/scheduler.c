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
#include "nact.h"

bool scheduler_yield_requested;
int scheduler_cmd_count;

/*
 * System 3.x games are basically driven by busy loops that repeatedly check
 * the state of input devices or timers until their state changes. On modern
 * computers such loops waste power, and in emscripten SDL events are not
 * queued until we call emscripten_sleep(). This function detects such busy
 * loops and schedules a yield.
*/
void scheduler_on_event(enum scheduler_event event) {
	static int frame_count = 0;
	static int cmd_count_of_prev_input = -1;
	static int timer_check_frame = -1;
	static int timer_check_count = 0;
	static int audio_check_frame = -1;
	static int va_status_check_count;

	switch (event) {
	case SCHEDULER_EVENT_NEW_FRAME:
		frame_count++;
		scheduler_yield_requested = false;
		break;

	case SCHEDULER_EVENT_INPUT_CHECK_MISS:
		if (scheduler_cmd_count != cmd_count_of_prev_input)
			scheduler_yield_requested = true;
		break;

	case SCHEDULER_EVENT_INPUT_CHECK_HIT:
		cmd_count_of_prev_input = scheduler_cmd_count;
		break;

	case SCHEDULER_EVENT_TIMER_CHECK:
		// Allow up to 10 timer checks in single animation frame.
		if (frame_count == timer_check_frame) {
			if (++timer_check_count >= 10)
				scheduler_yield_requested = true;
		} else {
			timer_check_frame = frame_count;
			timer_check_count = 0;
		}
		break;

	case SCHEDULER_EVENT_AUDIO_CHECK:
		if (frame_count == audio_check_frame)
			scheduler_yield_requested = true;
		audio_check_frame = frame_count;
		break;

	case SCHEDULER_EVENT_VA_STATUS_CHECK:
		if (++va_status_check_count > 1) {
			va_status_check_count = 0;
			scheduler_yield_requested = true;
		}
		break;

	case SCHEDULER_EVENT_VA_UPDATE:
		va_status_check_count = 0;
		break;
	}
}
