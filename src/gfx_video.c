/*
 * gfx_video.c  SDL video init
 *
 * Copyright (C) 2000-     Fumihiko Murata       <fmurata@p1.tcnet.ne.jp>
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
/* $Id: gfx_video.c,v 1.11 2003/01/04 17:01:02 chikama Exp $ */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include "sdl_compat.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif
#ifdef __ANDROID__
#include <dlfcn.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#endif

#include "portab.h"
#include "system.h"
#include "gfx.h"
#include "gfx_private.h"
#include "xsystem35.h"
#include "image.h"
#ifdef ALICEHUB_IOS
#include "EnginePlatform.h"
#include "mixer.h"
#include "msgskip.h"
#endif

static SDL_Window *gfx_window;
SDL_Renderer *gfx_renderer;
SDL_Texture *gfx_texture;
SDL_Surface *main_surface; // offscreen surface
SDL_Palette *gfx_palette;
surface_t *gfx_dibinfo;
int view_w;
int view_h;
static bool gfx_fullscreen;
static bool gfx_integer_scaling;

static void window_init(const char *render_driver);
static void makeDIB(int width, int height, int depth);

#ifdef __ANDROID__
#define EGL_DRAW_SURFACE_ENUM 0x3059
#define ANDROID_ADPF_TARGET_NS 12500000LL
#define ANDROID_FRAME_PERIOD_NS 16666667LL

typedef void *(*pfn_eglGetCurrentDisplay_t)(void);
typedef void *(*pfn_eglGetCurrentSurface_t)(int readdraw);
typedef void *(*pfn_eglGetProcAddress_t)(const char *procname);
typedef unsigned int (*pfn_eglPresentationTimeANDROID_t)(void *dpy, void *surf, int64_t time_ns);

typedef struct APerformanceHintManager APerformanceHintManager;
typedef struct APerformanceHintSession APerformanceHintSession;
typedef APerformanceHintManager *(*pfn_adpf_get_manager_t)(void);
typedef APerformanceHintSession *(*pfn_adpf_create_session_t)(
	APerformanceHintManager *mgr, const int32_t *tids, size_t size, int64_t target_ns);
typedef int64_t (*pfn_adpf_get_pref_rate_t)(APerformanceHintManager *mgr);
typedef int (*pfn_adpf_report_work_t)(APerformanceHintSession *sess, int64_t actual_ns);
typedef void (*pfn_adpf_close_session_t)(APerformanceHintSession *sess);

static void *s_lib_egl = NULL;
static void *s_lib_android = NULL;
static pfn_eglGetCurrentDisplay_t s_pfn_egl_cur_dpy = NULL;
static pfn_eglGetCurrentSurface_t s_pfn_egl_cur_surf = NULL;
static pfn_eglPresentationTimeANDROID_t s_pfn_egl_pres_time = NULL;

static APerformanceHintSession *s_adpf_session = NULL;
static pfn_adpf_get_manager_t s_pfn_adpf_get_mgr = NULL;
static pfn_adpf_create_session_t s_pfn_adpf_create = NULL;
static pfn_adpf_get_pref_rate_t s_pfn_adpf_get_rate = NULL;
static pfn_adpf_report_work_t s_pfn_adpf_report = NULL;
static pfn_adpf_close_session_t s_pfn_adpf_close = NULL;
static int64_t s_adpf_min_interval_ns = 16000000LL;
static int64_t s_last_adpf_report_ns = 0;

static int64_t s_last_frame_end_ns = 0;
static int64_t s_work_start_ns = 0;
static int64_t s_frame_sleep_ns = 0;
static int64_t s_work_cputime_start_ns = 0;
static bool s_android_in_background = false;
static bool s_adpf_needs_focus_refresh = false;

static inline int64_t android_monotonic_ns(void) {
	struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
		return 0;
	return (int64_t)ts.tv_sec * 1000000000LL + (int64_t)ts.tv_nsec;
}

