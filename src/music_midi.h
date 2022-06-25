#ifndef __MUSIC_MIDI_H__
#define __MUSIC_MIDI_H__

#include "portab.h"
#include "midi.h"
#include "ald_manager.h"

struct _midiobj {
	mididevice_t *dev;
	dridata *dfile;
};
typedef struct _midiobj midiobj_t;

int musmidi_init(void);
int musmidi_exit(void);
int musmidi_reset(void);
int musmidi_start(int no, int loop);
int musmidi_stop(void);
int musmidi_pause(void);
int musmidi_unpause(void);
midiplaystate musmidi_getpos(void);
int musmidi_setflag(int mode, int index, int val);
int musmidi_getflag(int mode, int index);
int musmidi_fadestart(int time, int volume, int stop);
boolean musmidi_fading(void);

#endif /* __MUSIC_MIDI_H__ */
