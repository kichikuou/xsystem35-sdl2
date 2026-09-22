#include "sdl3_mixer_utils.h"

static int clamp_volume(int volume)
{
	if (volume < 0)
		return 0;
	if (volume > 100)
		return 100;
	return volume;
}

float sdl3_mixer_gain(int volume, int balance)
{
	return (float)(clamp_volume(volume) * clamp_volume(balance)) / 10000.0f;
}

int sdl3_mixer_10ms_to_ms(int time)
{
	return time * 10;
}

int sdl3_mixer_elapsed_10ms(uint32_t start, uint32_t now)
{
	return (now - start) / 10;
}