static inline int64_t android_thread_cputime_ns(void) {
	struct timespec ts;
	if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts) != 0)
		return 0;
	return (int64_t)ts.tv_sec * 1000000000LL + (int64_t)ts.tv_nsec;
}

static void android_apply_render_thread_sched(void) {
	/* THREAD_PRIORITY_URGENT_DISPLAY (-8) + 50us timer slack for accurate pacing */
	setpriority(PRIO_PROCESS, 0, -8);
	prctl(PR_SET_TIMERSLACK, 50000UL, 0, 0, 0);
}

static void android_apply_vsync(void) {
#if XSYSTEM35_SDL_VERSION == 2 && SDL_VERSION_ATLEAST(2, 0, 18)
	if (gfx_renderer)
		SDL_RenderSetVSync(gfx_renderer, 1);
#endif
}

static void android_reset_pacer_timestamps(void) {
	s_last_frame_end_ns = 0;
	s_work_start_ns = 0;
	s_frame_sleep_ns = 0;
	s_work_cputime_start_ns = 0;
}

static void android_adpf_close_session(void) {
	if (s_adpf_session && s_pfn_adpf_close) {
		s_pfn_adpf_close(s_adpf_session);
		s_adpf_session = NULL;
	}
	s_last_adpf_report_ns = 0;
}

static void android_adpf_open_session(void) {
	if (!s_pfn_adpf_get_mgr || !s_pfn_adpf_create || !s_pfn_adpf_report)
		return;
	android_adpf_close_session();
	APerformanceHintManager *mgr = s_pfn_adpf_get_mgr();
	if (!mgr)
		return;
	if (s_pfn_adpf_get_rate) {
		int64_t rate = s_pfn_adpf_get_rate(mgr);
		if (rate > s_adpf_min_interval_ns && rate <= 250000000LL)
			s_adpf_min_interval_ns = rate;
	}
	int32_t tid = (int32_t)gettid();
	s_adpf_session = s_pfn_adpf_create(mgr, &tid, 1, ANDROID_ADPF_TARGET_NS);
}

static void android_perf_pacing_init(void) {
	SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
	SDL_SetHint(SDL_HINT_ANDROID_BLOCK_ON_PAUSE_PAUSEAUDIO, "1");
	android_apply_render_thread_sched();

	if (!s_lib_egl)
		s_lib_egl = dlopen("libEGL.so", RTLD_NOW | RTLD_LOCAL);
	if (s_lib_egl) {
		s_pfn_egl_cur_dpy = (pfn_eglGetCurrentDisplay_t)dlsym(s_lib_egl, "eglGetCurrentDisplay");
		s_pfn_egl_cur_surf = (pfn_eglGetCurrentSurface_t)dlsym(s_lib_egl, "eglGetCurrentSurface");
		pfn_eglGetProcAddress_t get_proc =
			(pfn_eglGetProcAddress_t)dlsym(s_lib_egl, "eglGetProcAddress");
		if (get_proc)
			s_pfn_egl_pres_time = (pfn_eglPresentationTimeANDROID_t)get_proc("eglPresentationTimeANDROID");
		if (!s_pfn_egl_pres_time)
			s_pfn_egl_pres_time = (pfn_eglPresentationTimeANDROID_t)dlsym(s_lib_egl, "eglPresentationTimeANDROID");
	}

	if (!s_lib_android)
		s_lib_android = dlopen("libandroid.so", RTLD_NOW | RTLD_LOCAL);
	if (s_lib_android) {
		s_pfn_adpf_get_mgr = (pfn_adpf_get_manager_t)dlsym(s_lib_android, "APerformanceHint_getManager");
		s_pfn_adpf_create = (pfn_adpf_create_session_t)dlsym(s_lib_android, "APerformanceHint_createSession");
		s_pfn_adpf_get_rate = (pfn_adpf_get_pref_rate_t)dlsym(s_lib_android, "APerformanceHint_getPreferredUpdateRateNanos");
		s_pfn_adpf_report = (pfn_adpf_report_work_t)dlsym(s_lib_android, "APerformanceHint_reportActualWorkDuration");
		s_pfn_adpf_close = (pfn_adpf_close_session_t)dlsym(s_lib_android, "APerformanceHint_closeSession");
		android_adpf_open_session();
	}
}

