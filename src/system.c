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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef ALICEHUB_IOS
#include <fcntl.h>
#include <unistd.h>
#endif
#include "system.h"
#include "sdl_compat.h"
#include "debugger.h"
#include "gfx.h"
#include "scheduler.h"

#ifdef __ANDROID__
#include <android/log.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#endif

#include "nact.h"

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

void sys_write_crash_log(const char *reason) {
#ifdef ALICEHUB_IOS
	const char *path = getenv("XSYSTEM4_CRASH_LOG_PATH");
	if (!path || !*path) return;
	int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd < 0) return;
	dprintf(fd, "=== XSystem35 Crash Report ===\nReason: %s\nPage: %d Address: 0x%x\n",
		reason ? reason : "(unknown)", nact ? nact->current_page : -1,
		nact ? nact->current_addr : 0);
	close(fd);
#elif defined(__ANDROID__)
	char buf[1024];
	int len = snprintf(buf, sizeof(buf),
		"=== XSystem35 Crash Report ===\nReason: %s\nPage: %d Address: 0x%x\n",
		reason ? reason : "(unknown)",
		nact ? nact->current_page : -1,
		nact ? nact->current_addr : 0);
	if (len <= 0) return;
	if (len >= (int)sizeof(buf)) len = (int)sizeof(buf) - 1;

	const char *candidates[3];
	int ncand = 0;
	char ext_crash[PATH_MAX];
#if XSYSTEM35_SDL_VERSION == 2
	const char *ext_dir = SDL_AndroidGetExternalStoragePath();
#else
	const char *ext_dir = SDL_GetAndroidExternalStoragePath();
#endif
	if (ext_dir && ext_dir[0]) {
		snprintf(ext_crash, sizeof(ext_crash), "%s/crash.log", ext_dir);
		candidates[ncand++] = ext_crash;
	}
	char save_crash[PATH_MAX];
	if (nact && nact->files.save_path && nact->files.save_path[0]) {
		snprintf(save_crash, sizeof(save_crash), "%s/crash.log", nact->files.save_path);
		candidates[ncand++] = save_crash;
	}
	candidates[ncand++] = "crash.log";

	for (int i = 0; i < ncand; i++) {
		int fd = open(candidates[i], O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (fd >= 0) {
			ssize_t unused_w = write(fd, buf, (size_t)len);
			(void)unused_w;
			close(fd);
		}
	}
	__android_log_write(ANDROID_LOG_FATAL, "XSystem35Crash", buf);
#else
	(void)reason;
#endif
}

void sys_error(char *format, ...) {
	char buf[512];
	va_list args;
	
	va_start(args, format);
	vsnprintf(buf, sizeof buf, format, args);
	va_end(args);
	sys_write_crash_log(buf);
	sys_show_message_box(MESSAGEBOX_ERROR, "xsystem35", buf);

	sys_exit(1);
}

void sys_exit(int code) {
#ifdef ALICEHUB_IOS
	extern void xsystem35_ios_exit(int code);
	xsystem35_ios_exit(code);
#endif
	sys35_remove();
	exit(code);
}

uint32_t sys_get_ticks(void) {
	return SDL_GetTicks();
}

void sys_sleep(int msec) {
	if (sys_is_quit_requested()) {
		nact_quit(false);
		return;
	}
	gfx_updateScreen();
	dbg_onsleep();
#ifdef __EMSCRIPTEN__
	emscripten_sleep(msec);
#elif defined(__ANDROID__)
	gfx_android_delay(msec);
#else
	while (msec > 0 && !sys_is_quit_requested()) {
		int chunk = msec > 50 ? 50 : msec;
		SDL_Delay(chunk);
		msec -= chunk;
	}
	if (sys_is_quit_requested())
		nact_quit(false);
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
	if (sys_is_quit_requested()) {
		nact_quit(false);
		return;
	}
#ifdef __ANDROID__
	gfx_android_pre_present();
	gfx_updateScreen();
	dbg_onsleep();
	gfx_android_post_present_wait();
#else
	gfx_updateScreen();
	dbg_onsleep();
#ifdef __EMSCRIPTEN__
	wait_vsync();
#else
	SDL_Delay(16);
#endif
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
	SDL_ShowSimpleMessageBox(flags, title_utf8, message_utf8, gfx_getWindow());
}
