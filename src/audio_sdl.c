/*
 * audio_sdl.c  SDL audio lowlevel acess
 *
 * Copyright (C) 2017 <KichikuouChrome@gmail.com>
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

#include "portab.h"

#include <stdlib.h>
#include <SDL.h>

#include "system.h"
#include "audio.h"
#include "music_pcm.h"

typedef struct {
	Uint8* stream;
	int len;
} audio_sdl_t;

static void audio_callback(void* userdata, Uint8* stream, int len) {
	audio_sdl_t *asdl = (audio_sdl_t*)((audiodevice_t*)userdata)->data_pcm;
	asdl->stream = stream;
	asdl->len = len;
	muspcm_write2dev();
	memset(asdl->stream, 0, asdl->len);
	asdl->stream = NULL;
	asdl->len = 0;
}

static int audio_open(audiodevice_t *dev, chanfmt_t fmt) {
	SDL_AudioSpec spec;
	memset(&spec, 0, sizeof(spec));
	spec.freq = fmt.rate;
	spec.format = fmt.bit == 16 ? AUDIO_S16 : AUDIO_U8;
	spec.channels = fmt.ch;
	spec.samples = 4096;
	spec.callback = audio_callback;
	spec.userdata = dev;
	if (SDL_OpenAudio(&spec, NULL) < 0) {
		WARNING("SDL_OpenAudio failed\n");
		return NG;
	}
	dev->buf.len = spec.samples * (fmt.bit / 8) * fmt.ch;
	SDL_PauseAudio(0);
	return OK;
}

static int audio_close(audiodevice_t *dev) {
	SDL_CloseAudio();
	return OK;
}

static int audio_write(audiodevice_t *dev, unsigned char *buf, int cnt) {
	audio_sdl_t *asdl = (audio_sdl_t*)dev->data_pcm;
	if (cnt > asdl->len)
		cnt = asdl->len;
	memcpy(asdl->stream, buf, cnt);
	asdl->stream += cnt;
	asdl->len -= cnt;
	return cnt;
}

static void mixer_set_level(audiodevice_t *dev, int ch, int level) {
}

static int mixer_get_level(audiodevice_t *dev, int ch) {
	return 0;
}

static int sdlaudio_exit(audiodevice_t *dev) {
	if (dev == NULL) return OK;
	
	return OK;
}

int sdlaudio_init(audiodevice_t *dev) {
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
		return NG;

	audio_sdl_t *asdl = calloc(1, sizeof(audio_sdl_t));
	dev->data_pcm = asdl;
	
	dev->id      = AUDIO_PCM_SDL;
	dev->fd      = -1;
	dev->open    = audio_open;
	dev->close   = audio_close;
	dev->write   = audio_write;
	dev->mix_set = mixer_set_level;
	dev->mix_get = mixer_get_level;
	dev->exit    = sdlaudio_exit;
	
	NOTICE("SDL audio initilize OK\n");
	return OK;
}