static void android_perf_pacing_fini(void) {
	android_adpf_close_session();
	android_reset_pacer_timestamps();
}
#endif

void gfx_reset_frame_pacer(void) {
#ifdef __ANDROID__
	android_reset_pacer_timestamps();
	android_apply_vsync();
	android_apply_render_thread_sched();
	if (!s_android_in_background && !s_adpf_session)
		android_adpf_open_session();
#endif
}

void gfx_android_on_background(void) {
#ifdef __ANDROID__
	s_android_in_background = true;
	s_adpf_needs_focus_refresh = true;
	android_adpf_close_session();
	android_reset_pacer_timestamps();
#endif
}

void gfx_android_on_foreground(void) {
#ifdef __ANDROID__
	bool should_reopen = s_android_in_background || !s_adpf_session || s_adpf_needs_focus_refresh;
	s_android_in_background = false;
	SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
	android_apply_vsync();
	android_apply_render_thread_sched();
	if (should_reopen) {
		android_adpf_open_session();
		s_adpf_needs_focus_refresh = false;
	}
	android_reset_pacer_timestamps();
#endif
}

void gfx_android_delay(Uint32 ms) {
#ifdef __ANDROID__
	int64_t t0 = android_monotonic_ns();
	SDL_Delay(ms);
	int64_t t1 = android_monotonic_ns();
	if (t0 > 0 && t1 > t0)
		s_frame_sleep_ns += (t1 - t0);
#else
	SDL_Delay(ms);
#endif
}

void gfx_android_pre_present(void) {
#ifdef __ANDROID__
	int64_t work_end_ns = android_monotonic_ns();
	if (!s_android_in_background && s_adpf_session && s_pfn_adpf_report &&
	    s_work_start_ns > 0 && work_end_ns > s_work_start_ns) {
		int64_t wall_elapsed_ns = work_end_ns - s_work_start_ns;
		int64_t actual_ns = (wall_elapsed_ns > s_frame_sleep_ns)
			? (wall_elapsed_ns - s_frame_sleep_ns) : 0;
		int64_t cputime_now_ns = android_thread_cputime_ns();
		if (s_work_cputime_start_ns > 0 && cputime_now_ns >= s_work_cputime_start_ns) {
			int64_t cpu_active_ns = cputime_now_ns - s_work_cputime_start_ns;
			int64_t cap_ns = cpu_active_ns + 4500000LL;
			if (actual_ns > cap_ns)
				actual_ns = cap_ns;
		}
		if (actual_ns < 500000LL)
			actual_ns = 500000LL;
		else if (actual_ns > 20000000LL)
			actual_ns = 20000000LL;
		if (actual_ns >= 10500000LL ||
		    (work_end_ns - s_last_adpf_report_ns) >= s_adpf_min_interval_ns) {
			s_pfn_adpf_report(s_adpf_session, actual_ns);
			s_last_adpf_report_ns = work_end_ns;
		}
	}
	s_frame_sleep_ns = 0;
#endif
}

void gfx_android_on_pre_swap(void) {
#ifdef __ANDROID__
	if (s_last_frame_end_ns > 0 &&
	    s_pfn_egl_pres_time && s_pfn_egl_cur_dpy && s_pfn_egl_cur_surf) {
		int64_t now_ns = android_monotonic_ns();
		int64_t present_ns = s_last_frame_end_ns + 15500000LL;
		if (now_ns > 0 && present_ns > now_ns && (present_ns - now_ns) < 25000000LL) {
			void *dpy = s_pfn_egl_cur_dpy();
			void *surf = dpy ? s_pfn_egl_cur_surf(EGL_DRAW_SURFACE_ENUM) : NULL;
			if (dpy && surf)
				s_pfn_egl_pres_time(dpy, surf, present_ns);
		}
	}
#endif
}

