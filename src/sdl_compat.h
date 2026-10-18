/*
 * Copyright (C) 2026 kichikuou <KichikuouChrome@gmail.com>
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

// Include SDL through this header only.
//
// Where an SDL3 function has an exact SDL2 equivalent and its name is not
// used by SDL2, use the SDL3 name. For SDL2, this header defines it as a
// macro or an inline function. Functions that return a success value return
// true on success, as in SDL3.
//
// Where the SDL2 and SDL3 functions have different parameters or behavior,
// use the sdl_ function of this header. These functions also return true on
// success.

#ifndef __SDL_COMPAT_H__
#define __SDL_COMPAT_H__

#include "config.h"

#include <stdbool.h>

#if XSYSTEM35_SDL_VERSION == 2
#include <SDL.h>
#elif XSYSTEM35_SDL_VERSION == 3
#include <SDL3/SDL.h>
#else
#error "Unsupported XSYSTEM35_SDL_VERSION"
#endif

#if SDL_MAJOR_VERSION != XSYSTEM35_SDL_VERSION
#error "The SDL headers do not match XSYSTEM35_SDL_VERSION"
#endif

#if XSYSTEM35_SDL_VERSION == 2

typedef SDL_RWops SDL_IOStream;
typedef SDL_mutex SDL_Mutex;
typedef SDL_cond SDL_Condition;
typedef SDL_atomic_t SDL_AtomicInt;
typedef SDL_RendererFlip SDL_FlipMode;

#define SDL_AUDIO_S16LE AUDIO_S16LSB
#define SDL_SCALEMODE_NEAREST SDL_ScaleModeNearest
#define SDL_SCANCODE_COUNT SDL_NUM_SCANCODES

#define SDL_SYSTEM_CURSOR_TEXT SDL_SYSTEM_CURSOR_IBEAM
#define SDL_SYSTEM_CURSOR_NOT_ALLOWED SDL_SYSTEM_CURSOR_NO
#define SDL_SYSTEM_CURSOR_MOVE SDL_SYSTEM_CURSOR_SIZEALL
#define SDL_SYSTEM_CURSOR_NESW_RESIZE SDL_SYSTEM_CURSOR_SIZENESW
#define SDL_SYSTEM_CURSOR_NS_RESIZE SDL_SYSTEM_CURSOR_SIZENS
#define SDL_SYSTEM_CURSOR_NWSE_RESIZE SDL_SYSTEM_CURSOR_SIZENWSE
#define SDL_SYSTEM_CURSOR_EW_RESIZE SDL_SYSTEM_CURSOR_SIZEWE

#define SDL_EVENT_QUIT SDL_QUIT
#define SDL_EVENT_DID_ENTER_FOREGROUND SDL_APP_DIDENTERFOREGROUND
#define SDL_EVENT_KEY_DOWN SDL_KEYDOWN
#define SDL_EVENT_KEY_UP SDL_KEYUP
#define SDL_EVENT_TEXT_EDITING SDL_TEXTEDITING
#define SDL_EVENT_TEXT_INPUT SDL_TEXTINPUT
#define SDL_EVENT_MOUSE_MOTION SDL_MOUSEMOTION
#define SDL_EVENT_MOUSE_BUTTON_DOWN SDL_MOUSEBUTTONDOWN
#define SDL_EVENT_MOUSE_BUTTON_UP SDL_MOUSEBUTTONUP
#define SDL_EVENT_MOUSE_WHEEL SDL_MOUSEWHEEL
#define SDL_EVENT_JOYSTICK_AXIS_MOTION SDL_JOYAXISMOTION
#define SDL_EVENT_JOYSTICK_BALL_MOTION SDL_JOYBALLMOTION
#define SDL_EVENT_JOYSTICK_HAT_MOTION SDL_JOYHATMOTION
#define SDL_EVENT_JOYSTICK_BUTTON_DOWN SDL_JOYBUTTONDOWN
#define SDL_EVENT_JOYSTICK_BUTTON_UP SDL_JOYBUTTONUP
#define SDL_EVENT_JOYSTICK_ADDED SDL_JOYDEVICEADDED
#define SDL_EVENT_FINGER_DOWN SDL_FINGERDOWN
#define SDL_EVENT_FINGER_UP SDL_FINGERUP
#define SDL_EVENT_FINGER_MOTION SDL_FINGERMOTION

#define SDL_Swap16LE SDL_SwapLE16
#define SDL_Swap32LE SDL_SwapLE32

#define SDL_GetRectIntersection SDL_IntersectRect
#define SDL_CreatePalette SDL_AllocPalette
#define SDL_DestroyPalette SDL_FreePalette
#define SDL_DestroySurface SDL_FreeSurface
#define SDL_DestroyCursor SDL_FreeCursor
#define SDL_CloseJoystick SDL_JoystickClose
#define SDL_GetJoystickName SDL_JoystickName
#define SDL_GetNumJoystickAxes SDL_JoystickNumAxes
#define SDL_GetNumJoystickButtons SDL_JoystickNumButtons
#define SDL_IOFromFile SDL_RWFromFile
#define SDL_IOFromConstMem SDL_RWFromConstMem
#define SDL_GetIOSize SDL_RWsize
#define SDL_CreateCondition SDL_CreateCond
#define SDL_DestroyCondition SDL_DestroyCond
#define SDL_GetAtomicInt SDL_AtomicGet
#define SDL_SetAtomicInt SDL_AtomicSet
#define SDL_CompareAndSwapAtomicInt SDL_AtomicCAS

static inline bool SDL_GetRectUnion(const SDL_Rect *a, const SDL_Rect *b, SDL_Rect *result) {
	SDL_UnionRect(a, b, result);
	return true;
}

static inline SDL_Palette *SDL_GetSurfacePalette(SDL_Surface *surface) {
	return surface->format->palette;
}

static inline Uint32 SDL_MapSurfaceRGB(SDL_Surface *surface, Uint8 r, Uint8 g, Uint8 b) {
	return SDL_MapRGB(surface->format, r, g, b);
}

static inline Uint32 SDL_MapSurfaceRGBA(SDL_Surface *surface, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	return SDL_MapRGBA(surface->format, r, g, b, a);
}

static inline bool SDL_SetSurfaceColorKey(SDL_Surface *surface, bool enabled, Uint32 key) {
	return SDL_SetColorKey(surface, enabled, key) == 0;
}

static inline bool SDL_FillSurfaceRect(SDL_Surface *dst, const SDL_Rect *rect, Uint32 color) {
	return SDL_FillRect(dst, rect, color) == 0;
}

static inline bool SDL_FillSurfaceRects(SDL_Surface *dst, const SDL_Rect *rects, int count, Uint32 color) {
	return SDL_FillRects(dst, rects, count, color) == 0;
}

static inline bool SDL_BlitSurfaceUnchecked(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, const SDL_Rect *dstrect) {
	return SDL_LowerBlit(src, (SDL_Rect *)srcrect, dst, (SDL_Rect *)dstrect) == 0;
}

static inline bool SDL_SetRenderClipRect(SDL_Renderer *renderer, const SDL_Rect *rect) {
	return SDL_RenderSetClipRect(renderer, rect) == 0;
}

static inline bool SDL_RenderLine(SDL_Renderer *renderer, float x1, float y1, float x2, float y2) {
	return SDL_RenderDrawLineF(renderer, x1, y1, x2, y2) == 0;
}

static inline void SDL_SetEventEnabled(Uint32 type, bool enabled) {
	SDL_EventState(type, enabled ? SDL_ENABLE : SDL_DISABLE);
}

static inline bool SDL_TextInputActive(SDL_Window *window) {
	(void)window;
	return SDL_IsTextInputActive();
}

static inline bool SDL_SetTextInputArea(SDL_Window *window, const SDL_Rect *rect, int cursor) {
	(void)window;
	(void)cursor;
	SDL_SetTextInputRect(rect);
	return true;
}

static inline void SDL_SetJoystickEventsEnabled(bool enabled) {
	SDL_JoystickEventState(enabled ? SDL_ENABLE : SDL_DISABLE);
}

static inline size_t SDL_ReadIO(SDL_IOStream *context, void *ptr, size_t size) {
	return SDL_RWread(context, ptr, 1, size);
}

static inline bool SDL_CloseIO(SDL_IOStream *context) {
	return SDL_RWclose(context) == 0;
}

static inline bool SDL_LoadWAV_IO(SDL_IOStream *src, bool closeio, SDL_AudioSpec *spec, Uint8 **audio_buf, Uint32 *audio_len) {
	return SDL_LoadWAV_RW(src, closeio, spec, audio_buf, audio_len) != NULL;
}

static inline void SDL_SignalCondition(SDL_Condition *cond) {
	SDL_CondSignal(cond);
}

static inline void SDL_WaitCondition(SDL_Condition *cond, SDL_Mutex *mutex) {
	SDL_CondWait(cond, mutex);
}

static inline bool SDL_WaitConditionTimeout(SDL_Condition *cond, SDL_Mutex *mutex, Sint32 timeout_ms) {
	return SDL_CondWaitTimeout(cond, mutex, timeout_ms) == 0;
}

#endif // XSYSTEM35_SDL_VERSION == 2

// Surface

// Also creates a palette for an indexed format.
static inline SDL_Surface *sdl_CreateSurface(int width, int height, Uint32 format) {
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_CreateRGBSurfaceWithFormat(0, width, height, SDL_BITSPERPIXEL(format), format);
#else
	SDL_Surface *surface = SDL_CreateSurface(width, height, format);
	if (surface && SDL_ISPIXELFORMAT_INDEXED(format))
		SDL_CreateSurfacePalette(surface);
	return surface;
#endif
}

// Also creates a palette for an indexed format.
static inline SDL_Surface *sdl_CreateSurfaceFrom(int width, int height, Uint32 format, void *pixels, int pitch) {
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_CreateRGBSurfaceWithFormatFrom(pixels, width, height, SDL_BITSPERPIXEL(format), pitch, format);
#else
	SDL_Surface *surface = SDL_CreateSurfaceFrom(width, height, format, pixels, pitch);
	if (surface && SDL_ISPIXELFORMAT_INDEXED(format))
		SDL_CreateSurfacePalette(surface);
	return surface;
#endif
}

static inline Uint32 sdl_SurfaceFormat(const SDL_Surface *surface) {
#if XSYSTEM35_SDL_VERSION == 2
	return surface->format->format;
#else
	return surface->format;
#endif
}

static inline int sdl_SurfaceBitsPerPixel(const SDL_Surface *surface) {
	return SDL_BITSPERPIXEL(sdl_SurfaceFormat(surface));
}

static inline int sdl_SurfaceBytesPerPixel(const SDL_Surface *surface) {
	return SDL_BYTESPERPIXEL(sdl_SurfaceFormat(surface));
}

static inline void sdl_GetSurfaceRGB(Uint32 pixel, SDL_Surface *surface, Uint8 *r, Uint8 *g, Uint8 *b) {
#if XSYSTEM35_SDL_VERSION == 2
	SDL_GetRGB(pixel, surface->format, r, g, b);
#else
	SDL_GetRGB(pixel, SDL_GetPixelFormatDetails(surface->format), SDL_GetSurfacePalette(surface), r, g, b);
#endif
}

static inline void sdl_GetSurfaceRGBA(Uint32 pixel, SDL_Surface *surface, Uint8 *r, Uint8 *g, Uint8 *b, Uint8 *a) {
#if XSYSTEM35_SDL_VERSION == 2
	SDL_GetRGBA(pixel, surface->format, r, g, b, a);
#else
	SDL_GetRGBA(pixel, SDL_GetPixelFormatDetails(surface->format), SDL_GetSurfacePalette(surface), r, g, b, a);
#endif
}

static inline SDL_Surface *sdl_ConvertSurface(SDL_Surface *surface, Uint32 format) {
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_ConvertSurfaceFormat(surface, format, 0);
#else
	return SDL_ConvertSurface(surface, format);
#endif
}

// Same as SDL_BlitSurface(), but also stores the destination area that the
// blit changed in dstrect. The width and height of dstrect are ignored.
#if XSYSTEM35_SDL_VERSION == 2
static inline bool sdl_BlitSurfaceClipped(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
	return SDL_BlitSurface(src, srcrect, dst, dstrect) == 0;
}
#else
bool sdl_BlitSurfaceClipped(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect);
#endif

// Scales with nearest-neighbor sampling.
static inline bool sdl_BlitSurfaceScaled(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, const SDL_Rect *dstrect) {
#if XSYSTEM35_SDL_VERSION == 2
	SDL_Rect r;
	if (dstrect)
		r = *dstrect;
	return SDL_BlitScaled(src, srcrect, dst, dstrect ? &r : NULL) == 0;
#else
	return SDL_BlitSurfaceScaled(src, srcrect, dst, dstrect, SDL_SCALEMODE_NEAREST);
#endif
}

// Scales with nearest-neighbor sampling. Supports 8-bit surfaces.
static inline bool sdl_StretchSurface(SDL_Surface *src, const SDL_Rect *srcrect, SDL_Surface *dst, const SDL_Rect *dstrect) {
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_SoftStretch(src, srcrect, dst, dstrect) == 0;
#else
	return SDL_StretchSurface(src, srcrect, dst, dstrect, SDL_SCALEMODE_NEAREST);
#endif
}

#if XSYSTEM35_SDL_VERSION == 2
bool sdl_FlipSurface(SDL_Surface *surface, SDL_FlipMode flip);
#else
static inline bool sdl_FlipSurface(SDL_Surface *surface, SDL_FlipMode flip) {
	return SDL_FlipSurface(surface, flip);
}
#endif

// Rendering

#if XSYSTEM35_SDL_VERSION == 3
static inline const SDL_FRect *sdl_RectToFRect(const SDL_Rect *rect, SDL_FRect *frect) {
	if (!rect)
		return NULL;
	SDL_RectToFRect(rect, frect);
	return frect;
}
#endif

static inline bool sdl_RenderTexture(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *srcrect, const SDL_Rect *dstrect) {
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_RenderCopy(renderer, texture, srcrect, dstrect) == 0;
#else
	SDL_FRect src, dst;
	return SDL_RenderTexture(renderer, texture, sdl_RectToFRect(srcrect, &src), sdl_RectToFRect(dstrect, &dst));
#endif
}

static inline bool sdl_RenderTextureF(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *srcrect, const SDL_FRect *dstrect) {
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_RenderCopyF(renderer, texture, srcrect, dstrect) == 0;
#else
	SDL_FRect src;
	return SDL_RenderTexture(renderer, texture, sdl_RectToFRect(srcrect, &src), dstrect);
#endif
}

static inline bool sdl_RenderTextureRotated(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *srcrect, const SDL_Rect *dstrect, double angle, const SDL_Point *center, SDL_FlipMode flip) {
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_RenderCopyEx(renderer, texture, srcrect, dstrect, angle, center, flip) == 0;
#else
	SDL_FRect src, dst;
	SDL_FPoint fcenter;
	if (center)
		fcenter = (SDL_FPoint){center->x, center->y};
	return SDL_RenderTextureRotated(renderer, texture, sdl_RectToFRect(srcrect, &src), sdl_RectToFRect(dstrect, &dst),
		angle, center ? &fcenter : NULL, flip);
#endif
}

static inline bool sdl_RenderFillRect(SDL_Renderer *renderer, const SDL_Rect *rect) {
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_RenderFillRect(renderer, rect) == 0;
#else
	SDL_FRect frect;
	return SDL_RenderFillRect(renderer, sdl_RectToFRect(rect, &frect));
#endif
}

static inline bool sdl_GetTextureSize(SDL_Texture *texture, int *w, int *h) {
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_QueryTexture(texture, NULL, NULL, w, h) == 0;
#else
	float fw, fh;
	if (!SDL_GetTextureSize(texture, &fw, &fh))
		return false;
	*w = fw;
	*h = fh;
	return true;
#endif
}

static inline bool sdl_RenderTargetSupported(SDL_Renderer *renderer) {
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_RenderTargetSupported(renderer);
#else
	(void)renderer;
	return true;
#endif
}

// Sets the logical size with letterboxing, and with integer scaling if
// integer_scale is true.
static inline bool sdl_SetRenderLogicalPresentation(SDL_Renderer *renderer, int w, int h, bool integer_scale) {
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_RenderSetLogicalSize(renderer, w, h) == 0 &&
		SDL_RenderSetIntegerScale(renderer, integer_scale) == 0;
#else
	return SDL_SetRenderLogicalPresentation(renderer, w, h,
		integer_scale ? SDL_LOGICAL_PRESENTATION_INTEGER_SCALE : SDL_LOGICAL_PRESENTATION_LETTERBOX);
#endif
}

static inline bool sdl_RenderCoordinatesToWindow(SDL_Renderer *renderer, int x, int y, int *window_x, int *window_y) {
#if XSYSTEM35_SDL_VERSION == 2
	SDL_RenderLogicalToWindow(renderer, x, y, window_x, window_y);
	return true;
#else
	float wx, wy;
	if (!SDL_RenderCoordinatesToWindow(renderer, x, y, &wx, &wy))
		return false;
	*window_x = wx;
	*window_y = wy;
	return true;
#endif
}

#if XSYSTEM35_SDL_VERSION == 2
static inline SDL_Color sdl_VertexColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	return (SDL_Color){r, g, b, a};
}
#else
static inline SDL_FColor sdl_VertexColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	return (SDL_FColor){r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f};
}
#endif

// Window and display

// Creates a window whose content area has the same apparent size on displays
// with different pixel densities.
static inline SDL_Window *sdl_CreateWindow(const char *title, int w, int h, Uint32 flags) {
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, flags);
#else
	// On platforms where window coordinates are in pixels, such as Windows,
	// the content scale of the display is larger than 1.
	float scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
	if (scale <= 0.0f)
		scale = 1.0f;
	SDL_WindowFlags window_flags = flags;
#ifndef __EMSCRIPTEN__
	// The web page scales the canvas to its display size.
	window_flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
#endif
	return SDL_CreateWindow(title, (int)(w * scale + 0.5f), (int)(h * scale + 0.5f), window_flags);
#endif
}

// Sets the size of the window's content area, with the same scaling as
// sdl_CreateWindow().
static inline void sdl_SetWindowContentSize(SDL_Window *window, int w, int h) {
#if XSYSTEM35_SDL_VERSION == 2
	SDL_SetWindowSize(window, w, h);
#else
	// The display scale divided by the pixel density is the content scale.
	float density = SDL_GetWindowPixelDensity(window);
	float scale = SDL_GetWindowDisplayScale(window);
	if (density > 0.0f && scale > 0.0f)
		scale /= density;
	else
		scale = 1.0f;
	SDL_SetWindowSize(window, (int)(w * scale + 0.5f), (int)(h * scale + 0.5f));
#endif
}

static inline SDL_Renderer *sdl_CreateRenderer(SDL_Window *window) {
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_CreateRenderer(window, -1, 0);
#else
	return SDL_CreateRenderer(window, NULL);
#endif
}

// Uses the desktop resolution when fullscreen is true.
static inline bool sdl_SetWindowFullscreen(SDL_Window *window, bool fullscreen) {
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0) == 0;
#else
	return SDL_SetWindowFullscreen(window, fullscreen);
#endif
}

// Gets the current display mode of the primary display.
static inline bool sdl_GetCurrentDisplayMode(SDL_DisplayMode *mode) {
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_GetCurrentDisplayMode(0, mode) == 0;
#else
	const SDL_DisplayMode *m = SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay());
	if (!m)
		return false;
	*mode = *m;
	return true;
#endif
}

// Gets the desktop display mode of the primary display.
static inline bool sdl_GetDesktopDisplayMode(SDL_DisplayMode *mode) {
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_GetDesktopDisplayMode(0, mode) == 0;
#else
	const SDL_DisplayMode *m = SDL_GetDesktopDisplayMode(SDL_GetPrimaryDisplay());
	if (!m)
		return false;
	*mode = *m;
	return true;
#endif
}

#ifdef _WIN32
// Returns the HWND of the window, or NULL.
#if XSYSTEM35_SDL_VERSION == 2
void *sdl_GetWindowHWND(SDL_Window *window);
#else
static inline void *sdl_GetWindowHWND(SDL_Window *window) {
	return SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
}
#endif
#endif

// Input

static inline SDL_Keycode sdl_KeyboardEventKey(const SDL_KeyboardEvent *e) {
#if XSYSTEM35_SDL_VERSION == 2
	return e->keysym.sym;
#else
	return e->key;
#endif
}

static inline SDL_Scancode sdl_KeyboardEventScancode(const SDL_KeyboardEvent *e) {
#if XSYSTEM35_SDL_VERSION == 2
	return e->keysym.scancode;
#else
	return e->scancode;
#endif
}

// Returns the vertical scroll amount in whole steps.
static inline int sdl_MouseWheelEventY(const SDL_MouseWheelEvent *e) {
#if XSYSTEM35_SDL_VERSION == 2
	return e->y;
#else
	return e->integer_y;
#endif
}

// Returns the number of fingers that touch the device of the event.
static inline int sdl_CountTouchFingers(const SDL_TouchFingerEvent *e) {
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_GetNumTouchFingers(e->touchId);
#else
	int count = 0;
	SDL_free(SDL_GetTouchFingers(e->touchID, &count));
	return count;
#endif
}

// Converts the timestamp of an event to the time scale of SDL_GetTicks().
static inline Uint64 sdl_TimestampToTicks(Uint64 timestamp) {
#if XSYSTEM35_SDL_VERSION == 2
	return timestamp;
#else
	return SDL_NS_TO_MS(timestamp);
#endif
}

static inline bool sdl_StartTextInput(SDL_Window *window) {
#if XSYSTEM35_SDL_VERSION == 2
	(void)window;
	SDL_StartTextInput();
	return true;
#else
	return SDL_StartTextInput(window);
#endif
}

static inline bool sdl_StopTextInput(SDL_Window *window) {
#if XSYSTEM35_SDL_VERSION == 2
	(void)window;
	SDL_StopTextInput();
	return true;
#else
	return SDL_StopTextInput(window);
#endif
}

// Miscellaneous

static inline bool sdl_SaveBMP(SDL_Surface *surface, const char *file) {
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_SaveBMP(surface, file) == 0;
#else
	return SDL_SaveBMP(surface, file);
#endif
}

static inline bool sdl_ShowMessageBox(const SDL_MessageBoxData *messageboxdata, int *buttonid) {
#if XSYSTEM35_SDL_VERSION == 2
	return SDL_ShowMessageBox(messageboxdata, buttonid) == 0;
#else
	return SDL_ShowMessageBox(messageboxdata, buttonid);
#endif
}

#endif /* __SDL_COMPAT_H__ */
