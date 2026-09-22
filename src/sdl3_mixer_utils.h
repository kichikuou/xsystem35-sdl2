#ifndef XSYSTEM35_SDL3_MIXER_UTILS_H
#define XSYSTEM35_SDL3_MIXER_UTILS_H

#include <stdint.h>

float sdl3_mixer_gain(int volume, int balance);
int sdl3_mixer_10ms_to_ms(int time);
int sdl3_mixer_elapsed_10ms(uint32_t start, uint32_t now);

#endif /* XSYSTEM35_SDL3_MIXER_UTILS_H */