void gfx_android_post_present_wait(void) {
#ifdef __ANDROID__
	int64_t after_swap_ns = android_monotonic_ns();
	if (after_swap_ns > 0) {
		int64_t target_end_ns = (s_last_frame_end_ns > 0)
			? (s_last_frame_end_ns + ANDROID_FRAME_PERIOD_NS) : 0;
		if (target_end_ns > after_swap_ns + 300000LL &&
		    (target_end_ns - after_swap_ns) < 17000000LL) {
			struct timespec ts = {
				.tv_sec  = (time_t)(target_end_ns / 1000000000LL),
				.tv_nsec = (long)(target_end_ns % 1000000000LL),
			};
			while (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, NULL) == EINTR) {
			}
			after_swap_ns = android_monotonic_ns();
		}
		if (target_end_ns > 0 && after_swap_ns >= target_end_ns &&
		    (after_swap_ns - target_end_ns) < 1500000LL)
			s_last_frame_end_ns = target_end_ns;
		else
			s_last_frame_end_ns = after_swap_ns;
	} else {
		s_last_frame_end_ns = 0;
	}
	s_work_start_ns = after_swap_ns;
	s_work_cputime_start_ns = android_thread_cputime_ns();
	s_frame_sleep_ns = 0;
#endif
}

/* SDL の初期化 */
int gfx_Initialize(const char *render_driver) {
	window_init(render_driver);
	
	/* offscreen Pixmap */
	makeDIB(SYS35_DEFAULT_WIDTH, SYS35_DEFAULT_HEIGHT, SYS35_DEFAULT_DEPTH);

	gfx_setViewSize(SYS35_DEFAULT_WIDTH, SYS35_DEFAULT_HEIGHT);

#ifdef __EMSCRIPTEN__
	// Prevent SDL from calling emscripten_exit_fullscreen on visibilitychange
	emscripten_set_visibilitychange_callback(NULL, 0, NULL);
#endif
#ifdef __ANDROID__
	android_perf_pacing_init();
#endif

	return 0;
}

void gfx_Remove(void) {
#ifdef ALICEHUB_IOS
	alicehub_ios_set_background_hook(NULL);
#endif
#ifdef __ANDROID__
	android_perf_pacing_fini();
#endif
	if (gfx_palette)
		SDL_DestroyPalette(gfx_palette);
	if (main_surface)
		SDL_DestroySurface(main_surface);
	if (gfx_renderer)
		SDL_DestroyRenderer(gfx_renderer);
	SDL_Quit();
#ifdef ALICEHUB_IOS
	gfx_texture = NULL;
	gfx_palette = NULL;
	main_surface = NULL;
	gfx_renderer = NULL;
	gfx_window = NULL;
	gfx_integer_scaling = gfx_fullscreen = false;
#endif
}

/* name is UTF-8 */
#ifdef __EMSCRIPTEN__
EM_JS(void, gfx_setWindowTitle, (char *name), {
	xsystem35.shell.setWindowTitle(UTF8ToString(name));
});
#else
void gfx_setWindowTitle(char *name) {
	SDL_SetWindowTitle(gfx_window, name);
}
#endif

#ifdef ALICEHUB_IOS
static void ios_background_changed(SDL_bool backgrounded) {
	if (backgrounded) {
		SDL_RenderFlush(gfx_renderer);
		msgskip_syncFile();
	}
	mixer_pause_all(backgrounded);
}
#endif

