#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/audioio.h>
#include <glib.h>

#include "system.h"
#include "audio.h"
#include "audio_sun.h"
#include "mixer_sun.c"


static int audio_open(audiodevice_t *audio, chanfmt_t fmt);
static int audio_close(audiodevice_t *audio);
static int audio_write(audiodevice_t *audio, unsigned char *buf, int cnt);

static int audio_open(audiodevice_t *audio, chanfmt_t fmt) {
	audio_sun_t *asun = (audio_sun_t *)audio->data_pcm;
	audio_info_t info;
	
	if (0 > (asun->fd = open(asun->dev, O_WRONLY, 0))) {
		perror("open");
		WARNING("Opening audio device %s failed\n", asun->dev);
		return -1;
	}
	
	AUDIO_INITINFO(&info);
	info.play.sample_rate = fmt.rate;
	info.play.channels    = fmt.ch;
	info.play.precision   = fmt.bit;
	info.play.encoding    = AUDIO_ENCODING_LINEAR;
	info.play.samples     = 0;
	
	if (0 > ioctl(asun->fd, AUDIO_SETINFO, &info)) {
		WARNING("Unable to set AUDIO_SETINFO\n");
		goto _err_exit;
	}
	
	audio->buf.len = 4096;
	audio->fd = asun->fd;
	
	return OK;

 _err_exit:
	audio->fd = -1;
	return NG;
}
	
static int audio_close(audiodevice_t *audio) {
	audio_sun_t *asun = (audio_sun_t *)audio->data_pcm;
	int ret = OK;
	
	if (asun->fd > -1) {
		ret = close(asun->fd);
	}
	
	audio->fd = asun->fd = -1;
	
	return ret;
}

static int audio_write(audiodevice_t *audio, unsigned char *buf, int cnt) {
	audio_sun_t *asun = (audio_sun_t *)audio->data_pcm;
	int ret = 0;
	
	if (cnt == 0) return 0;

	if (asun->fd > -1) {
		ret = write(asun->fd, buf, cnt);
		if (ret < 0) {
			perror("write");
		}
	}
	return ret;
}

int sunaudio_exit(audiodevice_t *dev) {
	if (dev == NULL) return OK;
	
	mixer_exit(dev);
	
	g_free(dev->data_pcm);
	g_free(dev->data_mix);
	return OK;
}

int sunaudio_init(audiodevice_t *dev, char *devaudio, char *devaudioctl) {
	audio_sun_t *asun;

	asun = g_new0(audio_sun_t, 1);
	
	asun->fd = -1;
	asun->dev = (devaudio == NULL ? "/dev/audio" : devaudio);
	mixer_init(dev, devaudioctl);
	
	dev->data_pcm = asun;
	dev->id = AUDIO_PCM_SUN;
	dev->fd = -1;
	dev->open = audio_open;
	dev->close = audio_close;
	dev->write = audio_write;
	dev->mix_set = mixer_set_level;
	dev->mix_get = mixer_get_level;
	dev->exit = sunaudio_exit;
	
	NOTICE("SUN audio Initilize OK\n");
	
	return OK;
}
