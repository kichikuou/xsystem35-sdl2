#ifndef XSYSTEM35_WIN_WINDOW_H
#define XSYSTEM35_WIN_WINDOW_H

#include <windows.h>
#include "sdl_compat.h"

#if XSYSTEM35_SDL_VERSION == 2
#include <SDL_syswm.h>
#endif

static inline HWND win_get_hwnd(SDL_Window *window)
{
#if XSYSTEM35_SDL_VERSION == 2
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	if (!SDL_GetWindowWMInfo(window, &info))
		return NULL;
	return info.info.win.window;
#else
	return (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(window),
		SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
#endif
}

#endif
