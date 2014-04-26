#include "audio_sun.h"

static void mixer_set_level(audiodevice_t *dev, int ch, int level) {
	return;
}

static int mixer_get_level(audiodevice_t *dev, int ch) {
	return 0;
}

static int mixer_exit(audiodevice_t *dev) {
	return OK;
}

static int mixer_init(audiodevice_t *dev, char *devmix) {
	mixer_sun_t *mix;
	
	// mix = g_new0(mixer_sun_t, 1);

	
	return OK;
}

