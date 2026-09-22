#ifndef XSYSTEM35_SDL3_MIXER_BACKEND_H
#define XSYSTEM35_SDL3_MIXER_BACKEND_H

#include "portab.h"
#include "sdl_mixer_compat.h"

bool sdl3_mixer_backend_init(int audio_buffer_size);
void sdl3_mixer_backend_exit(void);
MIX_Mixer *sdl3_mixer_backend_get(void);

#endif /* XSYSTEM35_SDL3_MIXER_BACKEND_H */
