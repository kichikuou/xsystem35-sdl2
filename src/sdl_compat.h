#ifndef XSYSTEM35_SDL_COMPAT_H
#define XSYSTEM35_SDL_COMPAT_H

#ifndef XSYSTEM35_SDL_VERSION
#define XSYSTEM35_SDL_VERSION 2
#endif

#if XSYSTEM35_SDL_VERSION == 2
#include <SDL.h>
#ifdef _WIN32
#include <SDL_syswm.h>
#endif
#elif XSYSTEM35_SDL_VERSION == 3
#include <SDL3/SDL.h>
#else
#error "Unsupported XSYSTEM35_SDL_VERSION"
#endif

#if SDL_MAJOR_VERSION != XSYSTEM35_SDL_VERSION
#error "The selected SDL headers do not match XSYSTEM35_SDL_VERSION"
#endif

#if XSYSTEM35_SDL_VERSION == 3
#undef SDL_SwapLE16
#undef SDL_SwapLE32
#define SDL_SwapLE16 SDL_Swap16LE
#define SDL_SwapLE32 SDL_Swap32LE
#endif

#endif /* XSYSTEM35_SDL_COMPAT_H */
