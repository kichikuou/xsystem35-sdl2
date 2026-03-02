#ifndef __MUSIC_MIDI_H__
#define __MUSIC_MIDI_H__

#include "portab.h"
#include "midi.h"

bool musmidi_init(void);
void musmidi_exit(void);
void musmidi_reset(void);
bool musmidi_start(int no, int loop);
void musmidi_stop(void);
void musmidi_pause(void);
void musmidi_unpause(void);
midiplaystate musmidi_getpos(void);
bool musmidi_setflag(int mode, int index, int val);
int musmidi_getflag(int mode, int index);
bool musmidi_fadestart(int time, int volume, int stop);
bool musmidi_fading(void);

#endif /* __MUSIC_MIDI_H__ */