static void window_init(const char *render_driver) {
	if (render_driver)
		SDL_SetHint(SDL_HINT_RENDER_DRIVER, render_driver);
	SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
#if XSYSTEM35_SDL_VERSION == 2
	SDL_SetHint(SDL_HINT_ACCELEROMETER_AS_JOYSTICK, "0");
#endif

	SDL_Init(SDL_INIT_VIDEO);
	
#ifdef __EMSCRIPTEN__
	// Stop SDL from calling emscripten_sleep() in functions that are called
	// indirectly, which does not work with ASYNCIFY_IGNORE_INDIRECT=1. For
	// details, see https://github.com/emscripten-core/emscripten/issues/10746.
	SDL_SetHint(SDL_HINT_EMSCRIPTEN_ASYNCIFY, "0");

	const char *title = NULL;  // Don't let SDL change document.title.
#else
	const char title[] = "XSystem35 Version " VERSION;
#endif

	Uint32 flags = SDL_WINDOW_RESIZABLE;
#ifdef ALICEHUB_IOS
	SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	flags |= SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_ALLOW_HIGHDPI;
#endif
#ifdef __ANDROID__
	SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
	SDL_SetHint(SDL_HINT_ANDROID_BLOCK_ON_PAUSE_PAUSEAUDIO, "1");
	SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
	SDL_SetHint(SDL_HINT_ANDROID_TRAP_BACK_BUTTON, "1");
	// SDL3 hides the system bars only for a fullscreen window.
	flags |= SDL_WINDOW_FULLSCREEN;
#endif

	gfx_window = sdl_CreateWindow(
		title, SYS35_DEFAULT_WIDTH, SYS35_DEFAULT_HEIGHT, flags);
#if XSYSTEM35_SDL_VERSION == 2
#ifdef __ANDROID__
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
#endif
#endif
	gfx_renderer = sdl_CreateRenderer(gfx_window);
#ifdef ALICEHUB_IOS
	alicehub_ios_set_background_hook(ios_background_changed);
	alicehub_ios_prepare_window(gfx_window);
#endif
#if XSYSTEM35_SDL_VERSION == 3
#ifdef __ANDROID__
	SDL_SetDefaultTextureScaleMode(gfx_renderer, SDL_SCALEMODE_LINEAR);
#else
	SDL_SetDefaultTextureScaleMode(gfx_renderer, SDL_SCALEMODE_NEAREST);
#endif
#endif
	SDL_SetRenderDrawColor(gfx_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
	gfx_palette = SDL_CreatePalette(256);
}

static void makeDIB(int width, int height, int depth) {
	
	if (main_surface) {
		SDL_DestroySurface(main_surface);
	}

	uint32_t format = 0;
	switch (depth) {
	case 8:
		format = SDL_PIXELFORMAT_INDEX8;
		break;
	case 16:
		format = SDL_PIXELFORMAT_RGB565;
		break;
	case 24:
		format = SDL_PIXELFORMAT_XRGB8888;
		// Graphic routines in modules/ assume 4 bytes/pixel mode for 24-bit surfaces.
		break;
	default:
		SYSERROR("invalid pixel depth %d", depth);
	}

	main_surface = sdl_CreateSurface(width, height, format);
	
	if (sdl_SurfaceBitsPerPixel(main_surface) == 8) {
		SDL_SetSurfacePalette(main_surface, gfx_palette);
	}

	if (gfx_dibinfo) {
		free(gfx_dibinfo);
	}
	
	gfx_dibinfo = calloc(1, sizeof(surface_t));
	gfx_dibinfo->width  = width;
	gfx_dibinfo->height = height;
	gfx_dibinfo->alpha  = NULL;
	gfx_dibinfo->sdl_surface = main_surface;
	
	image_setdepth(sdl_SurfaceBitsPerPixel(main_surface));
}

/* offscreen の設定 */
void gfx_setWorldSize(int width, int height, int depth) {
	makeDIB(width, height, depth);
	SDL_FillSurfaceRect(main_surface, NULL, 0);
}

void gfx_getViewSize(int *width, int *height) {
	if (width)
		*width = view_w;
	if (height)
		*height = view_h;
}

SDL_Window *gfx_getWindow(void) {
	return gfx_window;
}

static SDL_Point view_to_window_point(int x, int y) {
	SDL_Point point;
	sdl_RenderCoordinatesToWindow(gfx_renderer, x, y, &point.x, &point.y);
	return point;
}

void gfx_warpMouse(int x, int y) {
	SDL_Point point = view_to_window_point(x, y);
	SDL_WarpMouseInWindow(gfx_window, point.x, point.y);
}

SDL_Rect gfx_viewToWindowRect(SDL_Rect rect) {
	SDL_Point tl = view_to_window_point(rect.x, rect.y);
	SDL_Point br = view_to_window_point(rect.x + rect.w, rect.y + rect.h);
	return (SDL_Rect){tl.x, tl.y, br.x - tl.x, br.y - tl.y};
}

void gfx_convertEventCoordinates(SDL_Event *e) {
#if XSYSTEM35_SDL_VERSION == 2
	// The renderer of SDL2 converts the coordinates in events.
	(void)e;
#else
	SDL_ConvertEventToRenderCoordinates(gfx_renderer, e);
	switch (e->type) {
	case SDL_EVENT_FINGER_DOWN:
	case SDL_EVENT_FINGER_UP:
	case SDL_EVENT_FINGER_MOTION:
		e->tfinger.x = SDL_clamp(e->tfinger.x / view_w, 0.0f, 1.0f);
		e->tfinger.y = SDL_clamp(e->tfinger.y / view_h, 0.0f, 1.0f);
		break;
	}
#endif
}

/*  DIBの取得 */
surface_t *gfx_getDIB(void) {
	return gfx_dibinfo;
}

SDL_Surface *gfx_createSurfaceView(SDL_Surface *sf, int x, int y, int w, int h) {
	uint8_t *pixels = sf->pixels;
	pixels += y * sf->pitch + x * sdl_SurfaceBytesPerPixel(sf);
	SDL_Surface *view = sdl_CreateSurfaceFrom(w, h, sdl_SurfaceFormat(sf), pixels, sf->pitch);
	if (SDL_GetSurfacePalette(sf))
		SDL_SetSurfacePalette(view, SDL_GetSurfacePalette(sf));
	return view;
}

void gfx_setFullscreen(bool on) {
#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__) && !defined(ALICEHUB_IOS)
	if (on == gfx_fullscreen)
		return;
	sdl_SetWindowFullscreen(gfx_window, on);
	gfx_fullscreen = on;
#endif
}

bool gfx_isFullscreen(void) {
	return gfx_fullscreen;
}

void gfx_raiseWindow(void) {
	SDL_RaiseWindow(gfx_window);
}

void gfx_setViewSize(int w, int h) {
	if (w == view_w && h == view_h) return;

	view_w = w;
	view_h = h;

#if !defined(__ANDROID__) && !defined(ALICEHUB_IOS)
	sdl_SetWindowContentSize(gfx_window, w, h);
#endif
	sdl_SetRenderLogicalPresentation(gfx_renderer, w, h, gfx_integer_scaling);
	if (gfx_texture)
		SDL_DestroyTexture(gfx_texture);
	gfx_texture = SDL_CreateTexture(
		gfx_renderer, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STREAMING, w, h);

#ifdef __EMSCRIPTEN__
	EM_ASM( xsystem35.shell.windowSizeChanged(); );
#endif
}

void gfx_setIntegerScaling(bool enable) {
	gfx_integer_scaling = enable;
	sdl_SetRenderLogicalPresentation(gfx_renderer, view_w, view_h, enable);
}

bool EMSCRIPTEN_KEEPALIVE save_screenshot(const char* path) {
	SDL_Rect *r = &nact->ags.view_area;
	SDL_Surface *view = gfx_createSurfaceView(main_surface, r->x, r->y, r->w, r->h);
	bool ok = sdl_SaveBMP(view, path);
	SDL_DestroySurface(view);
	return ok;
}
