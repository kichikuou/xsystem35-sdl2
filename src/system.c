/*
 * Copyright (C) 2025 kichikuou <KichikuouChrome@gmail.com>
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

#include "system.h"
#include <SDL.h>
#include "debugger.h"
#include "gfx.h"
#include "gfx_private.h"
#include "scheduler.h"
#include "sdl_core.h"

#ifdef __ANDROID__
#include <android/log.h>
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#ifdef DEBUG
#define DEBUGLEVEL 2
#else
#define DEBUGLEVEL 0
#endif /* DEBUG */

static int debuglv = DEBUGLEVEL;

void sys_set_debug_level(int level) {
	debuglv = level;
}

void sys_message(int lv, char *format, ...) {
	if (debuglv < lv)
		return;

	va_list args;
	va_start(args, format);

#ifdef __ANDROID__
	const int prio_table[] = {
		ANDROID_LOG_FATAL,
		ANDROID_LOG_ERROR,
		ANDROID_LOG_WARN,
		ANDROID_LOG_INFO,
		ANDROID_LOG_INFO,
		ANDROID_LOG_VERBOSE,
	};
	int prio = prio_table[min(lv, 5)];
	__android_log_vprint(prio, "xsystem35", format, args);
#else
	if (!dbg_console_vprintf(lv, format, args))
		vfprintf(stderr, format, args);
#endif
	va_end(args);
}

void sys_error(char *format, ...) {
	char buf[512];
	va_list args;
	
	va_start(args, format);
	vsnprintf(buf, sizeof buf, format, args);
	va_end(args);
	sys_show_message_box(MESSAGEBOX_ERROR, "xsystem35", buf);

	sys35_remove();
	exit(1);
}

void sys_exit(int code) {
	sys35_remove();
#ifdef __EMSCRIPTEN__
	EM_ASM( xsystem35.shell.quit(); );
	sys_sleep(1000000000);
#else
	exit(code);
#endif
}

uint32_t sys_get_ticks(void) {
	return SDL_GetTicks();
}

void sys_sleep(int msec) {
	gfx_updateScreen();
	dbg_onsleep();
#ifdef __EMSCRIPTEN__
	emscripten_sleep(msec);
#else
	SDL_Delay(msec);
#endif
	scheduler_on_event(SCHEDULER_EVENT_SLEEP);
}

#ifdef __EMSCRIPTEN__
EM_JS(void, wait_vsync, (void), {
	// We need a `return` here for JSPI support (ASYNCIFY=2).
	return Asyncify.handleSleep(function(wakeUp) {
		window.requestAnimationFrame(function() {
			wakeUp();
		});
	});
});
#endif

void sys_wait_vsync(void) {
	gfx_updateScreen();
	dbg_onsleep();
#ifdef __EMSCRIPTEN__
	wait_vsync();
#else
	SDL_Delay(16);
#endif
	scheduler_on_event(SCHEDULER_EVENT_SLEEP);
}

void sys_show_message_box(enum messagebox_type type, const char* title_utf8, const char* message_utf8) {
	uint32_t flags = 0;
	switch (type) {
	case MESSAGEBOX_ERROR: flags = SDL_MESSAGEBOX_ERROR; break;
	case MESSAGEBOX_WARNING: flags = SDL_MESSAGEBOX_WARNING; break;
	case MESSAGEBOX_INFO: flags = SDL_MESSAGEBOX_INFORMATION; break;
	}
	SDL_ShowSimpleMessageBox(flags, title_utf8, message_utf8, gfx_window);
}
