#ifndef __AUDIO_SUN_H__
#define __AUDIO_SUN_H__

typedef struct {
	int fd;
	char *dev;
} audio_sun_t;


typedef struct {
	char *mdev;
} mixer_sun_t;

extern int sunaudio_init(audiodevice_t *dev, char *devaudio, char *devaudioctl);

#endif /* __AUDIO_SUN_H__ */
