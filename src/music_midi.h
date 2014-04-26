#ifndef __MUSIC_MIDI_H__
#define __MUSIC_MIDI_H__

#include "portab.h"
#include "midi.h"
#include "ald_manager.h"

struct _midiobj {
	int st; // オブジェクトの遷移状態
	
	boolean in_play; // 演奏中？

	int no;
	int loop;
	int cnt;
	mididevice_t *dev;

	dridata *dfile;
};
typedef struct _midiobj midiobj_t;

extern int musmidi_init();
extern int musmidi_exit();
extern int musmidi_start(int no, int loop);
extern int musmidi_stop();
extern int musmidi_pause();
extern int musmidi_unpause();
extern midiplaystate musmidi_getpos();
extern int musmidi_setflag(int mode, int index, int val);
extern int musmidi_getflag(int mode, int index);
extern int musmidi_cb();

#endif /* __MUSIC_MIDI_H__ */
