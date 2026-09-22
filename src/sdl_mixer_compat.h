#ifndef XSYSTEM35_SDL_MIXER_COMPAT_H
#define XSYSTEM35_SDL_MIXER_COMPAT_H

#include "sdl_compat.h"

#if XSYSTEM35_SDL_VERSION == 2
#include <SDL_mixer.h>
#elif XSYSTEM35_SDL_VERSION == 3
#include <SDL3_mixer/SDL_mixer.h>
#else
#error "Unsupported XSYSTEM35_SDL_VERSION"
#endif

#endif /* XSYSTEM35_SDL_MIXER_COMPAT_H */
